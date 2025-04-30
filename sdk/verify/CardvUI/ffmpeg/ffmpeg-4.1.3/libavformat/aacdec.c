/*
 * raw ADTS AAC demuxer
 * Copyright (c) 2008 Michael Niedermayer <michaelni@gmx.at>
 * Copyright (c) 2009 Robert Swain ( rob opendot cl )
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "avio_internal.h"
#include "internal.h"
#include "id3v1.h"
#include "id3v2.h"
#include "apetag.h"

#define ADTS_HEADER_SIZE 7
#define CALC_FRAME_NUM 15
#define AAC_SAMPLES_PER_FRAME 1024

typedef struct AACDecContext {
    int average_frame_size;
    AVStream *st;
} AACDecContext;

static uint32_t adts_get_sample_rate(const uint8_t sampling_freq_idx)
{
    static const uint32_t sample_rates[] =
    {
        96000, 88200, 64000, 48000, 44100, 32000,
        24000, 22050, 16000, 12000, 11025, 8000
    };

    if (sampling_freq_idx < sizeof(sample_rates) / sizeof(sample_rates[0])) {
        return sample_rates[sampling_freq_idx];
    }

    return 0;
}

static int adts_read_fixed_header(AVFormatContext *s, AVStream *st)
{
    uint8_t profile;
    uint8_t sampling_freq_idx;
    uint32_t sample_rate;
    uint8_t header[2];
    uint8_t channel;

    avio_seek(s->pb, 2, SEEK_SET);
    if (avio_read(s->pb, header, 2) < 2) {
        av_log(NULL, AV_LOG_ERROR, "adts header error\n");
        return -1;
    }

    profile = (header[0] >> 6) & 0x3;
    st->codecpar->profile = profile;

    sampling_freq_idx = (header[0] >> 2) & 0xf;
    sample_rate = adts_get_sample_rate(sampling_freq_idx);
    if (sample_rate == 0) {
        av_log(NULL, AV_LOG_ERROR, "adts sample rate error\n");
        return -1;
    }
    st->codecpar->sample_rate = sample_rate;

    channel = (header[0] & 0x1) << 2 | (header[1] >> 6);
    if (channel == 0) {
        av_log(NULL, AV_LOG_ERROR, "adts channels error\n");
        return -1;
    }
    st->codecpar->channels = channel;

    return 0;
}

static int adts_get_frame_size(AVFormatContext *s, int64_t offset)
{
    int frame_size = 0;
    uint8_t header[3];

    avio_seek(s->pb, offset + 3, SEEK_SET);
    if (avio_read(s->pb, header, 3) < 3) {
        return 0;
    }

    frame_size = (header[0] & 0x3) << 11 | header[1] << 3 | header[2] >> 5;
    return frame_size;
}

static int adts_aac_probe(AVProbeData *p)
{
    int max_frames = 0, first_frames = 0;
    int fsize, frames;
    const uint8_t *buf0 = p->buf;
    const uint8_t *buf2;
    const uint8_t *buf;
    const uint8_t *end = buf0 + p->buf_size - 7;

    buf = buf0;

    for (; buf < end; buf = buf2 + 1) {
        buf2 = buf;

        for (frames = 0; buf2 < end; frames++) {
            uint32_t header = AV_RB16(buf2);
            if ((header & 0xFFF6) != 0xFFF0) {
                if (buf != buf0) {
                    // Found something that isn't an ADTS header, starting
                    // from a position other than the start of the buffer.
                    // Discard the count we've accumulated so far since it
                    // probably was a false positive.
                    frames = 0;
                }
                break;
            }
            fsize = (AV_RB32(buf2 + 3) >> 13) & 0x1FFF;
            if (fsize < 7)
                break;
            fsize = FFMIN(fsize, end - buf2);
            buf2 += fsize;
        }
        max_frames = FFMAX(max_frames, frames);
        if (buf == buf0)
            first_frames = frames;
    }

    if (first_frames >= 3)
        return AVPROBE_SCORE_EXTENSION + 1;
    else if (max_frames > 100)
        return AVPROBE_SCORE_EXTENSION;
    else if (max_frames >= 3)
        return AVPROBE_SCORE_EXTENSION / 2;
    else if (first_frames >= 1)
        return 1;
    else
        return 0;
}

static int adts_aac_read_header(AVFormatContext *s)
{
    AVStream *st;
    uint16_t state;
    uint64_t cur_offset, frame_offset = 0, file_size = avio_size(s->pb);
    uint64_t estimate_duration;
    int estimate_frame_num, calc_frame_num = 0;
    int frame_size, calc_frame_size_sum = 0;
    AACDecContext *aac_ctx = s->priv_data;

    st = avformat_new_stream(s, NULL);
    if (!st)
        return AVERROR(ENOMEM);

    aac_ctx->st = st;

    st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codecpar->codec_id   = s->iformat->raw_codec_id;
    st->need_parsing         = AVSTREAM_PARSE_FULL_RAW;

    ff_id3v1_read(s);
    if ((s->pb->seekable & AVIO_SEEKABLE_NORMAL) &&
        !av_dict_get(s->metadata, "", NULL, AV_DICT_IGNORE_SUFFIX)) {
        int64_t cur = avio_tell(s->pb);
        ff_ape_parse_tag(s);
        avio_seek(s->pb, cur, SEEK_SET);
    }

    // skip data until the first ADTS frame is found
    state = avio_r8(s->pb);
    while (!avio_feof(s->pb) && avio_tell(s->pb) < s->probesize) {
        state = (state << 8) | avio_r8(s->pb);
        if ((state >> 4) != 0xFFF)
            continue;
        avio_seek(s->pb, -2, SEEK_CUR);
        break;
    }
    if ((state >> 4) != 0xFFF)
        return AVERROR_INVALIDDATA;

    cur_offset = avio_tell(s->pb);

    if (adts_read_fixed_header(s, st) < 0)
        return AVERROR_INVALIDDATA;

    avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);

    while (frame_offset < file_size && calc_frame_num < CALC_FRAME_NUM) {
        frame_size = adts_get_frame_size(s, frame_offset);
        if (frame_size == 0)
            return AVERROR_INVALIDDATA;

        frame_offset += frame_size;
        calc_frame_size_sum += frame_size;
        calc_frame_num++;
    }

    aac_ctx->average_frame_size = calc_frame_size_sum / calc_frame_num;
    if (calc_frame_num < CALC_FRAME_NUM)
        estimate_frame_num = calc_frame_num;
    else
        estimate_frame_num = file_size / aac_ctx->average_frame_size;

    estimate_duration = estimate_frame_num * (AAC_SAMPLES_PER_FRAME * 1000000LL / st->codecpar->sample_rate);
    estimate_duration = av_rescale_q(estimate_duration, AV_TIME_BASE_Q, st->time_base);
    st->duration = estimate_duration;

    avio_seek(s->pb, cur_offset, SEEK_SET);
    return 0;
}

static int handle_id3(AVFormatContext *s, AVPacket *pkt)
{
    AVDictionary *metadata = NULL;
    AVIOContext ioctx;
    ID3v2ExtraMeta *id3v2_extra_meta = NULL;
    int ret;

    ret = av_append_packet(s->pb, pkt, ff_id3v2_tag_len(pkt->data) - pkt->size);
    if (ret < 0) {
        av_packet_unref(pkt);
        return ret;
    }

    ffio_init_context(&ioctx, pkt->data, pkt->size, 0, NULL, NULL, NULL, NULL);
    ff_id3v2_read_dict(&ioctx, &metadata, ID3v2_DEFAULT_MAGIC, &id3v2_extra_meta);
    if ((ret = ff_id3v2_parse_priv_dict(&metadata, &id3v2_extra_meta)) < 0)
        goto error;

    if (metadata) {
        if ((ret = av_dict_copy(&s->metadata, metadata, 0)) < 0)
            goto error;
        s->event_flags |= AVFMT_EVENT_FLAG_METADATA_UPDATED;
    }

error:
    av_packet_unref(pkt);
    ff_id3v2_free_extra_meta(&id3v2_extra_meta);
    av_dict_free(&metadata);

    return ret;
}

static int adts_aac_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    int ret, fsize;
    int estimate_frame_num = 0;
    AACDecContext *aac_ctx = s->priv_data;
    AVStream *st = aac_ctx->st;

    // Parse all the ID3 headers between frames
    while (1) {
        ret = av_get_packet(s->pb, pkt, FFMAX(ID3v2_HEADER_SIZE, ADTS_HEADER_SIZE));
        if (ret >= ID3v2_HEADER_SIZE && ff_id3v2_match(pkt->data, ID3v2_DEFAULT_MAGIC)) {
            if ((ret = handle_id3(s, pkt)) >= 0) {
                continue;
            }
        }
        break;
    }

    if (ret < 0)
        return ret;

    if (ret < ADTS_HEADER_SIZE) {
        av_packet_unref(pkt);
        return AVERROR(EIO);
    }

    estimate_frame_num =
        (avio_tell(s->pb) + aac_ctx->average_frame_size / 2) / aac_ctx->average_frame_size;
    pkt->pts = estimate_frame_num * (AAC_SAMPLES_PER_FRAME * 1000000LL / st->codecpar->sample_rate);
    pkt->pts = av_rescale_q(pkt->pts, AV_TIME_BASE_Q, st->time_base);

    if ((AV_RB16(pkt->data) >> 4) != 0xfff) {
        av_packet_unref(pkt);
        return AVERROR_INVALIDDATA;
    }

    fsize = (AV_RB32(pkt->data + 3) >> 13) & 0x1FFF;
    if (fsize < ADTS_HEADER_SIZE) {
        av_packet_unref(pkt);
        return AVERROR_INVALIDDATA;
    }

    ret = av_append_packet(s->pb, pkt, fsize - pkt->size);
    if (ret < 0)
        av_packet_unref(pkt);

    return ret;
}

AVInputFormat ff_aac_demuxer = {
    .name         = "aac",
    .long_name    = NULL_IF_CONFIG_SMALL("raw ADTS AAC (Advanced Audio Coding)"),
    .read_probe   = adts_aac_probe,
    .read_header  = adts_aac_read_header,
    .read_packet  = adts_aac_read_packet,
    .flags        = AVFMT_GENERIC_INDEX,
    .extensions   = "aac",
    .mime_type    = "audio/aac,audio/aacp,audio/x-aac",
    .raw_codec_id = AV_CODEC_ID_AAC,
    .priv_data_size = sizeof(AACDecContext),
};
