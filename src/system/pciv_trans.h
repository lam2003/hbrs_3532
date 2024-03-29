#pragma once

//self
#include "system/pciv_comm.h"

namespace rs
{

struct PCIVBuffer
{
    uint32_t phy_addr;
    uint8_t *vir_addr;
    int len;
    VB_BLK blk;
};

class PCIVTrans 
{
public:
    explicit PCIVTrans();

    virtual ~PCIVTrans();

    int32_t Initialize(std::shared_ptr<PCIVComm> pciv_comm, const pciv::MemoryInfo &mem_info);

    void Close();

protected:

    static int32_t TransportData(std::shared_ptr<PCIVComm> pciv_comm, pciv::PosInfo &pos_info, PCIVBuffer &buf, const pciv::MemoryInfo &mem_info);

    static int32_t QueryWritePos(const pciv::PosInfo &pos_info, int len);

private:
    pciv::MemoryInfo mem_info_;
    pciv::PosInfo pos_info_;
    PCIVBuffer buf_;
    std::unique_ptr<std::thread> trans_thread_;
    std::atomic<bool> run_;
    std::shared_ptr<PCIVComm> pciv_comm_;
    bool init_;
};
}; // namespace rs