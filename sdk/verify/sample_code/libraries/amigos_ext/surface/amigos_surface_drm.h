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

#ifndef __AMIGOS_SURFACE_DRM_H__
#define __AMIGOS_SURFACE_DRM_H__

#include "amigos_base.h"

typedef struct DmaBufCfg_s
{
    unsigned int DmaBufWidth;
    unsigned int DmaBufHeight;
    unsigned int DmaBufFmt;
}DmaBufCfg_t;

template <class T>
class AmigosSurfaceDrm: public T
{
    public:
        explicit AmigosSurfaceDrm(const std::string &strInSection) : T(strInSection, E_SYS_MOD_DRM)
        {
            for (auto itMapOut = T::mapModOutputInfo.begin(); itMapOut != T::mapModOutputInfo.end(); itMapOut++)
            {
                stDmaBufCfg.DmaBufWidth = T::pDb->GetOutUint(itMapOut->second.curLoopId, "VID_W", 1024);
                stDmaBufCfg.DmaBufHeight = T::pDb->GetOutUint(itMapOut->second.curLoopId, "VID_H", 600);
                stDmaBufCfg.DmaBufFmt = T::pDb->GetOutUint(itMapOut->second.curLoopId, "VID_FMT", 11);
            }
        }
        virtual ~AmigosSurfaceDrm()
        {
        }
    protected:
        DmaBufCfg_t stDmaBufCfg;
};
#endif //__AMIGOS_SURFACE_DRM_H__
