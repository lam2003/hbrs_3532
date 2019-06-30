#pragma once
//stl
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
//self
#include "common/global.h"
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

class PCIVTrans : public VideoSink<VENC_STREAM_S>
{
public:
    virtual ~PCIVTrans();

    static PCIVTrans *Instance();

    int32_t Initialize(pciv::Context *ctx, const pciv::MemoryInfo &mem_info);

    void Close();

    void OnFrame(const VENC_STREAM_S &st, int chn) override;

protected:
    static int32_t TransportData(pciv::Context *ctx, pciv::PosInfo &pos_info, PCIVBuffer &buf, const pciv::MemoryInfo &mem_info);

    static int32_t QueryWritePos(const pciv::PosInfo &pos_info, int len);

    explicit PCIVTrans();

private:
    pciv::MemoryInfo mem_info_;
    pciv::PosInfo pos_info_;
    PCIVBuffer buf_;
    std::mutex mux_;
    std::atomic<bool> run_;
    std::unique_ptr<std::thread> trans_thread_;
    std::unique_ptr<std::thread> recv_msg_thread_;
    pciv::Context *ctx_;
    bool init_;
};
}; // namespace rs