﻿#ifdef SUPPORT_PLAYER_MODULE
#include "videostream.h"
#include "packet.h"
#include "frame.h"
#include "player.h"


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

static int queue_picture(player_stat_t *is, AVFrame *src_frame, double pts, double duration, int64_t pos)
{
    frame_t *vp;

    if (!(vp = frame_queue_peek_writable(&is->video_frm_queue)))
        return -1;

    vp->sar = src_frame->sample_aspect_ratio;
    vp->uploaded = 0;

    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;

    vp->pts = pts;
    vp->duration = duration;
    vp->pos = pos;
    //vp->serial = serial;

    //set_default_window_size(vp->width, vp->height, vp->sar);

    // 将AVFrame拷入队列相应位置
    av_frame_move_ref(vp->frame, src_frame);
    //printf("queue frame fomat: %d\n",vp->frame->format);
    // 更新队列计数及写索引
    //printf("before queue ridx: %d,widx: %d,size: %d,maxsize: %d\n ",is->video_frm_queue.rindex,is->video_frm_queue.windex,is->video_frm_queue.size,is->video_frm_queue.max_size);
    frame_queue_push(&is->video_frm_queue);
    //printf("after queue ridx: %d,widx: %d,size: %d,maxsize: %d\n ",is->video_frm_queue.rindex,is->video_frm_queue.windex,is->video_frm_queue.size,is->video_frm_queue.max_size);
    return 0;
}


// 从packet_queue中取一个packet，解码生成frame
static int video_decode_frame(player_stat_t *is, AVFrame *frame)
{
    int ret;
    AVCodecContext *p_codec_ctx = is->p_vcodec_ctx;
    packet_queue_t *p_pkt_queue = &is->video_pkt_queue;

    while (1) {
        AVPacket pkt;
        while (1) {
            if (p_pkt_queue->abort_request) {
                return -1;
            }
            // 3. 从解码器接收frame
            // 3.1 一个视频packet含一个视频frame
            //     解码器缓存一定数量的packet后，才有解码后的frame输出
            //     frame输出顺序是按pts的顺序，如IBBPBBP
            //     frame->pkt_pos变量是此frame对应的packet在视频文件中的偏移地址，值同pkt.pos
            ret = avcodec_receive_frame(p_codec_ctx, frame);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    printf("[video] the decoder has been fully flushed\n");
                    avcodec_flush_buffers(p_codec_ctx);
                    return 0;
                } else if (ret == AVERROR(EAGAIN)) {
                    // printf("[video] fetch a frame, try again\n");
                    break;
                } else {
                    printf("[video] other errors\n");
                    continue;
                }
            } else {
                is->recv_frame_cnt++;
                pthread_cond_signal(&is->continue_read_thread);
                frame->pts = frame->best_effort_timestamp;
                return 1;   // 成功解码得到一个视频帧或一个音频帧，则返回
            }
        }

        // 1. 取出一个packet。使用pkt对应的serial赋值给d->pkt_serial
        if (packet_queue_get(p_pkt_queue, &pkt, true) < 0) {
            printf("[video] get packet abort\n");
            return -1;
        }

        if (pkt.data == is->p_flush_pkt->data) {
            // 复位解码器内部状态/刷新内部缓冲区。
            printf("[video] flush decoder\n");
            avcodec_flush_buffers(p_codec_ctx);
        } else {
            // 如果是最后一个空的packet,只取frame不再送packet
            if (pkt.data == NULL || pkt.size == 0) {
                p_codec_ctx->flags |= (1 << 5);
            } else {
                p_codec_ctx->flags &= ~(1 << 5);
                is->send_frame_cnt++;
            }

            // 2. 将packet发送给解码器
            //    发送packet的顺序是按dts递增的顺序，如IPBBPBB
            //    pkt.pos变量可以标识当前packet在视频文件中的地址偏移
            if (avcodec_send_packet(p_codec_ctx, &pkt) == AVERROR(EAGAIN)) {
                printf("[video] send packet fail\n");
            }
            av_packet_unref(&pkt);
        }
    }
}

