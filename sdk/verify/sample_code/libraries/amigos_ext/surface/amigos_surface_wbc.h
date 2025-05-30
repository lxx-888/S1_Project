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

#ifndef __AMIGOS_SURFACE_WBC_H__
#define __AMIGOS_SURFACE_WBC_H__

#include "amigos_surface_base.h"

class AmigosSurfaceWbc : public AmigosSurfaceBase
{
public:
    struct WbcOutInfo
    {
        unsigned int uintDispDev;
        unsigned int uintSrcType;
        unsigned int uintWid;
        unsigned int uintHei;
        unsigned int uintCompressMode;
        std::string  strOutType;
        std::string  strOutFmt;
    public:
        WbcOutInfo()
        {
            uintDispDev      = 0;
            uintSrcType      = 0;
            uintWid          = 0;
            uintHei          = 0;
            uintCompressMode = 0;
        }
    };
    explicit AmigosSurfaceWbc(const std::string &strInSection);
    virtual ~AmigosSurfaceWbc();
    void GetInfo(std::map<unsigned int, WbcOutInfo> &info) const;
    void SetInfo(const std::map<unsigned int, WbcOutInfo> &info);

protected:
    std::map<unsigned int, WbcOutInfo> mapWbcInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_Wbc_H__
