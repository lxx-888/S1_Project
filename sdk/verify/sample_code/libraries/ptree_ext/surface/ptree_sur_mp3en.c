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
#include "ptree_sur_mp3en.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static PARENA_Tag_t *_PTREE_SUR_MP3EN_OccupyInfo(PTREE_SUR_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_MP3EN_OccupyInInfo(PTREE_SUR_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_MP3EN_OccupyOutInfo(PTREE_SUR_Obj_t *sysSur, void *pArena);
static int           _PTREE_SUR_MP3EN_LoadDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_Info_t *sysInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_MP3EN_LoadInDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_InInfo_t *sysInInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_MP3EN_LoadOutDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_OutInfo_t *sysOutInfo, PTREE_DB_Obj_t db);
static void _PTREE_SUR_MP3EN_Free(PTREE_SUR_Obj_t *sysSur);

static const PTREE_SUR_Ops_t G_PTREE_SUR_MP3EN_OPS = {
    .occupyInfo    = _PTREE_SUR_MP3EN_OccupyInfo,
    .occupyInInfo  = _PTREE_SUR_MP3EN_OccupyInInfo,
    .occupyOutInfo = _PTREE_SUR_MP3EN_OccupyOutInfo,
    .loadDb        = _PTREE_SUR_MP3EN_LoadDb,
    .loadInDb      = _PTREE_SUR_MP3EN_LoadInDb,
    .loadOutDb     = _PTREE_SUR_MP3EN_LoadOutDb,
};
static const PTREE_SUR_Hook_t G_PTREE_SUR_MP3EN_HOOK = {
    .free = _PTREE_SUR_MP3EN_Free,
};

static PARENA_Tag_t *_PTREE_SUR_MP3EN_OccupyInfo(PTREE_SUR_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_MP3EN_Info_t, base, PTREE_SUR_Info_t);
}
static PARENA_Tag_t *_PTREE_SUR_MP3EN_OccupyInInfo(PTREE_SUR_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_MP3EN_InInfo_t, base, PTREE_SUR_InInfo_t);
}
static PARENA_Tag_t *_PTREE_SUR_MP3EN_OccupyOutInfo(PTREE_SUR_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_MP3EN_OutInfo_t, base, PTREE_SUR_OutInfo_t);
}
static int _PTREE_SUR_MP3EN_LoadDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_MP3EN_Info_t *mp3enInfo = CONTAINER_OF(sysInfo, PTREE_SUR_MP3EN_Info_t, base);
    (void)sysSur;
    (void)db;

    mp3enInfo->bitRate = (unsigned int)PTREE_DB_GetInt(db, "BIT_RATE");
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_MP3EN_LoadInDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_InInfo_t *sysInInfo, PTREE_DB_Obj_t db)
{
    // PTREE_SUR_MP3EN_InInfo_t *mp3enInInfo = CONTAINER_OF(sysInInfo, PTREE_SUR_MP3EN_InInfo_t, base);
    (void)sysSur;
    (void)sysInInfo;
    (void)db;

    return SSOS_DEF_OK;
}
static int _PTREE_SUR_MP3EN_LoadOutDb(PTREE_SUR_Obj_t *sysSur, PTREE_SUR_OutInfo_t *sysOutInfo, PTREE_DB_Obj_t db)
{
    // PTREE_SUR_MP3EN_OutInfo_t *mp3enOutInfo = CONTAINER_OF(sysOutInfo, PTREE_SUR_MP3EN_OutInfo_t, base);
    (void)sysSur;
    (void)sysOutInfo;
    (void)db;

    return SSOS_DEF_OK;
}
static void _PTREE_SUR_MP3EN_Free(PTREE_SUR_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_MP3EN_New(void)
{
    PTREE_SUR_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_Init(sysSur, &G_PTREE_SUR_MP3EN_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_Register(sysSur, &G_PTREE_SUR_MP3EN_HOOK);
    return sysSur;
}

PTREE_MAKER_SUR_INIT(MP3EN, PTREE_SUR_MP3EN_New);