// 将视频包解码得到视频帧，然后写入picture队列
static void* video_decode_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    AVFrame *p_frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    int got_picture;
    AVRational tb = is->p_video_stream->time_base;
    AVRational frame_rate = av_guess_frame_rate(is->p_fmt_ctx, is->p_video_stream, NULL);

    if (p_frame == NULL) {
        printf("[video] decode thread no memory\n");
        return NULL;
    }

    printf("video time base : %.2fms.\n", 1000 * av_q2d(tb));
    printf("frame rate num : %d. frame rate den : %d.\n", frame_rate.num, frame_rate.den);

    while (1) {
        if (is->abort_request) {
            break;
        }

        if (is->speed_x && frame_queue_nb_remaining(&is->video_frm_queue)) {
            usleep(10000);
            continue;
        }

        got_picture = video_decode_frame(is, p_frame);
        if (got_picture < 0) {
            goto exit;
        }

        duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational) {frame_rate.den, frame_rate.num}) : 0);   // 当前帧播放时长
        pts = (p_frame->pts == AV_NOPTS_VALUE) ? NAN : p_frame->pts * av_q2d(tb);   // 当前帧显示时间戳

        //if (!is->speed_x)
        //    printf("frame duration : %f. video frame clock : %f.\n", duration, pts);

        ret = queue_picture(is, p_frame, pts, duration, p_frame->pkt_pos);   // 将当前帧压入frame_queue
        av_frame_unref(p_frame);

        if (ret < 0) {
            goto exit;
        }
    }

exit:
    av_frame_free(&p_frame);
    printf("[video] decode thread exit\n");
    return NULL;
}

// 根据视频时钟与同步时钟(如音频时钟)的差值，校正delay值，使视频时钟追赶或等待同步时钟
// 输入参数delay是上一帧播放时长，即上一帧播放后应延时多长时间后再播放当前帧，通过调节此值来调节当前帧播放快慢
// 返回值delay是将输入参数delay经校正后得到的值
static double compute_target_delay(double delay, player_stat_t *is)
{
    double sync_threshold, diff = 0;

    /* update delay to follow master synchronisation source */

    /* if video is slave, we try to correct big delays by
       duplicating or deleting a frame */
    // 视频时钟与同步时钟(如音频时钟)的差异，时钟值是上一帧pts值(实为：上一帧pts + 上一帧至今流逝的时间差)
    if (is->audio_idx >= 0 && is->video_idx >= 0)
        diff = get_clock(&is->video_clk) - get_clock(&is->audio_clk);
    else
        return delay;
    //printf("audio pts: %lf,video pts: %lf\n",is->audio_clk.pts,is->video_clk.pts);
    //printf("audio clock: %lf,video clock: %lf\n",get_clock(&is->audio_clk),get_clock(&is->video_clk));
    //printf("video pts: %lf,lu: %lf,curtime: %lf\n ",is->video_clk.pts,is->video_clk.last_updated,av_gettime_relative() / 1000000.0);
    //printf("audio pts: %lf,lu: %lf,curtime: %lf\n ",is->audio_clk.pts,is->audio_clk.last_updated,av_gettime_relative() / 1000000.0);
    //printf("video diff audio time: %lf\n",diff);
    // delay是上一帧播放时长：当前帧(待播放的帧)播放时间与上一帧播放时间差理论值
    // diff是视频时钟与同步时钟的差值

    /* skip or repeat frame. We take into account the
       delay to compute the threshold. I still don't know
       if it is the best guess */
    // 若delay < AV_SYNC_THRESHOLD_MIN，则同步域值为AV_SYNC_THRESHOLD_MIN
    // 若delay > AV_SYNC_THRESHOLD_MAX，则同步域值为AV_SYNC_THRESHOLD_MAX
    // 若AV_SYNC_THRESHOLD_MIN < delay < AV_SYNC_THRESHOLD_MAX，则同步域值为delay
    sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
    if (!isnan(diff)) {
        if (diff <= -sync_threshold)        // 视频时钟落后于同步时钟，且超过同步域值
            delay = FFMAX(0, delay + diff); // 当前帧播放时刻落后于同步时钟(delay+diff<0)则delay=0(视频追赶，立即播放)，否则delay=delay+diff
        else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)  // 视频时钟超前于同步时钟，且超过同步域值，但上一帧播放时长超长
            delay = delay + diff;           // 仅仅校正为delay=delay+diff，主要是AV_SYNC_FRAMEDUP_THRESHOLD参数的作用
        else if (diff >= sync_threshold)    // 视频时钟超前于同步时钟，且超过同步域值
            delay = 2 * delay;              // 视频播放要放慢脚步，delay扩大至2倍
    }

    av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n", delay, -diff);
    // printf("video: delay=%0.3f A-V=%f\n", delay, -diff);

    return delay;
}

