/*******************************************************************************
 * player.c
 *
 * history:
 *   2018-11-27 - [lei]     Create file: a simplest ffmpeg player
 *   2018-12-01 - [lei]     Playing audio
 *   2018-12-06 - [lei]     Playing audio & video
 *   2019-01-06 - [lei]     Add audio resampling, fix bug of unsupported audio
 *                          format(such as planar)
 *   2019-01-16 - [lei]     Sync video to audio.
 *
 * details:
 *   A simple ffmpeg player.
 *
 * refrence:
 *   ffplay.c in FFmpeg 4.1 project.
 *******************************************************************************/
#ifdef SUPPORT_PLAYER_MODULE
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "player.h"
#include "frame.h"
#include "packet.h"
#include "demux.h"
#include "videostream.h"
#include "audiostream.h"
#include "mi_vdec.h"

static pthread_mutex_t _player_mutex = PTHREAD_MUTEX_INITIALIZER;
static int av_sync_type = AV_SYNC_AUDIO_MASTER;
// static float seek_interval = 10;

// static int get_master_sync_type(player_stat_t *is) {
//     if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
//         if (is->p_video_stream)
//             return AV_SYNC_VIDEO_MASTER;
//         else
//             return AV_SYNC_AUDIO_MASTER;
//     } else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
//         if (is->p_audio_stream)
//             return AV_SYNC_AUDIO_MASTER;
//         else
//             return AV_SYNC_EXTERNAL_CLOCK;
//     } else {
//         return AV_SYNC_EXTERNAL_CLOCK;
//     }
// }

/* get the current master clock value */
// static double get_master_clock(player_stat_t *is)
// {
//     double val;
//     switch (get_master_sync_type(is)) {
//         case AV_SYNC_VIDEO_MASTER:
//             val = get_clock(&is->video_clk);
//             break;
//         case AV_SYNC_AUDIO_MASTER:
//             val = get_clock(&is->audio_clk);
//             break;
//         default:
//             val = get_clock(&is->extclk);
//             break;
//     }
//     return val;
// }

// 返回值：返回上一帧的pts更新值(上一帧pts+流逝的时间)
double get_clock(play_clock_t *c)
{
    if (*c->queue_serial != c->serial)
        return NAN;
    if (c->paused)
        return c->pts;
    else {
        double time = av_gettime_relative() / 1000000.0;
        double ret = c->pts_drift + time;   // 展开得： c->pts + (time - c->last_updated)
        return ret;
    }
}

void set_clock_at(play_clock_t *c, double pts, int serial, double time)
{
    c->pts = pts;
    c->last_updated = time;
    c->pts_drift = c->pts - time;
    c->serial = serial;
}

void set_clock(play_clock_t *c, double pts, int serial)
{
    double time = av_gettime_relative() / 1000000.0;
    set_clock_at(c, pts, serial, time);
}

void set_speed(player_stat_t *is, int speed, int dir)
{
    is->fast_dir = dir;
    is->speed_x = speed;
    if (is->speed_x == SPEED_NUM)
        is->speed_x = SPEED_NORMAL;
    if (is->speed_x) {
        if (is->fast_dir == FAST_FORWARD)
            packet_queue_flush_non_keyframe(&is->video_pkt_queue);
        else
            packet_queue_flush(&is->video_pkt_queue);
        packet_queue_flush(&is->audio_pkt_queue);
    }

    printf("set speed [%d] dir [%d]\n", is->speed_x, dir);
}

// static void set_clock_speed(play_clock_t *c, double speed)
// {
//     set_clock(c, get_clock(c), c->serial);
//     c->speed = speed;
// }

void init_clock(play_clock_t *c, int *queue_serial)
{
    c->speed = 1.0;
    c->paused = 0;
    c->queue_serial = queue_serial;
    set_clock(c, NAN, -1);
}

