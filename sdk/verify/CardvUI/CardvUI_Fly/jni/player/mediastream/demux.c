#ifdef SUPPORT_PLAYER_MODULE
#include "demux.h"
#include "packet.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static int decode_interrupt_cb(void *ctx)
{
    player_stat_t *is = (player_stat_t *)ctx;
    return is->abort_request;
}

static int demux_init(player_stat_t *is)
{
    AVFormatContext *p_fmt_ctx = NULL;
    int err = 0, i, ret;
    double video_duration = 0.0, audio_duration = 0.0;

    p_fmt_ctx = avformat_alloc_context();
    if (!p_fmt_ctx) {
        printf("Could not allocate context.\n");
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    // 中断回调机制。为底层I/O层提供一个处理接口，比如中止IO操作。
    p_fmt_ctx->interrupt_callback.callback = decode_interrupt_cb;
    p_fmt_ctx->interrupt_callback.opaque = is;

    // 1. 构建AVFormatContext
    // 1.1 打开视频文件：读取文件头，将文件格式信息存储在"fmt context"中
    err = avformat_open_input(&p_fmt_ctx, is->filename, NULL, NULL);
    if (err < 0) {
        printf("avformat_open_input() failed\n");
        ret = -1;
        goto fail;
    }

    is->p_fmt_ctx = p_fmt_ctx;

    // 1.2 Find which video / audio stream will be decoded.
    is->audio_idx = -1;
    is->video_idx = -1;
    for (i = 0; i < (int)p_fmt_ctx->nb_streams; i++) {
        AVStream * stream = p_fmt_ctx->streams[i];

        // Recude VmPeak for mpegts
        // If request_probe > 0, av_read_frame will search all file until read size > RAW_PACKET_BUFFER_SIZE,
        // And avformat_find_stream_info will search all file until read size > probesize.
        stream->request_probe = 0;

        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
            is->audio_idx == -1) {
            is->audio_idx = i;
        }
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            is->video_idx = i; // use the last stream
        }
    }

    if (is->audio_idx == -1 && is->video_idx == -1) {
        printf("Can't find audio & video stream\n");
        ret = -1;
 fail:
        // Release demux resource in player_deinit().
        if (is->playerController.fpPlayError)
            is->playerController.fpPlayError(err);
        return ret;
    } else {
        // avformat_find_stream_info() will try decode frame to find codec info of all streams.
        // but we don't need this stream (EX. 4K/1080p resolution stream) codec info.
        for (i = 0; i < (int)p_fmt_ctx->nb_streams; i++) {
            if (i != is->audio_idx && i != is->video_idx)
                p_fmt_ctx->streams[i]->need_codec_info = 0;
        }
    }

    printf("audio idx [%d] video idx [%d]\n", is->audio_idx, is->video_idx);

    // 1.3 搜索流信息：读取一段视频文件数据，尝试解码，将取到的流信息填入p_fmt_ctx->streams
    //     ic->streams是一个指针数组，数组大小是pFormatCtx->nb_streams
    err = avformat_find_stream_info(p_fmt_ctx, NULL);
    if (err < 0) {
        printf("avformat_find_stream_info() failed\n");
        ret = -1;
        goto fail;
    }

    av_dump_format(p_fmt_ctx, 0, is->filename, 0);

    if (is->audio_idx != -1) {
        AVStream * stream = p_fmt_ctx->streams[is->audio_idx];
        if (!stream->codecpar->format ||
            !stream->codecpar->sample_rate ||
            !stream->codecpar->channels)
            is->audio_idx = -1;
    }

    if (is->audio_idx == -1 && is->video_idx == -1) {
        ret = -1;
        goto fail;
    }

    // get media duration
    if (is->playerController.fpGetDuration)
        is->playerController.fpGetDuration(is->p_fmt_ctx->duration);

    if (is->audio_idx >= 0) {
        is->p_audio_stream = p_fmt_ctx->streams[is->audio_idx];
        // printf("audio codec_info_nb_frames:%d, nb_frames:%lld, probe_packet:%d\n", is->p_audio_stream->codec_info_nb_frames, is->p_audio_stream->nb_frames, is->p_audio_stream->probe_packets);
        // printf("audio duration:%lld(%d,%d), nb_frames:%lld\n", is->p_audio_stream->duration, is->p_audio_stream->time_base.num, is->p_audio_stream->time_base.den, is->p_audio_stream->nb_frames);
        audio_duration = (double)is->p_audio_stream->duration * (double)is->p_audio_stream->time_base.num / (double)is->p_audio_stream->time_base.den;
    }
    if (is->video_idx >= 0) {
        is->p_video_stream = p_fmt_ctx->streams[is->video_idx];
        // printf("video codec_info_nb_frames:%d, nb_frames:%lld, probe_packet:%d\n", is->p_video_stream->codec_info_nb_frames, is->p_video_stream->nb_frames, is->p_video_stream->probe_packets);
        // printf("video duration:%lld(%d,%d), nb_frames:%lld\n", is->p_video_stream->duration, is->p_video_stream->time_base.num, is->p_video_stream->time_base.den, is->p_video_stream->nb_frames);
        video_duration = (double)is->p_video_stream->duration * (double)is->p_video_stream->time_base.num / (double)is->p_video_stream->time_base.den;
    }

    printf("video duration [%f]\n", video_duration);
    printf("audio duration [%f]\n", audio_duration);

    // set GetCurPlayPos callback
    if (video_duration >= audio_duration) {
        is->playerController.fpGetCurrentPlayPosFromVideo = is->playerController.fpGetCurrentPlayPos;
        // printf("Play pos from video\n");
    } else {
        is->playerController.fpGetCurrentPlayPosFromAudio = is->playerController.fpGetCurrentPlayPos;
        // printf("Play pos from audio\n");
    }

    return 0;
}