static double vp_duration(player_stat_t *is, frame_t *vp, frame_t *nextvp) {
    if (vp->serial == nextvp->serial) {
        double duration = nextvp->pts - vp->pts;
        if (isnan(duration) || duration <= 0)
            return vp->duration;
        else
            return duration;
    } else {
        return 0.0;
    }
}

static void update_video_pts(player_stat_t *is, double pts, int64_t pos, int serial) {
    /* update current video pts */
    set_clock(&is->video_clk, pts, serial);            // 更新vidclock
    //-sync_clock_to_slave(&is->extclk, &is->vidclk);  // 将extclock同步到vidclock
}

static void video_display(player_stat_t *is)
{
    frame_t *vp;
    // struct timeval now;
    // static struct timeval last = {0};

    if (is->video_frm_queue.size == 0)
        return;

    vp = frame_queue_peek_last(&is->video_frm_queue);
    //printf("get vp disp ridx: %d,format: %d\n",is->video_frm_queue.rindex,vp->frame->format);

    // 图像转换：p_frm_raw->data ==> p_frm_yuv->data
    // 将源图像中一片连续的区域经过处理后更新到目标图像对应区域，处理的图像区域必须逐行连续
    // plane: 如YUV有Y、U、V三个plane，RGB有R、G、B三个plane
    // slice: 图像中一片连续的行，必须是连续的，顺序由顶部到底部或由底部到顶部
    // stride/pitch: 一行图像所占的字节数，Stride=BytesPerPixel*Width+Padding，注意对齐
    // AVFrame.*data[]: 每个数组元素指向对应plane
    // AVFrame.linesize[]: 每个数组元素表示对应plane中一行图像所占的字节数

    #if (0)
    if (is->decode_type == SOFT_DECODING) {
        sws_scale(is->img_convert_ctx,                    // sws context
                  (const uint8_t *const *)vp->frame->data,// src slice
                  vp->frame->linesize,                    // src stride
                  0,                                      // src slice y
                  is->p_vcodec_ctx->height,               // src slice height
                  is->p_frm_yuv->data,                    // dst planes
                  is->p_frm_yuv->linesize                 // dst strides
                  );

        //printf("displayvideo callback, w=%d, h=%d\n", is->p_vcodec_ctx->width, is->p_vcodec_ctx->height);
        if (is->playerController.fpDisplayVideo)
            is->playerController.fpDisplayVideo(vp->frame->width, vp->frame->height, is->p_frm_yuv->data[0], is->p_frm_yuv->data[1]);
    } else
    #endif
    if (is->decode_type == HARD_DECODING) {
        if (vp->frame->format == AV_PIX_FMT_YUYV422) {
            if (is->playerController.fpDisplayVideoYuv422)
                is->playerController.fpDisplayVideoYuv422(vp->frame->width,
                    vp->frame->height, vp->frame->data[0]);
        } 
        else if (vp->frame->format == AV_PIX_FMT_YUVJ422P) 
		{
            if (is->playerController.fpDisplayVideoYuv422p)
                is->playerController.fpDisplayVideoYuv422p(vp->frame->width,
                    vp->frame->height, vp->frame->data[0], vp->frame->data[1], vp->frame->data[2]);
        } 
		else {
            if (is->playerController.fpDisplayVideo)
                is->playerController.fpDisplayVideo(vp->frame->width,
                    vp->frame->height, vp->frame->data[0], vp->frame->data[1], vp->frame->data[2]);
        }
    }
#if 0
    gettimeofday(&now, NULL);
    if (now.tv_sec != last.tv_sec) {
        int i =0;
        AVPixFmtDescriptor *desc = (AVPixFmtDescriptor*)av_pix_fmt_desc_get(is->p_vcodec_ctx->pix_fmt);
        for (i = 0; i < 4; i++) {
            int plane = desc->comp[i].plane;
            printf("{%08x,%d}",vp->frame->data[plane],vp->frame->linesize[plane]);
            if (!vp->frame->data[plane] || !vp->frame->linesize[plane])
                printf(" Qsize=%d ",is->video_frm_queue.size);
                //return 0;
        }
        printf("0x%08x,0x%08x,0x%08x[0,%d] [0x%08x,0x%08x]fmt:%d\n",is->img_convert_ctx,vp->frame->data,vp->frame->linesize,is->p_vcodec_ctx->height,
            is->p_frm_yuv->data,is->p_frm_yuv->linesize,vp->frame->format);
        last = now;//
    }
#endif

    is->video_disp_cnt++;
}

