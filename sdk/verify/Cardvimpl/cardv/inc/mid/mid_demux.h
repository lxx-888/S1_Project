/*
 * mid_demux.h- Sigmastar
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
#ifndef _MID_DEMUX_H_
#define _MID_DEMUX_H_

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int     DemuxThumbnail(char *src_filename);
int64_t DemuxDuration(char *src_filename);

#endif //_MID_DEMUX_H_
