//self
#include "system/pciv_trans.h"
#include "common/buffer.h"
//stl
#include <map>

namespace rs
{

using namespace pciv;

static std::map<int, int> Slave1_VencChn2VdecChn = { //从片1 VENC通道与VDEC通道映射关系
    {0, 3}};
static std::map<int, int> Slave3_VencChn2VdecChn = { //从片3 VENC通道与VDEC通道映射关系
    {0, 0},
    {1, 1},
    {2, 2}};

PCIVTrans::~PCIVTrans()
{
    Close();
}

PCIVTrans::PCIVTrans() : run_(false),
                         trans_thread_(nullptr),
                         recv_msg_thread_(nullptr),
                         ctx_(nullptr),
                         init_(false)
{
}

PCIVTrans *PCIVTrans::Instance()
{
    static PCIVTrans *instance = new PCIVTrans;
    return instance;
}

static int Recv(pciv::Context *ctx, int remote_id, int port, uint8_t *tmp_buf, int32_t buf_len, rs::Buffer<allocator_1k> &msg_buf, const std::atomic<bool> &run, Msg &msg)
{
    int ret;
    do
    {
        ret = ctx->Recv(remote_id, port, tmp_buf, buf_len, 500000); //500ms
        if (ret > 0)
        {
            if (!msg_buf.Append(tmp_buf, ret))
            {
                log_e("append data to msg_buf failed");
                return KNotEnoughBuf;
            }
        }
        else if (ret < 0)
            return ret;
    } while (run && msg_buf.Size() < sizeof(msg));

    if (msg_buf.Size() >= sizeof(msg))
    {
        msg_buf.Get(reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
        msg_buf.Consume(sizeof(msg));
    }
    return KSuccess;
}

int32_t PCIVTrans::Initialize(Context *ctx, const MemoryInfo &mem_info)
{
    if (init_)
        return KInitialized;

    mem_info_ = mem_info;
    ctx_ = ctx;

    memset(&pos_info_, 0, sizeof(pos_info_));
    memset(&buf_, 0, sizeof(buf_));

    buf_.blk = HI_MPI_VB_GetBlock(VB_INVALID_POOLID, RS_PCIV_WINDOW_SIZE / 2, nullptr);
    if (buf_.blk == VB_INVALID_HANDLE)
    {
        log_e("HI_MPI_VB_GetBlock failed");
        return KSDKError;
    }

    buf_.phy_addr = HI_MPI_VB_Handle2PhysAddr(buf_.blk);
    if (buf_.phy_addr == 0)
    {
        log_e("HI_MPI_VB_Handle2PhysAddr failed");
        return KSDKError;
    }

    buf_.vir_addr = reinterpret_cast<uint8_t *>(HI_MPI_SYS_Mmap(buf_.phy_addr,
                                                                RS_PCIV_WINDOW_SIZE / 2));
    if (buf_.vir_addr == nullptr)
    {
        log_e("HI_MPI_SYS_Mmap failed");
        return KSDKError;
    }

    run_ = true;
    trans_thread_ = std::unique_ptr<std::thread>(new std::thread([this]() {
        int ret;
        while (run_)
        {
            usleep(10000); //10ms
            std::unique_lock<std::mutex> lock(mux_);
            if (buf_.len > 0)
            {
                ret = TransportData(ctx_, pos_info_, buf_, mem_info_);
                if (ret != KSuccess)
                    return;
            }
        }
    }));
    recv_msg_thread_ = std::unique_ptr<std::thread>(new std::thread([this]() {
        rs::pciv::Msg msg;
        uint8_t tmp_buf[1024];
        rs::Buffer<rs::allocator_1k> msg_buf;
        while (run_)
        {
            if (Recv(ctx_, RS_PCIV_MASTER_ID, ctx_->GetTransWritePort(), tmp_buf, sizeof(tmp_buf), msg_buf, run_, msg) == KSuccess && run_)
            {
                if (msg.type == Msg::Type::READ_DONE)
                {
                    PosInfo *tmp = reinterpret_cast<PosInfo *>(msg.data);
                    std::unique_lock<std::mutex> lock(mux_);
                    pos_info_.start_pos = tmp->end_pos;
                }
                else
                {
                    log_e("unknow msg type %d", msg.type);
                    continue;
                }
            }
        }
    }));

    init_ = true;

    return KSuccess;
}

void PCIVTrans::Close()
{
    if (!init_)
        return;

    run_ = false;
    trans_thread_->join();
    trans_thread_.reset();
    trans_thread_ = nullptr;

    recv_msg_thread_->join();
    recv_msg_thread_.reset();
    recv_msg_thread_ = nullptr;

    HI_MPI_SYS_Munmap(buf_.vir_addr, RS_PCIV_WINDOW_SIZE / 2);
    HI_MPI_VB_ReleaseBlock(buf_.blk);

    init_ = false;
}

int32_t PCIVTrans::QueryWritePos(const PosInfo &pos_info, int len)
{
    if (pos_info.end_pos >= pos_info.start_pos)
    {
        if ((pos_info.end_pos + len) <= RS_PCIV_WINDOW_SIZE)
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

int32_t PCIVTrans::TransportData(Context *ctx, PosInfo &pos_info, pciv::Buffer &buf, const MemoryInfo &mem_info)
{
    int32_t ret;

    int32_t write_pos = QueryWritePos(pos_info, buf.len);
    if (write_pos == -1)
    {
        log_e("query write pos failed");
        return KNotEnoughBuf;
    }

    PCIV_DMA_BLOCK_S dma_blks[PCIV_MAX_DMABLK];
    dma_blks[0].u32BlkSize = buf.len;
    dma_blks[0].u32SrcAddr = buf.phy_addr;
    dma_blks[0].u32DstAddr = mem_info.phy_addr + write_pos;

    PCIV_DMA_TASK_S dma_ask;
    dma_ask.pBlock = &dma_blks[0];
    dma_ask.u32Count = 1;
    dma_ask.bRead = HI_FALSE;

    ret = HI_MPI_PCIV_DmaTask(&dma_ask);
    while (ret == HI_ERR_PCIV_BUSY)
    {
        ret = HI_MPI_PCIV_DmaTask(&dma_ask);
    }
    if (ret != KSuccess)
    {
        log_e("HI_MPI_PCIV_DmaTask failed with %#x", ret);
        return KSDKError;
    }

    pos_info.end_pos = write_pos + buf.len;

    PosInfo tmp;
    tmp.start_pos = write_pos;
    tmp.end_pos = write_pos + buf.len;

    Msg msg;
    msg.type = Msg::Type::WRITE_DONE;
    memcpy(msg.data, &tmp, sizeof(tmp));

    ret = ctx->Send(RS_PCIV_MASTER_ID, ctx->GetTransReadPort(), reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
    if (ret != KSuccess)
        return ret;

    buf.len = 0;

    return KSuccess;
}

void PCIVTrans::OnFrame(const VENC_STREAM_S &st, int chn)
{
    if (!init_)
        return;
    StreamInfo st_info;

    int32_t len = 0;
    for (uint32_t i = 0; i < st.u32PackCount; i++)
    {
        len += st.pstPack[i].u32Len[0];
        len += st.pstPack[i].u32Len[1];
    }

    int32_t align_len = (len % 4 == 0 ? len : (len + (4 - len % 4)));

    std::unique_lock<std::mutex> lock(mux_);
    uint32_t free_len = (RS_PCIV_WINDOW_SIZE / 2) - buf_.len;
    if (free_len < sizeof(StreamInfo) + align_len)
    {
        log_d("local buffer not enough");
        return;
    }

    st_info.align_len = align_len;
    st_info.len = len;
    st_info.pts = st.pstPack[0].u64PTS;
#if CHIP_TYPE == 1
    st_info.vdec_chn = Slave1_VencChn2VdecChn[chn];
#elif CHIP_TYPE == 3
    st_info.vdec_chn = Slave3_VencChn2VdecChn[chn];
#endif
    memcpy(buf_.vir_addr + buf_.len, &st_info, sizeof(st_info));
    buf_.len += sizeof(st_info);

    for (uint32_t i = 0; i < st.u32PackCount; i++)
    {
        memcpy(buf_.vir_addr + buf_.len,
               st.pstPack[i].pu8Addr[0],
               st.pstPack[i].u32Len[0]);

        buf_.len += st.pstPack[i].u32Len[0];

        memcpy(buf_.vir_addr + buf_.len,
               st.pstPack[i].pu8Addr[1],
               st.pstPack[i].u32Len[1]);
        buf_.len += st.pstPack[i].u32Len[1];
    }

    buf_.len += (align_len - len);
}
}; // namespace rs