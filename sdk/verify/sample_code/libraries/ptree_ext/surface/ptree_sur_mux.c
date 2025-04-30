/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include "ssos_list.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "mi_common_datatype.h"
#include "ptree_sur_sys.h"
#include "ptree_enum.h"
#include "ptree_packet.h"
#include "ptree_maker.h"
#include "ptree_sur_mux.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

PTREE_ENUM_DEFINE(PTREE_SUR_MUX_Type_e, {E_PTREE_SUR_MUX_TYPE_TS, "ts"}, {E_PTREE_SUR_MUX_TYPE_MP4, "mp4"}, )

static PARENA_Tag_t *_PTREE_SUR_MUX_OccupyInfo(PTREE_SUR_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_MUX_OccupyInInfo(PTREE_SUR_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_MUX_OccupyOutInfo(PTREE_SUR_Obj_t *sysSur, void *pArena);
static int           _PTREE_SUR_MUX_LoadDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_Info_t *sysInfo, PTREE_DB_Obj_t db);
static int           _PTREE_SUR_MUX_LoadInDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_InInfo_t *sysInInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_MUX_LoadOutDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_OutInfo_t *sysOutInfo, PTREE_DB_Obj_t db);
static void _PTREE_SUR_MUX_Free(PTREE_SUR_Obj_t *sysSur);

static const PTREE_SUR_Ops_t G_PTREE_SUR_MUX_OPS = {
    .occupyInfo    = _PTREE_SUR_MUX_OccupyInfo,
    .occupyInInfo  = _PTREE_SUR_MUX_OccupyInInfo,
    .occupyOutInfo = _PTREE_SUR_MUX_OccupyOutInfo,
    .loadDb        = _PTREE_SUR_MUX_LoadDb,
    .loadInDb      = _PTREE_SUR_MUX_LoadInDb,
    .loadOutDb     = _PTREE_SUR_MUX_LoadOutDb,
};
static const PTREE_SUR_Hook_t G_PTREE_SUR_MUX_HOOK = {
    .free = _PTREE_SUR_MUX_Free,
};

static PARENA_Tag_t *_PTREE_SUR_MUX_OccupyInfo(PTREE_SUR_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_MUX_Info_t, base, PTREE_SUR_Info_t);
}
static PARENA_Tag_t *_PTREE_SUR_MUX_OccupyInInfo(PTREE_SUR_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_MUX_InInfo_t, base, PTREE_SUR_InInfo_t);
}
static PARENA_Tag_t *_PTREE_SUR_MUX_OccupyOutInfo(PTREE_SUR_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_MUX_OutInfo_t, base, PTREE_SUR_OutInfo_t);
}
static int _PTREE_SUR_MUX_LoadDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_MUX_Info_t *muxInfo = CONTAINER_OF(sysInfo, PTREE_SUR_MUX_Info_t, base);
    const char *          str     = NULL;
    (void)sysSur;

    muxInfo->bitRateV       = (unsigned int)PTREE_DB_GetInt(db, "V_BIT_RATE");
    muxInfo->bitRateA       = (unsigned int)PTREE_DB_GetInt(db, "A_BIT_RATE");
    muxInfo->audioFrameSize = (unsigned int)PTREE_DB_GetInt(db, "AUDIO_FRAME_SIZE");
    str                     = PTREE_DB_GetStr(db, "FORMAT");
    if (str && strlen(str) > 1)
    {
        muxInfo->format = PTREE_ENUM_FROM_STR(PTREE_SUR_MUX_Type_e, str);
    }
    if (E_PTREE_SUR_MUX_TYPE_MP4 == muxInfo->format)
    {
        PTREE_DB_ProcessKey(db, "FORMAT_PARAM");
        str = PTREE_DB_GetStr(db, "FILE");
        snprintf(muxInfo->fileName, 64, "%s", str);
        PTREE_DB_ProcessBack(db);
    }

    return SSOS_DEF_OK;
}
static int _PTREE_SUR_MUX_LoadInDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_InInfo_t *sysInInfo, PTREE_DB_Obj_t db)
{
    // PTREE_SUR_MUX_InInfo_t *muxInInfo = CONTAINER_OF(sysInInfo, PTREE_SUR_MUX_InInfo_t, base);
    (void)sysSur;
    (void)sysInInfo;
    (void)db;
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_MUX_LoadOutDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_OutInfo_t *sysOutInfo, PTREE_DB_Obj_t db)
{
    // PTREE_SUR_MUX_OutInfo_t *muxOutInfo = CONTAINER_OF(sysOutInfo, PTREE_SUR_MUX_OutInfo_t, base);
    (void)sysSur;
    (void)sysOutInfo;
    (void)db;

    return SSOS_DEF_OK;
}
static void _PTREE_SUR_MUX_Free(PTREE_SUR_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_MUX_New(void)
{
    PTREE_SUR_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_Init(sysSur, &G_PTREE_SUR_MUX_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_Register(sysSur, &G_PTREE_SUR_MUX_HOOK);
    return sysSur;
}

PTREE_MAKER_SUR_INIT(MUX, PTREE_SUR_MUX_New);