int demux_deinit()
{
    // Release demux resource in player_deinit().
    return 0;
}

static int stream_has_enough_packets(AVStream *st, int stream_id, packet_queue_t *queue)
{
    //printf("id: %d,disposition: %d,nb_packets: %d\n",stream_id,st->disposition,queue->nb_packets);
    return stream_id < 0 ||
           queue->abort_request ||
           (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
           (queue->nb_packets > MIN_FRAMES);
}

static void step_to_next_frame(player_stat_t *is)
{
    /* if the stream is paused unpause it, then step */
    if (is->paused)
        stream_toggle_pause(is);
    is->step = 1;
}

/* this thread gets the stream from the disk or the network */
static void* demux_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    int ret;
    AVPacket pkt1, *pkt = &pkt1;

    struct timeval now;
    struct timespec outtime;

    pthread_mutex_t wait_mutex;

    if (pthread_mutex_init(&wait_mutex, NULL) != SUCCESS) {
        printf("demux_thread failed\n");
        return NULL;
    }

    av_init_packet(is->p_flush_pkt);
    is->p_flush_pkt->data = (uint8_t *)is->p_flush_pkt;
    if (is->video_idx >= 0)
        packet_queue_put(&is->video_pkt_queue, is->p_flush_pkt);
    if (is->audio_idx >= 0)
        packet_queue_put(&is->audio_pkt_queue, is->p_flush_pkt);

    is->eof = 0;
    is->audio_complete = 0;
    is->video_complete = 0;
    // 4. 解复用处理
    while (1) {
        if (is->abort_request) {
            break;
        }

        if (is->eof) {
            /* wait 10 ms */
            pthread_mutex_lock(&wait_mutex);
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec;
            outtime.tv_nsec = now.tv_usec * 1000 + 10 * 1000 * 1000;//timeout 10ms
            pthread_cond_timedwait(&is->continue_read_thread, &wait_mutex, &outtime);
            pthread_mutex_unlock(&wait_mutex);
            continue;
        }

        if (is->paused != is->last_paused) {
            is->last_paused = is->paused;
            if (is->paused)
                is->read_pause_return = av_read_pause(is->p_fmt_ctx);
            else
                av_read_play(is->p_fmt_ctx);
        }

        if (is->paused) {
            /* wait 10 ms */
            pthread_mutex_lock(&wait_mutex);
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec;
            outtime.tv_nsec = now.tv_usec * 1000 + 10 * 1000 * 1000;//timeout 10ms
            pthread_cond_timedwait(&is->continue_read_thread, &wait_mutex, &outtime);
            pthread_mutex_unlock(&wait_mutex);
            continue;
        }

        if (is->seek_req) {
            int64_t seek_target = is->seek_pos;
            int64_t seek_min    = is->seek_rel > 0 ? seek_target - is->seek_rel + 2: INT64_MIN;
            int64_t seek_max    = is->seek_rel < 0 ? seek_target - is->seek_rel - 2: INT64_MAX;
            int stream_index    = is->video_idx >= 0 ? is->video_idx : is->audio_idx;

            if (!(is->seek_flags & AVSEEK_FLAG_BYTE)) {
                seek_target = seek_target * is->p_fmt_ctx->streams[stream_index]->time_base.den / is->p_fmt_ctx->streams[stream_index]->time_base.num / 1000000;
            }

            // FIXME the +-2 is due to rounding being not done in the correct direction in generation
            // of the seek_pos/seek_rel variables

            ret = avformat_seek_file(is->p_fmt_ctx, stream_index, seek_min, seek_target, seek_max, is->seek_flags);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR,
                       "%s: error while seeking\n", is->p_fmt_ctx->url);
            } else {
                if (is->audio_idx >= 0) {
                    packet_queue_flush(&is->audio_pkt_queue);
                    packet_queue_put(&is->audio_pkt_queue, is->p_flush_pkt);
                }
                if (is->video_idx >= 0) {
                    packet_queue_flush(&is->video_pkt_queue);
                    packet_queue_put(&is->video_pkt_queue, is->p_flush_pkt);
                }
                /*
                if (is->seek_flags & AVSEEK_FLAG_BYTE) {
                   set_clock(&is->extclk, NAN, 0);
                } else {
                   set_clock(&is->extclk, seek_target / (double)AV_TIME_BASE, 0);
                }
                */
            }
            is->seek_req = 0;
            is->seek_req_not_done = 1;
            is->eof = 0;
            if (is->paused)
                step_to_next_frame(is);
        }

        /* if the queue are full, no need to read more */
        if (is->video_idx >= 0) {
            if (/*is->audio_pkt_queue.size + is->video_pkt_queue.size > MAX_QUEUE_SIZE ||*/
                /*(stream_has_enough_packets(is->p_audio_stream, is->audio_idx, &is->audio_pkt_queue) &&*/
                stream_has_enough_packets(is->p_video_stream, is->video_idx, &is->video_pkt_queue)) {
                /* wait 10 ms */
                pthread_mutex_lock(&wait_mutex);
                gettimeofday(&now, NULL);
                outtime.tv_sec = now.tv_sec;
                outtime.tv_nsec = now.tv_usec * 1000 + 10 * 1000 * 1000;//timeout 10ms
                // printf("queue size: %d\n",is->audio_pkt_queue.size + is->video_pkt_queue.size);
                // printf("wait video queue avalible pktnb: %d\n",is->video_pkt_queue.nb_packets);
                // printf("wait audio queue avalible pktnb: %d\n",is->audio_pkt_queue.nb_packets);
                pthread_cond_timedwait(&is->continue_read_thread, &wait_mutex, &outtime);
                pthread_mutex_unlock(&wait_mutex);
                continue;
            }
        } else {
             if (stream_has_enough_packets(is->p_audio_stream, is->audio_idx, &is->audio_pkt_queue)) {
                /* wait 10 ms */
                pthread_mutex_lock(&wait_mutex);
                gettimeofday(&now, NULL);
                outtime.tv_sec = now.tv_sec;
                outtime.tv_nsec = now.tv_usec * 1000 + 10 * 1000 * 1000;//timeout 10ms
                // printf("queue size: %d\n",is->audio_pkt_queue.size + is->video_pkt_queue.size);
                // printf("wait video queue avalible pktnb: %d\n",is->video_pkt_queue.nb_packets);
                // printf("wait audio queue avalible pktnb: %d\n",is->audio_pkt_queue.nb_packets);
                pthread_cond_timedwait(&is->continue_read_thread, &wait_mutex, &outtime);
                pthread_mutex_unlock(&wait_mutex);
                continue;
            }
        }

        if (is->speed_x && (is->recv_frame_cnt < is->send_frame_cnt || is->video_pkt_queue.nb_packets > 0)) {
            /* wait 10 ms */
            // printf("wait++ n[%d]\n", is->video_pkt_queue.nb_packets);
            pthread_mutex_lock(&wait_mutex);
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec;
            outtime.tv_nsec = now.tv_usec * 1000 + 10 * 1000 * 1000;//timeout 10ms
            pthread_cond_timedwait(&is->continue_read_thread, &wait_mutex, &outtime);
            pthread_mutex_unlock(&wait_mutex);
            // printf("wait-- n[%d]\n", is->video_pkt_queue.nb_packets);
            if (is->recv_frame_cnt + 1 == is->send_frame_cnt && is->video_pkt_queue.nb_packets == 0)
                packet_queue_put_nullpacket(&is->video_pkt_queue, is->video_idx);
            continue;
        }

        // 4.1 从输入文件中读取一个packet
        ret = av_read_frame(is->p_fmt_ctx, pkt);
        if (ret < 0) {
            if (((ret == AVERROR_EOF) || avio_feof(is->p_fmt_ctx->pb)) && !is->eof) {
                is->eof = 1;
                // 输入文件已读完，则往packet队列中发送NULL packet，以冲洗(flush)解码器，否则解码器中缓存的帧取不出来
                if (is->video_idx >= 0 && is->recv_frame_cnt != is->send_frame_cnt)
                    packet_queue_put_nullpacket(&is->video_pkt_queue, is->video_idx);
                if (is->audio_idx >= 0)
                    packet_queue_put_nullpacket(&is->audio_pkt_queue, is->audio_idx);
            }

            pthread_mutex_lock(&wait_mutex);
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec;
            outtime.tv_nsec = now.tv_usec * 1000 + 10 * 1000 * 1000;//timeout 10ms
            pthread_cond_timedwait(&is->continue_read_thread, &wait_mutex, &outtime);
            pthread_mutex_unlock(&wait_mutex);
            continue;
        } else {
            is->eof = 0;
        }

        if (is->seek_req_not_done) {
            if (pkt->stream_index == is->audio_idx)
                av_packet_unref(pkt);
            else if (pkt->stream_index == is->video_idx) {
                if (pkt->flags & AV_PKT_FLAG_KEY) {
                    is->seek_req_not_done = 0;
                    packet_queue_put(&is->video_pkt_queue, pkt);
                } else
                    av_packet_unref(pkt);
            } else
                av_packet_unref(pkt);
        } else if (is->speed_x) {
            if ((pkt->stream_index == is->video_idx) && (pkt->flags & AV_PKT_FLAG_KEY)) {
                packet_queue_put(&is->video_pkt_queue, pkt);
                if (is->fast_dir == FAST_BACKWARD) {
                    av_seek_frame(is->p_fmt_ctx, is->video_idx, pkt->pts - is->keyframe_interval, AVSEEK_FLAG_BACKWARD);
                    if (pkt->pts == 0) {
                        is->eof = 1;
                    }
                }
            } else {
                av_packet_unref(pkt);
            }
        } else {
            if (pkt->stream_index == is->audio_idx) {
                packet_queue_put(&is->audio_pkt_queue, pkt);
            } else if (pkt->stream_index == is->video_idx) {
                packet_queue_put(&is->video_pkt_queue, pkt);
                if (pkt->flags & AV_PKT_FLAG_KEY) {
                    if (is->last_keyframe_pts)
                        is->keyframe_interval = pkt->pts - is->last_keyframe_pts;
                    is->last_keyframe_pts = pkt->pts;
                }
            } else {
                av_packet_unref(pkt);
            }
        }
    }

    pthread_mutex_destroy(&wait_mutex);
    printf("demux thread exit\n");
    return NULL;
}

