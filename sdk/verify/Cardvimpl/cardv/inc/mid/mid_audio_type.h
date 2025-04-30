/*
 * mid_audio_type.h- Sigmastar
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 *
 * Author: jeff.li <jeff.li@sigmastar.com.cn>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __MI_AUDIO_TYPE_H__
#define __MI_AUDIO_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mid_common.h"
#include "mi_ai_datatype.h"
#include "mi_ao_datatype.h"
#include "mi_aio_datatype.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define MI_AUDIO_SAMPLE_PER_FRAME 1024

typedef void (*AudioRecUpdateSizeCB)(char *);

typedef enum
{
    AUDIO_IN_PCM = 0,
    AUDIO_IN_AAC,
    AUDIO_IN_MP3,
} AudioInEncType_e;

typedef enum
{
    AUDIO_OUT_PLAYING_WAV = 0,
    AUDIO_OUT_PLAYING_LIVE,
    AUDIO_OUT_IDLE,
} AudioOutStatus_e;

typedef struct _CarDVAudioInParam
{
    MI_U32           u32DevId;
    MI_U8            u8ChnId;
    MI_AI_If_e       enAiIf;
    MI_S16           s8VolumeInDb;
    MI_U32           u32UserFrameDepth;
    MI_U32           u32BufQueueDepth;
    MI_BOOL          bAudioInMute;
    MI_AI_Attr_t     stAudioAttr;
    MI_U32           u32CompBufSize;
    AudioInEncType_e eAudioInEncType;
} CarDVAudioInParam;

typedef struct _CarDVAudioOutParam
{
    MI_U32               u32DevId;
    MI_AO_If_e           enAoIfs;
    MI_S16               s8VolumeOutDb;
    MI_AO_Attr_t         stAoDevAttr;
    MI_AO_GainFading_e   eAudioOutGainFading;
    MI_AUDIO_I2sConfig_t stI2sConfig;
} CarDVAudioOutParam;

// WAVE file header format
typedef struct _WavHeader_s
{
    MI_U8  riff[4];              // RIFF string
    MI_U32 ChunkSize;            // overall size of file in bytes
    MI_U8  wave[4];              // WAVE string
    MI_U8  fmt_chunk_marker[4];  // fmt string with trailing null char
    MI_U32 length_of_fmt;        // length of the format data
    MI_U16 format_type;          // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    MI_U16 channels;             // no.of channels
    MI_U32 sample_rate;          // sampling rate (blocks per second)
    MI_U32 byterate;             // SampleRate * NumChannels * BitsPerSample/8
    MI_U16 block_align;          // NumChannels * BitsPerSample/8
    MI_U16 bits_per_sample;      // bits per sample, 8- 8bits, 16- 16 bits etc
    MI_U8  data_chunk_header[4]; // DATA string or FLLR string
    MI_U32 data_size; // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} WavHeader_t;

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__MI_AUDIO_TYPE_H__