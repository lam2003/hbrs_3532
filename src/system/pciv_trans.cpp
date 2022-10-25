//self
#include "system/pciv_trans.h"
#include "common/err_code.h"
#include "common/buffer.h"

namespace rs
{

using namespace pciv;

static int Recv(std::shared_ptr<PCIVComm> pciv_comm, int remote_id, int port, uint8_t *tmp_buf, int32_t buf_len, Buffer<allocator_1k> &msg_buf, const std::atomic<bool> &run, Msg &msg)
{
    int ret;
    do
    {
        ret = pciv_comm->Recv(remote_id, port, tmp_buf, buf_len, 500000); //500ms
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

PCIVTrans::~PCIVTrans()
{
    Close();
}

PCIVTrans::PCIVTrans() : trans_thread_(nullptr),
                         run_(false),
                         pciv_comm_(nullptr),
                         init_(false)
{
}

int32_t PCIVTrans::Initialize(std::shared_ptr<PCIVComm> pciv_comm, const MemoryInfo &mem_info)
{
    if (init_)
        return KInitialized;

    log_d("PCIV_TRANS start");

    mem_info_ = mem_info;
    pciv_comm_ = pciv_comm;

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

        int maxfd;
        int fds[4];
        fd_set read_fds;
        for (int i = 0; i < 4; i++)
        {
            fds[i] = HI_MPI_VENC_GetFd(i);
            if (fds[i] <= 0)
            {
                log_e("HI_MPI_VENC_GetFd failed");
                return;
            }
            if (maxfd <= fds[i])
                maxfd = fds[i];
            HI_MPI_VENC_StartRecvPic(i);
        }

        VENC_STREAM_S st;
        VENC_CHN_STAT_S stat;
        StreamInfo st_info;

        timeval val;
        bool full = false;
        int venc_chn = 0;
        int venc_chn_full = 0;

        Msg msg;
        uint8_t tmp_buf[1024];
        Buffer<allocator_1k> msg_buf;

        while (run_ || full)
        {
            FD_ZERO(&read_fds);
            for (int i = 0; i < 4; i++)
                FD_SET(fds[i], &read_fds);

            val.tv_sec = 1;
            val.tv_usec = 0;

            ret = select(maxfd + 1, &read_fds, NULL, NULL, &val);
            if (ret <= 0)
                continue;

            for (int i = 0; i < 4; i++)
            {
                venc_chn = i;
                if (!full)
                {
                    if (!FD_ISSET(fds[venc_chn], &read_fds))
                        continue;

                    memset(&st, 0, sizeof(st));
                    ret = HI_MPI_VENC_Query(venc_chn, &stat);
                    if (ret != KSuccess)
                    {
                        log_e("HI_MPI_VENC_Query failed with %#x", ret);
                        return;
                    }

                    st.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) *
                                                       stat.u32CurPacks);
                    st.u32PackCount = stat.u32CurPacks;

                    ret = HI_MPI_VENC_GetStream(venc_chn, &st, HI_IO_BLOCK);
                    if (ret != KSuccess)
                    {
                        log_e("HI_MPI_VENC_Query failed with %#x", ret);
                        return;
                    }
                }
                else
                {
                    full = false;
                    venc_chn = venc_chn_full;
                }

                uint32_t dma_len = 0;
                uint32_t len = 0;
                for (uint32_t j = 0; j < st.u32PackCount; j++)
                {
                    len += st.pstPack[j].u32Len[0];
                    len += st.pstPack[j].u32Len[1];
                }

                if (0 == (len % 4))
                {
                    dma_len = len;
                }
                else
                {
                    dma_len = len + (4 - (len % 4));
                }

                uint32_t free_len = (RS_PCIV_WINDOW_SIZE / 2) - buf_.len;
                if (free_len < sizeof(StreamInfo) + dma_len)
                {
                    full = true;
                    venc_chn_full = venc_chn;
                    break;
                }
                st_info.align_len = dma_len;
                st_info.len = len;
                st_info.pts = st.pstPack[0].u64PTS;
#if CHIP_TYPE == 1
                st_info.vdec_chn = Slave1_VencChn2VdecChn[i];
#else
                st_info.vdec_chn = Slave3_VencChn2VdecChn[i];
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

                buf_.len += (dma_len - len);

                ret = HI_MPI_VENC_ReleaseStream(venc_chn, &st);
                if (ret != KSuccess)
                {
                    log_e("HI_MPI_VENC_ReleaseStream failed with %#x", ret);
                    return;
                }

                free(st.pstPack);
            }

            ret = TransportData(pciv_comm_, pos_info_, buf_, mem_info_);
            if (ret != KSuccess)
                return;

            ret = Recv(pciv_comm_, RS_PCIV_MASTER_ID, RS_PCIV_TRANS_WRITE_PORT, tmp_buf, sizeof(tmp_buf), msg_buf, run_, msg);
            if (ret != KSuccess)
                return;

            if (msg.type == Msg::Type::READ_DONE)
            {
                PosInfo *tmp = reinterpret_cast<PosInfo *>(msg.data);
                pos_info_.start_pos = tmp->end_pos;
            }
            else
            {
                log_e("unknow msg type %d", msg.type);
                continue;
            }
        }

        for (int i = 0; i < 4; i++)
            HI_MPI_VENC_StopRecvPic(i);
    }));

    init_ = true;

    return KSuccess;
}

void PCIVTrans::Close()
{
    if (!init_)
        return;
    log_d("PCIV_TRANS stop");
    run_ = false;
    trans_thread_->join();
    trans_thread_.reset();
    trans_thread_ = nullptr;

    HI_MPI_SYS_Munmap(buf_.vir_addr, RS_PCIV_WINDOW_SIZE / 2);
    HI_MPI_VB_ReleaseBlock(buf_.blk);

    pciv_comm_ = nullptr;

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

int32_t PCIVTrans::TransportData(std::shared_ptr<PCIVComm> pciv_comm, PosInfo &pos_info, PCIVBuffer &buf, const MemoryInfo &mem_info)
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

    ret = pciv_comm->Send(RS_PCIV_MASTER_ID, RS_PCIV_TRANS_READ_PORT, reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
    if (ret != KSuccess)
        return ret;

    buf.len = 0;

    return KSuccess;
}

}; // namespace rs