int open_demux(player_stat_t *is)
{
    if (demux_init(is) != 0) {
        printf("demux_init() failed\n");
        return -1;
    }

    CheckFuncResult(pthread_create(&is->read_tid, NULL, demux_thread, is));
    return 0;
}

int demux_thumbnail(char *src_filename, char *dst_filename)
{
    AVFormatContext *p_fmt_ctx = NULL;
    AVPacket pkt;
    int fd = 0;
    int ret = 0;
    unsigned int stream_idx = 0;
    int thumb_idx = 0;

    ret = access(dst_filename, F_OK);
    if (ret == 0) {
        unlink(dst_filename);
    }

    /* open input file, and allocate format context */
    if (avformat_open_input(&p_fmt_ctx, src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        return -1;
    }

    for (stream_idx = 0; stream_idx < p_fmt_ctx->nb_streams; stream_idx++) {
        AVStream * stream = p_fmt_ctx->streams[stream_idx];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_DATA) {
            thumb_idx = stream_idx; // First AVMEDIA_TYPE_DATA stream is thumbnail.
            break;
        }
    }

    for (stream_idx = 0; stream_idx < p_fmt_ctx->nb_streams; stream_idx++) {
        AVStream * stream = p_fmt_ctx->streams[stream_idx];
        stream->request_probe = 0;
    }

    /* dump input information to stderr */
    // av_dump_format(p_fmt_ctx, 0, src_filename, 0);

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    /* read frames from the file */
    while (av_read_frame(p_fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == thumb_idx) {
            if (pkt.size > 4 &&
                pkt.data[0] == 0xFF && pkt.data[1] == 0xD8 &&
                pkt.data[pkt.size-2] == 0xFF && pkt.data[pkt.size-1] == 0xD9) {
                fd = open(dst_filename,  O_WRONLY | O_CREAT, 0666);
                if (fd) {
                    write(fd, pkt.data, pkt.size);
                    close(fd);
                }
            }
            av_packet_unref(&pkt);
            break;
        }
        av_packet_unref(&pkt);
    }

    avformat_close_input(&p_fmt_ctx);
    return ret;
}

int64_t demux_duration(char *src_filename)
{
    AVFormatContext *p_fmt_ctx = NULL;
    unsigned int stream_idx = 0;
    int64_t duration = -1;

    /* open input file, and allocate format context */
    if (avformat_open_input(&p_fmt_ctx, src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        return -1;
    }

    if (p_fmt_ctx->duration != AV_NOPTS_VALUE) {
        goto L_EXIT;
    }

    for (stream_idx = 0; stream_idx < p_fmt_ctx->nb_streams; stream_idx++) {
        AVStream * stream = p_fmt_ctx->streams[stream_idx];
        stream->request_probe = 0;
        // Don't nead codec info, just need duration.
        stream->need_codec_info = 0;
    }

    if (avformat_find_stream_info(p_fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream %s\n", src_filename);
        goto L_ERR;
    }

    // av_dump_format(p_fmt_ctx, 0, src_filename, 0);

L_EXIT:
    duration = p_fmt_ctx->duration;
    // printf("[%s] duration [%lld]\n", src_filename, duration);
L_ERR:
    avformat_close_input(&p_fmt_ctx);
    return duration;
}

#endif