// static void sync_play_clock_to_slave(play_clock_t *c, play_clock_t *slave)
// {
//     double clock = get_clock(c);
//     double slave_clock = get_clock(slave);
//     if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
//         set_clock(c, slave_clock, slave->serial);
// }

static void audio_decoder_abort(player_stat_t *is)
{
    packet_queue_abort(&is->audio_pkt_queue);
    frame_queue_signal(&is->audio_frm_queue);

    if (is->audioDecode_tid)
        pthread_join(is->audioDecode_tid, NULL);
    if (is->audioPlay_tid)
        pthread_join(is->audioPlay_tid, NULL);

    packet_queue_flush(&is->audio_pkt_queue);

    if (is->p_acodec_ctx)
        avcodec_free_context(&is->p_acodec_ctx);

    if (is->audio_swr_ctx)
        swr_free(&is->audio_swr_ctx);
    if (is->audio_frm_rwr)
        av_freep(&is->audio_frm_rwr);
    is->audio_frm_size = 0;
    is->audio_frm_rwr_size = 0;
    is->audio_frm_rwr = NULL;
}

static void video_decoder_abort(player_stat_t *is)
{
    packet_queue_abort(&is->video_pkt_queue);
    frame_queue_signal(&is->video_frm_queue);

    if (is->videoDecode_tid)
        pthread_join(is->videoDecode_tid, NULL);
    if (is->videoPlay_tid)
        pthread_join(is->videoPlay_tid, NULL);

    packet_queue_flush(&is->video_pkt_queue);

    if (is->p_vcodec_ctx)
        avcodec_free_context(&is->p_vcodec_ctx);
}

static void stream_component_close(player_stat_t *is, int stream_index)
{
    AVFormatContext *p_fmt_ctx = is->p_fmt_ctx;
    AVCodecParameters *p_codec_par;

    if (!p_fmt_ctx)
        return;

    if (stream_index < 0 || stream_index >= (int)p_fmt_ctx->nb_streams)
        return;

    p_codec_par = p_fmt_ctx->streams[stream_index]->codecpar;
    switch (p_codec_par->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        audio_decoder_abort(is);
        break;
    case AVMEDIA_TYPE_VIDEO:
        video_decoder_abort(is);
        break;
    default:
        break;
    }

    p_fmt_ctx->streams[stream_index]->discard = AVDISCARD_ALL;
    switch (p_codec_par->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        is->p_audio_stream = NULL;
        is->audio_idx = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->p_video_stream = NULL;
        is->video_idx = -1;
        break;
    default:
        break;
    }
}

bool player_is_complete(player_stat_t *is)
{
    bool complete = FALSE;

    pthread_mutex_lock(&_player_mutex);
    if (is) {
        if (is->video_idx >= 0 && is->audio_idx >= 0) {
            if (is->audio_complete && is->video_complete) {
                complete = TRUE;
            }
        } else {
            if (is->audio_complete || is->video_complete) {
                complete = TRUE;
            }
        }
    }
    pthread_mutex_unlock(&_player_mutex);

    return complete;
}

