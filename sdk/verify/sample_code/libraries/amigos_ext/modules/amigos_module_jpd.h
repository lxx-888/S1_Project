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
#ifndef __AMIGOS_MODULE_JPD_H__
#define __AMIGOS_MODULE_JPD_H__

#include "amigos_module_mi_base.h"
#include "amigos_surface_jpd.h"
#include "ss_packet.h"

class AmigosModuleJpd: public AmigosSurfaceJpd, public AmigosModuleMiBase
{
    public:
        explicit AmigosModuleJpd(const std::string &strInSection);
        ~AmigosModuleJpd() override;
        unsigned int GetModId() const override;
        unsigned int GetInputType(unsigned int port) const override;
        unsigned int GetOutputType(unsigned int port) const override;
    private:
        void _Init() override;
        void _Deinit() override;
        int _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;
        ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
        stream_packer  *_CreateInputStreamPacker(unsigned int inPortId, bool &bFast) override;
        static std::map<unsigned int, unsigned int> mapJpdCreateDev;
};
#endif

