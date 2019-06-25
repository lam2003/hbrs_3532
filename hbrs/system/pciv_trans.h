#pragma once

#include "common/global.h"
#include "system/pciv_comm.h"

namespace rs
{

class PCIVTrans
{
public:
    virtual ~PCIVTrans();

    static PCIVTrans *Instance();

    int32_t Initialize(pciv::Context *ctx, const pciv::MemoryInfo &mem_info);

    void Close();

    int32_t TransportData(uint32_t local_phy_addr, int32_t len);

protected:
    static int32_t QueryWritePos(pciv::PosInfo &pos_info, int32_t len);

    explicit PCIVTrans();

private:
    pciv::MemoryInfo mem_info_;
    pciv::PosInfo pos_info_;
    pciv::Context *ctx_;
    bool init_;
};
}; // namespace rs