/* called to display each frame */
static void video_refresh(void *opaque, double *remaining_time, bool flush)
{
    player_stat_t *is = (player_stat_t *)opaque;
    double time;
    static bool first_frame = true;

retry:
    // 暂停处理：不停播放上一帧图像
    if (is->paused)
        goto display;

    // 所有帧已显示
    if (frame_queue_nb_remaining(&is->video_frm_queue) <= 0) {
        // if file is eof and there is no paket in queue, then do complete
        if (!is->abort_request && is->eof && is->video_pkt_queue.nb_packets == 0 && is->video_disp_cnt > 0) {
            if (is->video_complete == 0) {
                printf("[video] complete\n");
            }
            is->video_complete = 1;
        }
        return;
    }

    if (flush) {
        while (frame_queue_nb_remaining(&is->video_frm_queue) > 0) {
            frame_queue_next(&is->video_frm_queue);
        }
        return;
    }

    double last_duration, duration, delay;
    frame_t *vp, *lastvp;

    /* dequeue the picture */
    lastvp = frame_queue_peek_last(&is->video_frm_queue);     // 上一帧：上次已显示的帧
    vp = frame_queue_peek(&is->video_frm_queue);              // 当前帧：当前待显示的帧
    //printf("refresh ridx: %d,rs:%d,widx: %d,size: %d,maxsize: %d\n",is->video_frm_queue.rindex,is->video_frm_queue.rindex_shown,is->video_frm_queue.windex,is->video_frm_queue.size,is->video_frm_queue.max_size);
    //printf("lastpos: %ld,lastpts: %lf,vppos: %ld,vppts: %lf\n",lastvp->pos,lastvp->pts,vp->pos,vp->pts);
    // lastvp和vp不是同一播放序列(一个seek会开始一个新播放序列)，将frame_timer更新为当前时间
    if (first_frame) {
        is->frame_timer = av_gettime_relative() / 1000000.0;
        first_frame = false;
    }

    /* compute nominal last_duration */
    last_duration = vp_duration(is, lastvp, vp);        // 上一帧播放时长：vp->pts - lastvp->pts
    delay = compute_target_delay(last_duration, is);    // 根据视频时钟和同步时钟的差值，计算delay值
    time = av_gettime_relative() / 1000000.0;

    if (is->speed_x) {
        double disp_interval = 1.0 / (1 << is->speed_x);
        if (time < is->frame_timer + disp_interval) {
            // printf("current: %.2f, last: %.2f\n", time, is->frame_timer);
            *remaining_time = FFMIN((is->frame_timer + disp_interval), *remaining_time);
            return;
        }
        // printf("current: %.2f, des: %.2f\n", time, (is->frame_timer + disp_interval));
        is->frame_timer = time;
        set_clock(&is->audio_clk, vp->pts, vp->serial);
    } else {
        // printf("time [%f] last [%f] delay [%f] last_duration [%f]\n", time, is->frame_timer, delay, last_duration);
        // 当前帧播放时刻(is->frame_timer+delay)大于当前时刻(time)，表示播放时刻未到
        if (time < is->frame_timer + delay) {
            // 播放时刻未到，则更新刷新时间remaining_time为当前时刻到下一播放时刻的时间差
            *remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
            // 播放时刻未到，则不播放，直接返回
            // printf("NP\n");
            return;
        }
        // 更新frame_timer值
        is->frame_timer += delay;
    }

    // 校正frame_timer值：若frame_timer落后于当前系统时间太久(超过最大同步域值)，则更新为当前系统时间
    if (delay > 0 && ((time - is->frame_timer) > AV_SYNC_THRESHOLD_MAX)) {
        is->frame_timer = time;
    }

    pthread_mutex_lock(&is->video_frm_queue.mutex);
    if (!isnan(vp->pts)) {
        update_video_pts(is, vp->pts, vp->pos, vp->serial); // 更新视频时钟：时间戳、时钟时间
    }
    pthread_mutex_unlock(&is->video_frm_queue.mutex);

    //if (is->speed_x)
    //    printf("frame pts : %.2f. clock pts : %.2f[%lld,%d,%d]\n", vp->pts, is->video_clk.pts,vp->pos,vp->serial,vp->frame->key_frame);

    // update ui pos
    if (is->speed_x == SPEED_NORMAL && is->playerController.fpGetCurrentPlayPosFromVideo) {
        long long videoPts = (long long)(is->video_clk.pts * 1000000LL);
        AVRational frame_rate = av_guess_frame_rate(is->p_fmt_ctx, is->p_video_stream, NULL);
        long long frame_duration = 1000000LL * (long long)frame_rate.den / frame_rate.num;
        is->playerController.fpGetCurrentPlayPosFromVideo(videoPts, frame_duration);
    } else if (is->speed_x != SPEED_NORMAL && is->playerController.fpGetCurrentPlayPos) {
        long long videoPts = (long long)(is->video_clk.pts * 1000000LL);
        AVRational frame_rate = av_guess_frame_rate(is->p_fmt_ctx, is->p_video_stream, NULL);
        long long frame_duration = 1000000LL * (long long)frame_rate.den / frame_rate.num;
        is->playerController.fpGetCurrentPlayPos(videoPts, frame_duration);
    }

    // 是否要丢弃未能及时播放的视频帧
    if (frame_queue_nb_remaining(&is->video_frm_queue) > 1) {  // 队列中未显示帧数>1(只有一帧则不考虑丢帧)
        frame_t *nextvp = frame_queue_peek_next(&is->video_frm_queue);  // 下一帧：下一待显示的帧
        duration = vp_duration(is, vp, nextvp);             // 当前帧vp播放时长 = nextvp->pts - vp->pts
        // 当前帧vp未能及时播放，即下一帧播放时刻(is->frame_timer+duration)小于当前系统时刻(time)
        if (time > is->frame_timer + duration)
        {
            frame_queue_next(&is->video_frm_queue);   // 删除上一帧已显示帧，即删除lastvp，读指针加1(从lastvp更新到vp)
            goto retry;
        }
    }
    // 删除当前读指针元素，读指针+1。若未丢帧，读指针从lastvp更新到vp；若有丢帧，读指针从vp更新到nextvp
    frame_queue_next(&is->video_frm_queue);

    if (is->step && !is->paused) {
        stream_toggle_pause(is);
    }

display:
    video_display(is);                      // 取出当前帧vp(若有丢帧是nextvp)进行播放
}

