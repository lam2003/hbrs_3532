#include "system/pciv_trans.h"

namespace rs
{

using namespace pciv;

PCIVTrans::~PCIVTrans()
{
}

PCIVTrans::PCIVTrans()
{
}

PCIVTrans *PCIVTrans::Instance()
{
    static PCIVTrans *instance = new PCIVTrans;
    return instance;
}

int32_t PCIVTrans::Initialize(Context *ctx, const MemoryInfo &mem_info)
{
    if (init_)
        return KInitialized;

    mem_info_ = mem_info;
    pos_info_.start_pos = 0;
    pos_info_.end_pos = 0;
    ctx_ = ctx;

    return KSuccess;
}

int32_t PCIVTrans::QueryWritePos(PosInfo &pos_info, int32_t len)
{
    if (pos_info.end_pos >= pos_info.start_pos)
    {
        if ((pos_info.end_pos + len) <= PCIV_WINDOW_SIZE)
            return pos_info.end_pos;
        else if (len < pos_info.start_pos)
            return 0;
    }
    else
    {
        if ((pos_info.end_pos + len) < pos_info.start_pos)
            return pos_info.end_pos;
    }
    return -1;
}

int32_t PCIVTrans::TransportData(uint32_t local_phy_addr, int32_t len)
{
    if (!init_)
        return KUnInitialized;

    int32_t ret;

    int32_t write_pos = QueryWritePos(pos_info_, len);
    if (write_pos == -1)
        return KNotEnoughBuf;

    PCIV_DMA_BLOCK_S dma_blks[PCIV_MAX_DMABLK];
    dma_blks[0].u32BlkSize = len;
    dma_blks[0].u32SrcAddr = local_phy_addr;
    dma_blks[0].u32DstAddr = mem_info_.phy_addr + write_pos;

    PCIV_DMA_TASK_S dma_ask;
    dma_ask.pBlock = &dma_blks[0];
    dma_ask.u32Count = 1;
    dma_ask.bRead = HI_FALSE;

    ret = HI_MPI_PCIV_DmaTask(&dma_ask);
    while (ret == HI_ERR_PCIV_BUSY)
    {
        usleep(10000); //10ms
        ret = HI_MPI_PCIV_DmaTask(&dma_ask);
    }
    if (ret != KSuccess)
    {
        log_e("HI_MPI_PCIV_DmaTask failed with %#x", ret);
        return KSDKError;
    }

    pos_info_.end_pos = write_pos + len;

    Msg msg;
    msg.type = Msg::Type::WRITE_DONE;
    memcpy(msg.data, &pos_info_, sizeof(pos_info_));
    ret = ctx_->Send(PCIV_MASTER_ID, ctx_->GetTransReadPort(), reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
    if (ret != KSuccess)
        return ret;

    return KSuccess;
}

}; // namespace rs