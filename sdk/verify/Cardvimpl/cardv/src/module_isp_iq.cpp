/*
 * module_isp.cpp- Sigmastar
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

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "module_common.h"

#if CARDV_ISP_ENABLE

#include "mi_common.h"
#include "mi_isp_iq.h"
#include "mi_isp_cus3a_api.h"
#include "mi_isp_datatype.h"
#include "mi_isp_awb.h"
#include "mi_isp_awb_datatype.h"
#include "mi_isp_af.h"
#include "mi_isp_af_datatype.h"
#include "mi_isp_ae.h"
#include "mi_isp_ae_datatype.h"
#include "mi_vif_datatype.h"
#include "mi_vif.h"
#include "mi_sensor.h"
#include "isp_cus3a_if.h"

int    isp_dev            = 0;
int    isp_chn            = 0; //-1;
MI_U16 RGBGamma_L[3][256] = {
    {0,    20,   40,   61,   82,   102,  122,  140,  158, 175, 191,  206,  220,  234,  247,  261,  273,  286,  298,
     309,  321,  331,  342,  352,  362,  371,  380,  389, 397, 405,  413,  421,  429,  436,  444,  451,  459,  466,
     473,  479,  486,  493,  499,  505,  511,  517,  523, 529, 535,  540,  545,  551,  556,  561,  566,  571,  576,
     580,  585,  590,  595,  599,  604,  608,  613,  617, 621, 625,  629,  633,  636,  640,  644,  647,  651,  654,
     657,  661,  664,  667,  670,  674,  677,  680,  683, 686, 689,  692,  695,  698,  701,  704,  708,  711,  714,
     717,  720,  723,  726,  729,  732,  735,  738,  741, 744, 746,  749,  752,  755,  758,  761,  763,  766,  769,
     772,  775,  777,  780,  783,  786,  789,  791,  794, 797, 799,  802,  804,  807,  809,  812,  814,  816,  818,
     821,  823,  825,  827,  829,  831,  833,  835,  837, 839, 841,  843,  845,  847,  849,  851,  853,  855,  857,
     859,  861,  863,  865,  867,  869,  870,  872,  874, 876, 878,  879,  881,  883,  885,  886,  888,  890,  891,
     893,  894,  896,  898,  899,  901,  903,  905,  906, 908, 910,  911,  913,  915,  916,  918,  920,  922,  923,
     925,  927,  929,  930,  932,  934,  936,  938,  939, 941, 943,  945,  947,  949,  950,  952,  954,  955,  957,
     959,  960,  962,  963,  965,  966,  968,  969,  971, 972, 974,  975,  977,  978,  979,  981,  982,  984,  985,
     986,  988,  989,  991,  992,  993,  995,  996,  997, 999, 1000, 1001, 1003, 1004, 1005, 1007, 1008, 1009, 1010,
     1012, 1013, 1014, 1016, 1017, 1018, 1019, 1021, 1023},
    {0,    20,   40,   61,   82,   102,  122,  140,  158, 175, 191,  206,  220,  234,  247,  261,  273,  286,  298,
     309,  321,  331,  342,  352,  362,  371,  380,  389, 397, 405,  413,  421,  429,  436,  444,  451,  459,  466,
     473,  479,  486,  493,  499,  505,  511,  517,  523, 529, 535,  540,  545,  551,  556,  561,  566,  571,  576,
     580,  585,  590,  595,  599,  604,  608,  613,  617, 621, 625,  629,  633,  636,  640,  644,  647,  651,  654,
     657,  661,  664,  667,  670,  674,  677,  680,  683, 686, 689,  692,  695,  698,  701,  704,  708,  711,  714,
     717,  720,  723,  726,  729,  732,  735,  738,  741, 744, 746,  749,  752,  755,  758,  761,  763,  766,  769,
     772,  775,  777,  780,  783,  786,  789,  791,  794, 797, 799,  802,  804,  807,  809,  812,  814,  816,  818,
     821,  823,  825,  827,  829,  831,  833,  835,  837, 839, 841,  843,  845,  847,  849,  851,  853,  855,  857,
     859,  861,  863,  865,  867,  869,  870,  872,  874, 876, 878,  879,  881,  883,  885,  886,  888,  890,  891,
     893,  894,  896,  898,  899,  901,  903,  905,  906, 908, 910,  911,  913,  915,  916,  918,  920,  922,  923,
     925,  927,  929,  930,  932,  934,  936,  938,  939, 941, 943,  945,  947,  949,  950,  952,  954,  955,  957,
     959,  960,  962,  963,  965,  966,  968,  969,  971, 972, 974,  975,  977,  978,  979,  981,  982,  984,  985,
     986,  988,  989,  991,  992,  993,  995,  996,  997, 999, 1000, 1001, 1003, 1004, 1005, 1007, 1008, 1009, 1010,
     1012, 1013, 1014, 1016, 1017, 1018, 1019, 1021, 1023},
    {0,    20,   40,   61,   82,   102,  122,  140,  158, 175, 191,  206,  220,  234,  247,  261,  273,  286,  298,
     309,  321,  331,  342,  352,  362,  371,  380,  389, 397, 405,  413,  421,  429,  436,  444,  451,  459,  466,
     473,  479,  486,  493,  499,  505,  511,  517,  523, 529, 535,  540,  545,  551,  556,  561,  566,  571,  576,
     580,  585,  590,  595,  599,  604,  608,  613,  617, 621, 625,  629,  633,  636,  640,  644,  647,  651,  654,
     657,  661,  664,  667,  670,  674,  677,  680,  683, 686, 689,  692,  695,  698,  701,  704,  708,  711,  714,
     717,  720,  723,  726,  729,  732,  735,  738,  741, 744, 746,  749,  752,  755,  758,  761,  763,  766,  769,
     772,  775,  777,  780,  783,  786,  789,  791,  794, 797, 799,  802,  804,  807,  809,  812,  814,  816,  818,
     821,  823,  825,  827,  829,  831,  833,  835,  837, 839, 841,  843,  845,  847,  849,  851,  853,  855,  857,
     859,  861,  863,  865,  867,  869,  870,  872,  874, 876, 878,  879,  881,  883,  885,  886,  888,  890,  891,
     893,  894,  896,  898,  899,  901,  903,  905,  906, 908, 910,  911,  913,  915,  916,  918,  920,  922,  923,
     925,  927,  929,  930,  932,  934,  936,  938,  939, 941, 943,  945,  947,  949,  950,  952,  954,  955,  957,
     959,  960,  962,  963,  965,  966,  968,  969,  971, 972, 974,  975,  977,  978,  979,  981,  982,  984,  985,
     986,  988,  989,  991,  992,  993,  995,  996,  997, 999, 1000, 1001, 1003, 1004, 1005, 1007, 1008, 1009, 1010,
     1012, 1013, 1014, 1016, 1017, 1018, 1019, 1021, 1023}};

MI_U16 RGBGamma_N[3][256] = {
    {0,    8,    14,   24,   31,   40,   51,   59,   71,   83,   94,   104,  117,  126,  139,  155, 169, 182, 195, 212,
     225,  238,  254,  266,  281,  290,  301,  314,  325,  336,  346,  356,  365,  374,  384,  393, 400, 410, 415, 424,
     434,  441,  449,  457,  465,  471,  479,  487,  494,  500,  508,  514,  520,  528,  533,  540, 546, 553, 559, 566,
     571,  578,  583,  588,  593,  598,  605,  609,  615,  619,  624,  629,  633,  638,  642,  647, 650, 654, 658, 661,
     665,  669,  672,  675,  679,  682,  686,  690,  693,  696,  700,  703,  706,  709,  712,  715, 718, 721, 724, 728,
     730,  733,  736,  740,  743,  745,  748,  751,  753,  756,  759,  762,  765,  768,  770,  772, 775, 777, 781, 784,
     786,  789,  792,  794,  797,  799,  802,  804,  807,  809,  811,  814,  816,  818,  821,  824, 826, 828, 831, 833,
     835,  837,  839,  842,  844,  846,  848,  850,  852,  854,  856,  858,  859,  861,  863,  865, 866, 869, 871, 873,
     875,  876,  878,  880,  882,  883,  885,  886,  888,  890,  892,  893,  895,  897,  898,  900, 901, 904, 905, 906,
     908,  910,  911,  914,  916,  917,  919,  920,  922,  924,  926,  927,  928,  930,  932,  933, 935, 936, 938, 939,
     941,  943,  944,  946,  947,  949,  950,  952,  954,  955,  958,  959,  961,  962,  964,  965, 967, 968, 969, 971,
     972,  974,  976,  977,  979,  980,  982,  983,  985,  986,  987,  989,  990,  992,  993,  995, 996, 997, 999, 1002,
     1003, 1004, 1006, 1007, 1008, 1010, 1011, 1012, 1014, 1015, 1017, 1018, 1019, 1021, 1022, 1023},
    {0,    8,    14,   24,   31,   40,   51,   59,   71,   83,   94,   104,  117,  126,  139,  155, 169, 182, 195, 212,
     225,  238,  254,  266,  281,  290,  301,  314,  325,  336,  346,  356,  365,  374,  384,  393, 400, 410, 415, 424,
     434,  441,  449,  457,  465,  471,  479,  487,  494,  500,  508,  514,  520,  528,  533,  540, 546, 553, 559, 566,
     571,  578,  583,  588,  593,  598,  605,  609,  615,  619,  624,  629,  633,  638,  642,  647, 650, 654, 658, 661,
     665,  669,  672,  675,  679,  682,  686,  690,  693,  696,  700,  703,  706,  709,  712,  715, 718, 721, 724, 728,
     730,  733,  736,  740,  743,  745,  748,  751,  753,  756,  759,  762,  765,  768,  770,  772, 775, 777, 781, 784,
     786,  789,  792,  794,  797,  799,  802,  804,  807,  809,  811,  814,  816,  818,  821,  824, 826, 828, 831, 833,
     835,  837,  839,  842,  844,  846,  848,  850,  852,  854,  856,  858,  859,  861,  863,  865, 866, 869, 871, 873,
     875,  876,  878,  880,  882,  883,  885,  886,  888,  890,  892,  893,  895,  897,  898,  900, 901, 904, 905, 906,
     908,  910,  911,  914,  916,  917,  919,  920,  922,  924,  926,  927,  928,  930,  932,  933, 935, 936, 938, 939,
     941,  943,  944,  946,  947,  949,  950,  952,  954,  955,  958,  959,  961,  962,  964,  965, 967, 968, 969, 971,
     972,  974,  976,  977,  979,  980,  982,  983,  985,  986,  987,  989,  990,  992,  993,  995, 996, 997, 999, 1002,
     1003, 1004, 1006, 1007, 1008, 1010, 1011, 1012, 1014, 1015, 1017, 1018, 1019, 1021, 1022, 1023},
    {0,    8,    14,   24,   31,   40,   51,   59,   71,   83,   94,   104,  117,  126,  139,  155, 169, 182, 195, 212,
     225,  238,  254,  266,  281,  290,  301,  314,  325,  336,  346,  356,  365,  374,  384,  393, 400, 410, 415, 424,
     434,  441,  449,  457,  465,  471,  479,  487,  494,  500,  508,  514,  520,  528,  533,  540, 546, 553, 559, 566,
     571,  578,  583,  588,  593,  598,  605,  609,  615,  619,  624,  629,  633,  638,  642,  647, 650, 654, 658, 661,
     665,  669,  672,  675,  679,  682,  686,  690,  693,  696,  700,  703,  706,  709,  712,  715, 718, 721, 724, 728,
     730,  733,  736,  740,  743,  745,  748,  751,  753,  756,  759,  762,  765,  768,  770,  772, 775, 777, 781, 784,
     786,  789,  792,  794,  797,  799,  802,  804,  807,  809,  811,  814,  816,  818,  821,  824, 826, 828, 831, 833,
     835,  837,  839,  842,  844,  846,  848,  850,  852,  854,  856,  858,  859,  861,  863,  865, 866, 869, 871, 873,
     875,  876,  878,  880,  882,  883,  885,  886,  888,  890,  892,  893,  895,  897,  898,  900, 901, 904, 905, 906,
     908,  910,  911,  914,  916,  917,  919,  920,  922,  924,  926,  927,  928,  930,  932,  933, 935, 936, 938, 939,
     941,  943,  944,  946,  947,  949,  950,  952,  954,  955,  958,  959,  961,  962,  964,  965, 967, 968, 969, 971,
     972,  974,  976,  977,  979,  980,  982,  983,  985,  986,  987,  989,  990,  992,  993,  995, 996, 997, 999, 1002,
     1003, 1004, 1006, 1007, 1008, 1010, 1011, 1012, 1014, 1015, 1017, 1018, 1019, 1021, 1022, 1023}};

MI_U16 RGBGamma_H[3][256] = {
    {0,    7,    14,   22,   31,   39,   49,   58,   68,   78,   88,   98,   109,  119,  130,  141,  152,  163,  174,
     185,  196,  207,  218,  228,  238,  248,  258,  268,  278,  287,  297,  306,  315,  324,  333,  342,  350,  359,
     367,  375,  383,  391,  399,  407,  414,  422,  429,  437,  444,  451,  458,  465,  472,  478,  485,  491,  498,
     504,  510,  516,  522,  527,  533,  539,  544,  550,  556,  561,  566,  572,  577,  582,  587,  592,  597,  602,
     607,  612,  617,  621,  626,  631,  635,  640,  643,  647,  652,  656,  660,  664,  668,  672,  676,  680,  684,
     688,  692,  696,  700,  704,  708,  712,  716,  720,  724,  728,  732,  735,  739,  743,  747,  751,  754,  759,
     764,  768,  772,  775,  779,  783,  787,  790,  794,  797,  800,  804,  807,  810,  813,  816,  819,  822,  825,
     828,  831,  834,  836,  839,  842,  844,  847,  848,  851,  853,  855,  857,  860,  862,  864,  867,  869,  872,
     874,  876,  879,  881,  884,  886,  889,  891,  894,  897,  899,  902,  904,  907,  909,  912,  914,  917,  919,
     921,  923,  926,  928,  930,  932,  934,  936,  938,  940,  942,  944,  945,  947,  949,  951,  953,  954,  956,
     958,  959,  961,  963,  964,  966,  968,  969,  971,  972,  974,  975,  977,  978,  979,  981,  982,  984,  985,
     986,  987,  989,  990,  991,  992,  993,  995,  996,  997,  998,  999,  1000, 1001, 1002, 1003, 1004, 1005, 1005,
     1006, 1007, 1008, 1009, 1009, 1010, 1011, 1012, 1012, 1013, 1013, 1014, 1015, 1015, 1016, 1016, 1016, 1017, 1017,
     1018, 1018, 1018, 1019, 1019, 1019, 1019, 1019, 1019},
    {0,    7,    14,   22,   31,   39,   49,   58,   68,   78,   88,   98,   109,  119,  130,  141,  152,  163,  174,
     185,  196,  207,  218,  228,  238,  248,  258,  268,  278,  287,  297,  306,  315,  324,  333,  342,  350,  359,
     367,  375,  383,  391,  399,  407,  414,  422,  429,  437,  444,  451,  458,  465,  472,  478,  485,  491,  498,
     504,  510,  516,  522,  527,  533,  539,  544,  550,  556,  561,  566,  572,  577,  582,  587,  592,  597,  602,
     607,  612,  617,  621,  626,  631,  635,  640,  643,  647,  652,  656,  660,  664,  668,  672,  676,  680,  684,
     688,  692,  696,  700,  704,  708,  712,  716,  720,  724,  728,  732,  735,  739,  743,  747,  751,  754,  759,
     764,  768,  772,  775,  779,  783,  787,  790,  794,  797,  800,  804,  807,  810,  813,  816,  819,  822,  825,
     828,  831,  834,  836,  839,  842,  844,  847,  848,  851,  853,  855,  857,  860,  862,  864,  867,  869,  872,
     874,  876,  879,  881,  884,  886,  889,  891,  894,  897,  899,  902,  904,  907,  909,  912,  914,  917,  919,
     921,  923,  926,  928,  930,  932,  934,  936,  938,  940,  942,  944,  945,  947,  949,  951,  953,  954,  956,
     958,  959,  961,  963,  964,  966,  968,  969,  971,  972,  974,  975,  977,  978,  979,  981,  982,  984,  985,
     986,  987,  989,  990,  991,  992,  993,  995,  996,  997,  998,  999,  1000, 1001, 1002, 1003, 1004, 1005, 1005,
     1006, 1007, 1008, 1009, 1009, 1010, 1011, 1012, 1012, 1013, 1013, 1014, 1015, 1015, 1016, 1016, 1016, 1017, 1017,
     1018, 1018, 1018, 1019, 1019, 1019, 1019, 1019, 1019},
    {0,    7,    14,   22,   31,   39,   49,   58,   68,   78,   88,   98,   109,  119,  130,  141,  152,  163,  174,
     185,  196,  207,  218,  228,  238,  248,  258,  268,  278,  287,  297,  306,  315,  324,  333,  342,  350,  359,
     367,  375,  383,  391,  399,  407,  414,  422,  429,  437,  444,  451,  458,  465,  472,  478,  485,  491,  498,
     504,  510,  516,  522,  527,  533,  539,  544,  550,  556,  561,  566,  572,  577,  582,  587,  592,  597,  602,
     607,  612,  617,  621,  626,  631,  635,  640,  643,  647,  652,  656,  660,  664,  668,  672,  676,  680,  684,
     688,  692,  696,  700,  704,  708,  712,  716,  720,  724,  728,  732,  735,  739,  743,  747,  751,  754,  759,
     764,  768,  772,  775,  779,  783,  787,  790,  794,  797,  800,  804,  807,  810,  813,  816,  819,  822,  825,
     828,  831,  834,  836,  839,  842,  844,  847,  848,  851,  853,  855,  857,  860,  862,  864,  867,  869,  872,
     874,  876,  879,  881,  884,  886,  889,  891,  894,  897,  899,  902,  904,  907,  909,  912,  914,  917,  919,
     921,  923,  926,  928,  930,  932,  934,  936,  938,  940,  942,  944,  945,  947,  949,  951,  953,  954,  956,
     958,  959,  961,  963,  964,  966,  968,  969,  971,  972,  974,  975,  977,  978,  979,  981,  982,  984,  985,
     986,  987,  989,  990,  991,  992,  993,  995,  996,  997,  998,  999,  1000, 1001, 1002, 1003, 1004, 1005, 1005,
     1006, 1007, 1008, 1009, 1009, 1010, 1011, 1012, 1012, 1013, 1013, 1014, 1015, 1015, 1016, 1016, 1016, 1017, 1017,
     1018, 1018, 1018, 1019, 1019, 1019, 1019, 1019, 1019}};

int isp_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    // if (isp_chn == -1)
    // {
    //     for (int i = 0; i < VPE_MAX_CHANNEL_NUM; i++)
    //     {
    //         if (g_vpeParam[i].eVpeRunningMode != E_MI_VPE_RUN_INVALID &&
    //             g_vpeParam[i].eVpeRunningMode != E_MI_VPE_RUN_DVR_MODE)
    //         {
    //             isp_chn = i;
    //             break;
    //         }
    //     }
    // }

    // if (isp_chn == -1)
    //     return 0;

    switch (id)
    {
    case CMD_ISP_OPEN:
        break;
    case CMD_ISP_SET_CONTRAST: {
        int                      nValue;
        MI_ISP_IQ_ContrastType_t Contrast;

        MI_ISP_IQ_GetContrast(isp_dev, isp_chn, &Contrast);
        printf("IQ_SetContrast:[0,100] Cur=%d\n", Contrast.stManual.stParaAPI.u32Lev);
        nValue            = *param;
        Contrast.bEnable  = E_SS_IQ_TRUE;
        Contrast.enOpType = E_SS_IQ_OP_TYP_MANUAL;
        {
            Contrast.stManual.stParaAPI.u32Lev = (U32)nValue;
        }
        MI_ISP_IQ_SetContrast(isp_dev, isp_chn, &Contrast);
        printf("Contrast:%d\n", nValue);
        break;
    }
    case CMD_ISP_SET_BRIGHTNESS: {
        int                        nValue;
        MI_ISP_IQ_BrightnessType_t Brightness;

        MI_ISP_IQ_GetBrightness(isp_dev, isp_chn, &Brightness);
        printf("IQ_SetBrightness:[0,100] Cur=%d\n", Brightness.stManual.stParaAPI.u32Lev);
        nValue                               = *param;
        Brightness.bEnable                   = E_SS_IQ_TRUE;
        Brightness.enOpType                  = E_SS_IQ_OP_TYP_MANUAL;
        Brightness.stManual.stParaAPI.u32Lev = (U32)nValue;
        MI_ISP_IQ_SetBrightness(isp_dev, isp_chn, &Brightness);
        printf("Brightness:%d\n", nValue);
        break;
    }
    case CMD_ISP_SET_SATURATION: {
        int                        nValue;
        MI_ISP_IQ_SaturationType_t Saturation;

        MI_ISP_IQ_GetSaturation(isp_dev, isp_chn, &Saturation);
        printf("IQ_SetSaturation u8SatAllStr [0,127] Cur=%d\n", Saturation.stManual.stParaAPI.u8SatAllStr);
        nValue                                    = *param;
        Saturation.stManual.stParaAPI.u8SatAllStr = (U8)nValue; // 0~127
        Saturation.bEnable                        = E_SS_IQ_TRUE;
        Saturation.enOpType                       = E_SS_IQ_OP_TYP_MANUAL;
        MI_ISP_IQ_SetSaturation(isp_dev, isp_chn, &Saturation);
        printf("Saturation:%u\n", Saturation.stManual.stParaAPI.u8SatAllStr);
        break;
    }
    case CMD_ISP_SET_SHARPENESS: {
        int                       nValue;
        MI_ISP_IQ_SharpnessType_t Sharpnessv1;
        MI_ISP_IQ_GetSharpness(isp_dev, isp_chn, &Sharpnessv1);
        Sharpnessv1.bEnable = E_SS_IQ_TRUE;
#if (defined CHIP_I6)
        Sharpnessv1.stManual.stParaAPI.bDirEn = E_SS_IQ_TRUE;
#endif
        Sharpnessv1.enOpType = E_SS_IQ_OP_TYP_MANUAL;

        nValue = *(int *)param;
        printf("nValue = %d\n", nValue);
        if (nValue > 1023)
        {
            printf("out of rang, [0, 1023], %d\n", nValue);
            nValue = 1023;
        }

#if (defined CHIP_I6)
        Sharpnessv1.stManual.stParaAPI.u16SharpnessUD = nValue;
        Sharpnessv1.stManual.stParaAPI.u16SharpnessD  = nValue;
#elif (defined CHIP_M6)
        Sharpnessv1.stManual.stParaAPI.u8SharpnessUD[0] = nValue;
        Sharpnessv1.stManual.stParaAPI.u8SharpnessUD[1] = nValue;
        Sharpnessv1.stManual.stParaAPI.u8SharpnessUD[2] = nValue;
        Sharpnessv1.stManual.stParaAPI.u8SharpnessD[0]  = nValue;
        Sharpnessv1.stManual.stParaAPI.u8SharpnessD[1]  = nValue;
        Sharpnessv1.stManual.stParaAPI.u8SharpnessD[2]  = nValue;
#elif (defined CHIP_I6C) || (defined CHIP_IFADO) || (defined CHIP_IFORD)
        Sharpnessv1.stManual.stParaAPI.u8SharpnessUD[0] = nValue;
        Sharpnessv1.stManual.stParaAPI.u8SharpnessUD[1] = nValue;
        Sharpnessv1.stManual.stParaAPI.u8SharpnessUD[2] = nValue;
        Sharpnessv1.stManual.stParaAPI.u8SharpnessD[0]  = nValue;
        Sharpnessv1.stManual.stParaAPI.u8SharpnessD[1]  = nValue;
        Sharpnessv1.stManual.stParaAPI.u8SharpnessD[2]  = nValue;
#else
        Sharpnessv1.stManual.stParaAPI.u16SharpnessUD[0] = nValue;
        Sharpnessv1.stManual.stParaAPI.u16SharpnessUD[1] = nValue;
        Sharpnessv1.stManual.stParaAPI.u16SharpnessD[0]  = nValue;
        Sharpnessv1.stManual.stParaAPI.u16SharpnessD[1]  = nValue;

#endif
        MI_ISP_IQ_SetSharpness(isp_dev, isp_chn, &Sharpnessv1);
        break;
    }
    case CMD_ISP_SET_LIGHTSENSITIVITY: {
        int                       nValue = (U32)*param;
        MI_ISP_IQ_LightnessType_t Lightness;

        MI_ISP_IQ_GetLightness(isp_dev, isp_chn, &Lightness);
        printf("IQ_SetLightness [0,100] Cur=%d\n", Lightness.stManual.stParaAPI.u32Lev);
        if (nValue < 0 || nValue > 100)
        {
            nValue = 0;
        }
        Lightness.bEnable                   = E_SS_IQ_TRUE;
        Lightness.enOpType                  = E_SS_IQ_OP_TYP_MANUAL;
        Lightness.stManual.stParaAPI.u32Lev = (U32)nValue;
        MI_ISP_IQ_SetLightness(isp_dev, isp_chn, &Lightness);

        printf("Lightness:%d\n", nValue);
        break;
    }
    case CMD_ISP_SET_SCENE: {
        break;
    }
    case CMD_ISP_SET_WB_MODE: {
        int                   nValue;
        MI_ISP_AWB_AttrType_t awbAttr;
        nValue = *param;
        MI_ISP_AWB_GetAttr(isp_dev, isp_chn, &awbAttr);
        printf("WB MODE: %d\n", nValue);
        switch (nValue)
        {
        case 0: // AUTO
            awbAttr.eOpType = E_SS_AWB_MODE_AUTO;
            break;
        case 1: // DAYLIGHT
            awbAttr.eOpType                   = E_SS_AWB_MODE_MANUAL;
            awbAttr.stManualParaAPI.u16Bgain  = 1390;
            awbAttr.stManualParaAPI.u16Gbgain = 1024;
            awbAttr.stManualParaAPI.u16Grgain = 1024;
            awbAttr.stManualParaAPI.u16Rgain  = 1562;
            break;
        case 2: // CLOUDY
            awbAttr.eOpType                   = E_SS_AWB_MODE_MANUAL;
            awbAttr.stManualParaAPI.u16Bgain  = 1172;
            awbAttr.stManualParaAPI.u16Gbgain = 1024;
            awbAttr.stManualParaAPI.u16Grgain = 1024;
            awbAttr.stManualParaAPI.u16Rgain  = 1730;
            break;
        case 3: // FLUORESCENT_WHITE
            awbAttr.eOpType                   = E_SS_AWB_MODE_MANUAL;
            awbAttr.stManualParaAPI.u16Bgain  = 2530;
            awbAttr.stManualParaAPI.u16Gbgain = 1024;
            awbAttr.stManualParaAPI.u16Grgain = 1024;
            awbAttr.stManualParaAPI.u16Rgain  = 1500;
            break;
        case 4: // INCANDESCENT
            awbAttr.eOpType                   = E_SS_AWB_MODE_MANUAL;
            awbAttr.stManualParaAPI.u16Bgain  = 3086;
            awbAttr.stManualParaAPI.u16Gbgain = 1046;
            awbAttr.stManualParaAPI.u16Grgain = 1046;
            awbAttr.stManualParaAPI.u16Rgain  = 1024;
            break;
        default:
            awbAttr.eOpType = E_SS_AWB_MODE_AUTO;
            break;
        }
        MI_ISP_AWB_SetAttr(isp_dev, isp_chn, &awbAttr);
        break;
    }
    case CMD_ISP_SET_EXPOSURE_MODE: {
        MI_ISP_AE_EvCompType_t AeAttr;
        MI_ISP_AE_GetEvComp(isp_dev, isp_chn, &AeAttr);
        AeAttr.s32EV = (S32)*param;
        printf("EXPOSURE: %+d\n", AeAttr.s32EV);
        MI_ISP_AE_SetEvComp(isp_dev, isp_chn, &AeAttr);
        break;
    }
    case CMD_ISP_SET_EXPOSURE_LIMIT: {
        MI_ISP_AE_ExpoLimitType_t stAeExpoLimit = {0};
        MI_ISP_AE_GetExposureLimit(isp_dev, isp_chn, &stAeExpoLimit);
        // printf("[%d,%d][%d,%d][%d,%d][%d,%d]\n",
        //        stAeExpoLimit.u32MinShutterUS, stAeExpoLimit.u32MaxShutterUS,
        //        stAeExpoLimit.u32MinFNx10, stAeExpoLimit.u32MaxFNx10,
        //        stAeExpoLimit.u32MinSensorGain, stAeExpoLimit.u32MinISPGain,
        //        stAeExpoLimit.u32MaxSensorGain, stAeExpoLimit.u32MaxISPGain);

        if (*param)
        {
            stAeExpoLimit.u32MinShutterUS = 250000; // 4fps
            stAeExpoLimit.u32MaxShutterUS = 500000; // 2fps
            MI_ISP_AE_SetExposureLimit(isp_dev, isp_chn, &stAeExpoLimit);
        }
        else
        {
            stAeExpoLimit.u32MaxShutterUS = 33332; // 30fps
            MI_ISP_AE_SetExposureLimit(isp_dev, isp_chn, &stAeExpoLimit);
        }
        break;
    }
    case CMD_ISP_SET_ISO: {
        int                       nValue;
        MI_ISP_AE_ModeType_e      mode = E_SS_AE_MODE_A;
        MI_ISP_AE_ExpoValueType_t stExpoVal;

        nValue = *param;
        switch (nValue)
        {
        case 0: // AE_ISO_AUTO
            mode = E_SS_AE_MODE_A;
            MI_ISP_AE_SetExpoMode(isp_dev, isp_chn, &mode);
            break;
        case 1: // AE_ISO_100
        case 2: // AE_ISO_200
        case 3: // AE_ISO_400
        case 4: // AE_ISO_800
        case 5: // AE_ISO_1600
        case 6: // AE_ISO_3200
            mode = E_SS_AE_MODE_M;
            MI_ISP_AE_SetExpoMode(isp_dev, isp_chn, &mode);
            MI_ISP_AE_GetManualExpo(isp_dev, isp_chn, &stExpoVal);
            stExpoVal.u32ISPGain    = 1024;
            stExpoVal.u32SensorGain = 1024 * (1 << (nValue - 1));
            MI_ISP_AE_SetManualExpo(isp_dev, isp_chn, &stExpoVal);
            break;
        default:
            mode = E_SS_AE_MODE_A;
            MI_ISP_AE_SetExpoMode(isp_dev, isp_chn, &mode);
            break;
        }
        break;
    }

    case CMD_ISP_SET_SHUTTER: {
        int                       nValue;
        MI_ISP_AE_ModeType_e      mode = E_SS_AE_MODE_A;
        MI_ISP_AE_ExpoValueType_t stExpoVal;

        nValue = *param;
        switch (nValue)
        {
        case 0: // AE_SHUTTER_AUTO
            mode = E_SS_AE_MODE_A;
            MI_ISP_AE_SetExpoMode(isp_dev, isp_chn, &mode);
            break;
        case 1: // AE_SHUTTER_1/60
            mode = E_SS_AE_MODE_M;
            MI_ISP_AE_SetExpoMode(isp_dev, isp_chn, &mode);
            MI_ISP_AE_GetManualExpo(isp_dev, isp_chn, &stExpoVal);
            stExpoVal.u32US = (1 * 1000000) / 60;
            MI_ISP_AE_SetManualExpo(isp_dev, isp_chn, &stExpoVal);
            break;
        case 2: // AE_SHUTTER_1/120
            mode = E_SS_AE_MODE_M;
            MI_ISP_AE_SetExpoMode(isp_dev, isp_chn, &mode);
            MI_ISP_AE_GetManualExpo(isp_dev, isp_chn, &stExpoVal);
            stExpoVal.u32US = (1 * 1000000) / 120;
            MI_ISP_AE_SetManualExpo(isp_dev, isp_chn, &stExpoVal);
            break;
        case 3: // AE_SHUTTER_1/240
            mode = E_SS_AE_MODE_M;
            MI_ISP_AE_SetExpoMode(isp_dev, isp_chn, &mode);
            MI_ISP_AE_GetManualExpo(isp_dev, isp_chn, &stExpoVal);
            stExpoVal.u32US = (1 * 1000000) / 240;
            MI_ISP_AE_SetManualExpo(isp_dev, isp_chn, &stExpoVal);
            break;
        case 4: // AE_SHUTTER_1/500
            mode = E_SS_AE_MODE_M;
            MI_ISP_AE_SetExpoMode(isp_dev, isp_chn, &mode);
            MI_ISP_AE_GetManualExpo(isp_dev, isp_chn, &stExpoVal);
            stExpoVal.u32US = (1 * 1000000) / 500;
            MI_ISP_AE_SetManualExpo(isp_dev, isp_chn, &stExpoVal);
            break;
        case 5: // AE_SHUTTER_1/1000
            mode = E_SS_AE_MODE_M;
            MI_ISP_AE_SetExpoMode(isp_dev, isp_chn, &mode);
            MI_ISP_AE_GetManualExpo(isp_dev, isp_chn, &stExpoVal);
            stExpoVal.u32US = (1 * 1000000) / 1000;
            MI_ISP_AE_SetManualExpo(isp_dev, isp_chn, &stExpoVal);
            break;
        default:
            mode = E_SS_AE_MODE_A;
            MI_ISP_AE_SetExpoMode(isp_dev, isp_chn, &mode);
            break;
        }
        break;
    }

    case CMD_ISP_SET_FLICKER: {
        int                       nValue;
        MI_ISP_AE_FlickerExType_t Flicker;

        MI_ISP_AE_GetFlickerEx(isp_dev, isp_chn, &Flicker);
        nValue = *param;
        if (nValue == 50)
            Flicker.eFlickerType = E_SS_AE_FLICKER_TYPE_DETECT_50HZ;
        else if (nValue == 60)
            Flicker.eFlickerType = E_SS_AE_FLICKER_TYPE_DETECT_60HZ;
        else
            Flicker.eFlickerType = E_SS_AE_FLICKER_TYPE_DETECT_MAX;
        printf("FLICKER: %dHZ\n", Flicker.eFlickerType == E_SS_AE_FLICKER_TYPE_DETECT_60HZ ? 60 : 50);
        MI_ISP_AE_SetFlickerEx(isp_dev, isp_chn, &Flicker);
        break;
    }
    case CMD_ISP_SET_GAMMA: {
        int                       nValue;
        MI_U32                    j           = 0x0;
        MI_U16 *                  p_RGBGAMA_R = NULL;
        MI_U16 *                  p_RGBGAMA_G = NULL;
        MI_U16 *                  p_RGBGAMA_B = NULL;
        MI_ISP_IQ_RgbGammaType_t *Gamma       = new MI_ISP_IQ_RgbGammaType_t;

        MI_ISP_IQ_GetRgbGamma(isp_dev, isp_chn, Gamma);
        nValue = *param;
        switch (nValue)
        {
        case 0:
        case 10:
        case 20:
            Gamma->bEnable  = E_SS_IQ_TRUE;
            Gamma->enOpType = E_SS_IQ_OP_TYP_AUTO;
            break;
        case 30: // Low contrast
            Gamma->bEnable  = E_SS_IQ_TRUE;
            Gamma->enOpType = E_SS_IQ_OP_TYP_MANUAL;
            p_RGBGAMA_R     = RGBGamma_L[0];
            p_RGBGAMA_G     = RGBGamma_L[1];
            p_RGBGAMA_B     = RGBGamma_L[2];
            for (j = 0; j < 256; j++)
            {
                Gamma->stManual.stParaAPI.u16LutB[j] = *(p_RGBGAMA_B + j);
                Gamma->stManual.stParaAPI.u16LutG[j] = *(p_RGBGAMA_G + j);
                Gamma->stManual.stParaAPI.u16LutR[j] = *(p_RGBGAMA_R + j);
                // printf("%s:%d p_RGBGAMA_R+%d=%d, p_RGBGAMA_G+%d=%d, p_RGBGAMA_B+%d=%d\n", __func__,
                // __LINE__,j,*(p_RGBGAMA_R + j),j,*(p_RGBGAMA_G + j),j,*(p_RGBGAMA_B + j));
            }
            break;
        case 40:
            break;
        case 50: // Normal
            Gamma->bEnable  = E_SS_IQ_TRUE;
            Gamma->enOpType = E_SS_IQ_OP_TYP_MANUAL;
            p_RGBGAMA_R     = RGBGamma_N[0];
            p_RGBGAMA_G     = RGBGamma_N[1];
            p_RGBGAMA_B     = RGBGamma_N[2];
            for (j = 0; j < 256; j++)
            {
                Gamma->stManual.stParaAPI.u16LutB[j] = *(p_RGBGAMA_B + j);
                Gamma->stManual.stParaAPI.u16LutG[j] = *(p_RGBGAMA_G + j);
                Gamma->stManual.stParaAPI.u16LutR[j] = *(p_RGBGAMA_R + j);
                // printf("%s:%d p_RGBGAMA_R+%d=%d, p_RGBGAMA_G+%d=%d, p_RGBGAMA_B+%d=%d\n", __func__,
                // __LINE__,j,*(p_RGBGAMA_R + j),j,*(p_RGBGAMA_G + j),j,*(p_RGBGAMA_B + j));
            }
            break;
        case 60:
            break;
        case 70: // High contrast
            Gamma->bEnable  = E_SS_IQ_TRUE;
            Gamma->enOpType = E_SS_IQ_OP_TYP_MANUAL;
            p_RGBGAMA_R     = RGBGamma_H[0];
            p_RGBGAMA_G     = RGBGamma_H[1];
            p_RGBGAMA_B     = RGBGamma_H[2];
            for (j = 0; j < 256; j++)
            {
                Gamma->stManual.stParaAPI.u16LutB[j] = *(p_RGBGAMA_B + j);
                Gamma->stManual.stParaAPI.u16LutG[j] = *(p_RGBGAMA_G + j);
                Gamma->stManual.stParaAPI.u16LutR[j] = *(p_RGBGAMA_R + j);
                // printf("%s:%d p_RGBGAMA_R+%d=%d, p_RGBGAMA_G+%d=%d, p_RGBGAMA_B+%d=%d\n", __func__,
                // __LINE__,j,*(p_RGBGAMA_R + j),j,*(p_RGBGAMA_G + j),j,*(p_RGBGAMA_B + j));
            }
            break;
        case 80:
        case 90:
        case 100:
            Gamma->bEnable  = E_SS_IQ_TRUE;
            Gamma->enOpType = E_SS_IQ_OP_TYP_AUTO;
            break;
        default:
            break;
        }
        MI_ISP_IQ_SetRgbGamma(isp_dev, isp_chn, Gamma);

        for (j = 0; j < 256; j++)
        {
            printf("Gamma R:[%hu] Gamma G:[%hu] Gamma b:[%hu]\n", Gamma->stManual.stParaAPI.u16LutR[j],
                   Gamma->stManual.stParaAPI.u16LutG[j], Gamma->stManual.stParaAPI.u16LutB[j]);
        }
        delete Gamma;
        Gamma = NULL;
        break;
    }
    case CMD_ISP_SET_EFFECT: {
#if 0
        int                     nValue;
        MI_ISP_IQ_EFFECT_TYPE_t effectType;
        MI_ISP_IQ_GetEffect(isp_dev, isp_chn, &effectType);
        printf("EFFECT MODE: %d\n", effectType.bEnable);
        nValue = *param;
        if (nValue == 0)
            effectType.bEnable = E_SS_IQ_TRUE;
        else
            effectType.bEnable = SS_FALSE;
        MI_ISP_IQ_SetEffect(isp_dev, isp_chn, &effectType);
#endif
        break;
    }
    case CMD_ISP_SET_WINWGT_TYPE: {
        int                           nValue;
        MI_ISP_AE_WinWeightModeType_e WinWgtType;

        MI_ISP_AE_GetWinWgtType(isp_dev, isp_chn, &WinWgtType);
        nValue = *param;
        if (nValue == 0)
            WinWgtType = E_SS_AE_WEIGHT_AVERAGE;
        else if (nValue == 1)
            WinWgtType = E_SS_AE_WEIGHT_CENTER;
        else
            WinWgtType = E_SS_AE_WEIGHT_SPOT;
        MI_ISP_AE_SetWinWgtType(isp_dev, isp_chn, &WinWgtType);
        break;
    }
    default:
        break;
    }

    return 0;
}
#endif