static void* video_playing_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    double remaining_time = 0.0;
    int speed_x = is->speed_x;
    bool flush;

    while (1) {
        if (is->abort_request) {
            break;
        }

        if (remaining_time > 0.0) {
            //printf("delay time: %lf\n",remaining_time);
            av_usleep((unsigned)(remaining_time * 1000000.0));
        }

        if (speed_x == SPEED_NORMAL && is->speed_x != SPEED_NORMAL)
            flush = TRUE;
        else
            flush = FALSE;

        speed_x = is->speed_x;

        // 立即显示当前帧，或延时remaining_time后再显示
        remaining_time = REFRESH_RATE;
        video_refresh(is, &remaining_time, flush);
    }

    printf("[video] play thread exit\n");
    return NULL;
}

static int open_video_playing(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    int ret;
    int buf_size;
    AVPixFmtDescriptor *desc;

    // 为AVFrame.*data[]手工分配缓冲区，用于存储sws_scale()中目的帧视频数据
    buf_size = av_image_get_buffer_size(AV_PIX_FMT_NV12,
                                        is->p_vcodec_ctx->width,
                                        is->p_vcodec_ctx->height,
                                        1
                                        );

    // p_frm_buf_ref->data将作为p_frm_yuv的视频数据缓冲区
    is->p_frm_buf_ref = av_buffer_alloc_frame(buf_size);
    if (is->p_frm_buf_ref == NULL) {
        printf("av_buffer_alloc_frame() failed\n");
        return -1;
    }

    // 使用给定参数设定p_frm_yuv->data和p_frm_yuv->linesize
    ret = av_image_fill_arrays(is->p_frm_yuv->data,     // @param dst_data      data pointers to be filled in
                               is->p_frm_yuv->linesize, // @param dst_linesize  linesizes for the image in dst_data to be filled in
                               is->p_frm_buf_ref->data, // @param src           buffer which will contain or contains the actual image data, can be NULL
                               AV_PIX_FMT_NV12,         // @param pix_fmt       the pixel format of the image
                               is->p_vcodec_ctx->width, // @param width         the width of the image in pixels
                               is->p_vcodec_ctx->height,// @param height        the height of the image in pixels
                               1                        // @param align         the value used in src for linesize alignment
                               );
    if (ret < 0) {
        printf("av_image_fill_arrays() failed %d\n", ret);
        return -1;
    }

    // Convert for "deprecated pixel format used, make sure you did set range correctly"
    switch (is->p_vcodec_ctx->pix_fmt) {
    case AV_PIX_FMT_YUVJ420P:
        is->p_vcodec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    break;
    case AV_PIX_FMT_YUVJ422P:
        is->p_vcodec_ctx->pix_fmt = AV_PIX_FMT_YUV422P;
    break;
    case AV_PIX_FMT_YUVJ444P:
        is->p_vcodec_ctx->pix_fmt = AV_PIX_FMT_YUV444P;
    break;
    case AV_PIX_FMT_YUVJ440P:
        is->p_vcodec_ctx->pix_fmt = AV_PIX_FMT_YUV440P;
    break;
    default:
    break;
    }

    #if (0)
    // A2. 初始化SWS context，用于后续图像转换
    //     此处第6个参数使用的是FFmpeg中的像素格式，对比参考注释B3
    //     FFmpeg中的像素格式AV_PIX_FMT_YUV420P对应SDL中的像素格式SDL_PIXELFORMAT_IYUV
    //     如果解码后得到图像的不被SDL支持，不进行图像转换的话，SDL是无法正常显示图像的
    //     如果解码后得到图像的能被SDL支持，则不必进行图像转换
    //     这里为了编码简便，统一转换为SDL支持的格式AV_PIX_FMT_YUV420P==>SDL_PIXELFORMAT_IYUV
    is->img_convert_ctx = sws_getContext(is->p_vcodec_ctx->width,   // src width
                                         is->p_vcodec_ctx->height,  // src height
                                         is->p_vcodec_ctx->pix_fmt, // src format
                                         is->p_vcodec_ctx->width,   // dst width
                                         is->p_vcodec_ctx->height,  // dst height
                                         AV_PIX_FMT_NV12,           // dst format
                                         SWS_FAST_BILINEAR,         // flags
                                         NULL,                      // src filter
                                         NULL,                      // dst filter
                                         NULL                       // param
                                         );
    if (is->img_convert_ctx == NULL) {
        printf("sws_getContext() failed\n");
        return -1;
    }
    #endif

    desc = (AVPixFmtDescriptor*)av_pix_fmt_desc_get(is->p_vcodec_ctx->pix_fmt);
    printf("convert [%s] res [%dx%d]\n", desc->name, is->p_vcodec_ctx->width, is->p_vcodec_ctx->height);

    CheckFuncResult(pthread_create(&is->videoPlay_tid, NULL, video_playing_thread, is));
    return 0;
}

