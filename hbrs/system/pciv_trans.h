#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

#include "common/global.h"
#include "system/pciv_comm.h"

namespace rs
{
namespace pciv
{
struct Buffer
{
    uint32_t phy_addr;
    uint8_t *vir_addr;
    int len;
    VB_BLK blk;
};
} // namespace pciv
class PCIVTrans : public VideoSink<VENC_STREAM_S>
{
public:
    virtual ~PCIVTrans();

    static PCIVTrans *Instance();

    int32_t Initialize(pciv::Context *ctx, const pciv::MemoryInfo &mem_info);

    void Close();

    void OnFrame(const VENC_STREAM_S &st, int chn) override;

protected:
    int32_t TransportData();

    int32_t QueryWritePos();

    explicit PCIVTrans();

private:
    pciv::MemoryInfo mem_info_;
    pciv::PosInfo pos_info_;
    pciv::Buffer buf_;
    std::mutex mux_;
    std::atomic<bool> run_;
    std::unique_ptr<std::thread> trans_thread_;
    std::unique_ptr<std::thread> recv_msg_thread_;
    pciv::Context *ctx_;
    bool init_;
};
}; // namespace rs