player_stat_t *player_init(const char *p_input_file)
{
    player_stat_t *is;

    is = (player_stat_t *)malloc(sizeof(player_stat_t));
    if (!is) {
        return NULL;
    }

    memset(is, 0, sizeof(player_stat_t));

    is->filename = av_strdup(p_input_file);
    if (is->filename == NULL) {
        goto fail;
    }

    /* start video display */
    if (frame_queue_init(&is->video_frm_queue, &is->video_pkt_queue, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0 ||
        frame_queue_init(&is->audio_frm_queue, &is->audio_pkt_queue, SAMPLE_QUEUE_SIZE, 1) < 0) {
        goto fail;
    }

    is->p_flush_pkt = av_packet_alloc();
    if (is->p_flush_pkt == NULL) {
        goto fail;
    }

    if (packet_queue_init(&is->video_pkt_queue, is->p_flush_pkt) < 0 ||
        packet_queue_init(&is->audio_pkt_queue, is->p_flush_pkt) < 0) {
        goto fail;
    }

    if (pthread_cond_init(&is->continue_read_thread, NULL) != SUCCESS) {
        goto fail;
    }

    init_clock(&is->video_clk, &is->video_pkt_queue.serial);
    init_clock(&is->audio_clk, &is->audio_pkt_queue.serial);

    is->abort_request = 0;
    is->p_frm_buf_ref = NULL;
    is->p_frm_yuv = av_frame_alloc();
    if (is->p_frm_yuv == NULL) {
        printf("av_frame_alloc() failed\n");
        goto fail;
    }
    is->av_sync_type = av_sync_type;

    return is;
fail:
    player_deinit(is);
    return is;
}

int player_deinit(player_stat_t *is)
{
    pthread_mutex_lock(&_player_mutex);
    if (is == NULL) {
        pthread_mutex_unlock(&_player_mutex);
        return 0;
    }

    /* XXX: use a special url_shutdown call to abort parse cleanly */
    is->abort_request = 1;

    /* exit demux thread */
    if (is->read_tid)
        pthread_join(is->read_tid, NULL);

    /* close each stream */
    if (is->audio_idx >= 0) {
        stream_component_close(is, is->audio_idx);
        printf("audio stream_component_close finish\n");
    }
    if (is->video_idx >= 0) {
        stream_component_close(is, is->video_idx);
        printf("video stream_component_close finish\n");
    }

    if (is->p_frm_yuv)
        av_frame_free(&is->p_frm_yuv);

    if (is->p_frm_buf_ref)
        av_buffer_unref(&is->p_frm_buf_ref);

    /* free demux resource */
    if (is->p_fmt_ctx) {
        avformat_close_input(&is->p_fmt_ctx);
        avformat_free_context(is->p_fmt_ctx);
    }

    packet_queue_destroy(&is->video_pkt_queue);
    packet_queue_destroy(&is->audio_pkt_queue);

    /* free all pictures */
    frame_queue_destory(&is->video_frm_queue);
    frame_queue_destory(&is->audio_frm_queue);

    pthread_cond_destroy(&is->continue_read_thread);

    #if (0)
    if (is->img_convert_ctx)
        sws_freeContext(is->img_convert_ctx);
    #endif

    if (is->filename)
        av_free(is->filename);

    if (is->p_flush_pkt)
        av_packet_free(&is->p_flush_pkt);

    free(is);
    pthread_mutex_unlock(&_player_mutex);

    return 0;
}

/* pause or resume the video */
void stream_toggle_pause(player_stat_t *is)
{
    if (is->paused) {
        // 这里表示当前是暂停状态，将切换到继续播放状态。在继续播放之前，先将暂停期间流逝的时间加到frame_timer中
        is->frame_timer += av_gettime_relative() / 1000000.0 - is->video_clk.last_updated;
        if (is->read_pause_return != AVERROR(ENOSYS)) {
            is->video_clk.paused = 0;
        }
        set_clock(&is->video_clk, get_clock(&is->video_clk), is->video_clk.serial);
    }
    is->paused = is->audio_clk.paused = is->video_clk.paused = !is->paused;
}

void toggle_pause(player_stat_t *is)
{
    stream_toggle_pause(is);
    is->step = 0;
}

void stream_seek(player_stat_t *is, int64_t pos, int64_t rel, int seek_by_bytes)
{
    if (!is->seek_req) {
        is->seek_pos = pos;
        is->seek_rel = rel;
        is->seek_flags &= ~AVSEEK_FLAG_BYTE;
        if (seek_by_bytes) {
            is->seek_pos = pos * avio_size(is->p_fmt_ctx->pb) / is->p_fmt_ctx->duration;
            is->seek_flags |= AVSEEK_FLAG_BYTE;
        }
        is->seek_req = 1;
        pthread_cond_signal(&is->continue_read_thread);
    }
}
#endif