static int open_video_stream(player_stat_t *is)
{
    AVCodecParameters* p_codec_par = NULL;
    AVCodec* p_codec = NULL;
    AVCodecContext* p_codec_ctx = NULL;
    AVStream *p_stream = is->p_video_stream;
    int ret;

    // 1. 为视频流构建解码器AVCodecContext
    // 1.1 获取解码器参数AVCodecParameters
    p_codec_par = p_stream->codecpar;

    // 1.2 获取解码器
    // 对于h264/h265编码的视频选择硬解,其他编码格式使用ffmpeg软解
    switch (p_codec_par->codec_id) {
    case AV_CODEC_ID_H264 :
        p_codec = avcodec_find_decoder_by_name("h264");
        is->decode_type = HARD_DECODING;
        break;

    case AV_CODEC_ID_HEVC :
        p_codec = avcodec_find_decoder_by_name("hevc");
        is->decode_type = HARD_DECODING;
        break;

    case AV_CODEC_ID_MJPEG :
        p_codec = avcodec_find_decoder_by_name("mjpeg");
        is->decode_type = HARD_DECODING;
        break;

    default :
        p_codec = avcodec_find_decoder(p_codec_par->codec_id);
        is->decode_type = HARD_DECODING;
        break;
    }

    if (p_codec == NULL) {
        printf("Can't find codec!\n");
        return -1;
    }

    printf("open codec [%s]\n", p_codec->name);

    // 1.3 构建解码器AVCodecContext
    // 1.3.1 p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
    p_codec_ctx = avcodec_alloc_context3(p_codec);
    if (p_codec_ctx == NULL) {
        printf("avcodec_alloc_context3() failed\n");
        return -1;
    }

    // 1.3.2 p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
    ret = avcodec_parameters_to_context(p_codec_ctx, p_codec_par);
    if (ret < 0) {
        printf("avcodec_parameters_to_context() failed\n");
        return -1;
    }

    // 1.3.3 p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
    ret = avcodec_open2(p_codec_ctx, p_codec, NULL);
    if (ret < 0) {
        printf("avcodec_open2() failed\n");
        return -1;
    }

    is->p_vcodec_ctx = p_codec_ctx;
    is->p_vcodec_ctx->debug = FF_DEBUG_PICT_INFO;
    is->p_vcodec_ctx->flags = 0;

    // 2. 创建视频解码线程
    CheckFuncResult(pthread_create(&is->videoDecode_tid, NULL, video_decode_thread, is));

    return 0;
}

int open_video(player_stat_t *is)
{
    int ret;

    if (is && is->video_idx >= 0) {
        ret = open_video_stream(is);
        if (ret < 0) {
            printf("open_video_stream failed\n");
            return -1;
        }

        ret = open_video_playing(is);
        if (ret < 0) {
            printf("open_video_playing failed\n");
            return -1;
        }
    }

    return 0;
}
#endif
