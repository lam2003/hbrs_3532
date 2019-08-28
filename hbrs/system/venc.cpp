//self
#include "system/venc.h"
#include "common/err_code.h"
#include "common/buffer.h"

namespace rs
{
using namespace venc;

VideoEncode::VideoEncode() : sink_(nullptr),
                             thread_(nullptr),
                             run_(false),
                             init_(false)
{
}

VideoEncode::~VideoEncode()
{
    Close();
}

int32_t VideoEncode::Initialize(const Params &params)
{
    if (init_)
        return KInitialized;

    int ret;

    log_d("VENC start,grp:%d,chn:%d,width:%d,height:%d,src_frame_rate:%d,dst_frame_rate:%d,profile:%d,bitrate:%d,mode:%d",
          params.grp,
          params.chn,
          params.width,
          params.height,
          params.src_frame_rate,
          params.dst_frame_rate,
          params.profile,
          params.bitrate,
          params.mode);
    params_ = params;

    VENC_ATTR_H264_S h264_attr;
    memset(&h264_attr, 0, sizeof(h264_attr));

    h264_attr.u32MaxPicWidth = params_.width;
    h264_attr.u32MaxPicHeight = params_.height;
    h264_attr.u32PicWidth = params_.width;
    h264_attr.u32PicHeight = params_.height;
    h264_attr.u32BufSize = params_.width * params_.height * 2;
    h264_attr.u32Profile = params_.profile;
    h264_attr.bByFrame = HI_FALSE;
    h264_attr.bField = HI_FALSE;
    h264_attr.bMainStream = HI_TRUE;
    h264_attr.u32Priority = 0;
    h264_attr.bVIField = HI_FALSE;

    VENC_CHN_ATTR_S chn_attr;
    memset(&chn_attr, 0, sizeof(chn_attr));
    chn_attr.stVeAttr.enType = PT_H264;
    memcpy(&chn_attr.stVeAttr.stAttrH264e, &h264_attr, sizeof(h264_attr));

    switch (params_.mode)
    {
    case VENC_RC_MODE_H264CBR:
    {
        VENC_ATTR_H264_CBR_S cbr_attr;
        memset(&cbr_attr, 0, sizeof(cbr_attr));
        cbr_attr.u32Gop = params_.dst_frame_rate;
        cbr_attr.u32StatTime = 1;
        cbr_attr.u32ViFrmRate = params_.src_frame_rate;
        cbr_attr.fr32TargetFrmRate = params_.dst_frame_rate;
        cbr_attr.u32BitRate = params_.bitrate;
        cbr_attr.u32FluctuateLevel = 0;
        chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
        memcpy(&chn_attr.stRcAttr.stAttrH264Cbr, &cbr_attr,
               sizeof(cbr_attr));
        break;
    }
    case VENC_RC_MODE_H264VBR:
    {
        VENC_ATTR_H264_VBR_S vbr_attr;
        memset(&vbr_attr, 0, sizeof(vbr_attr));
        vbr_attr.u32Gop = params_.dst_frame_rate;
        vbr_attr.u32StatTime = 1;
        vbr_attr.u32ViFrmRate = params_.src_frame_rate;
        vbr_attr.fr32TargetFrmRate = params_.dst_frame_rate;
        vbr_attr.u32MinQp = 24;
        vbr_attr.u32MaxQp = 32;
        vbr_attr.u32MaxBitRate = params_.bitrate;
        chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
        memcpy(&chn_attr.stRcAttr.stAttrH264Vbr, &vbr_attr,
               sizeof(chn_attr));
        break;
    }
    default:
        RS_ASSERT(0);
    }

    ret = HI_MPI_VENC_CreateGroup(params_.grp);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VENC_CreateGroup failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_VENC_CreateChn(params_.chn, &chn_attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VENC_CreateChn failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_VENC_RegisterChn(params_.grp, params_.chn);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VENC_RegisterChn failed with %#x", ret);
        return KSDKError;
    }

    run_ = true;
    thread_ = std::unique_ptr<std::thread>(new std::thread([this]() {
        int32_t ret;

        MMZBuffer mmz_buffer(2 * 1024 * 1024);
        MMZBuffer packet_mmz_buffer(128 * 1024);

        uint8_t *packet_buf = packet_mmz_buffer.vir_addr;

        ret = HI_MPI_VENC_StartRecvPic(params_.chn);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_VENC_StartRecvPic failed with %#x", ret);
            return;
        }

        int32_t fd = HI_MPI_VENC_GetFd(params_.chn);
        if (fd < 0)
        {
            log_e("HI_MPI_VENC_GetFd failed");
            return;
        }

        fd_set fds;
        timeval tv;

        VENC_STREAM_S stream;
        VENC_CHN_STAT_S chn_stat;

        while (run_)
        {
            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            tv.tv_sec = 0;
            tv.tv_usec = 100000; //100ms

            ret = select(fd + 1, &fds, NULL, NULL, &tv);
            if (ret <= 0)
                continue;

            memset(&stream, 0, sizeof(stream));
            memset(&chn_stat, 0, sizeof(chn_stat));
            ret = HI_MPI_VENC_Query(params_.chn, &chn_stat);
            if (ret != KSuccess)
            {
                log_e("HI_MPI_VENC_Query failed with %#x", ret);
                return;
            }

            if (!chn_stat.u32CurPacks)
            {
                log_e("current frame is null");
                return;
            }

            stream.pstPack = reinterpret_cast<VENC_PACK_S *>(packet_buf);
            stream.u32PackCount = chn_stat.u32CurPacks;

            ret = HI_MPI_VENC_GetStream(params_.chn, &stream, HI_TRUE);
            if (ret != KSuccess)
            {
                log_e("HI_MPI_VENC_GetStream failed with %#x", ret);
                return;
            }
            {
                std::unique_lock<std::mutex> lock(sink_mux_);
                if (sink_ != nullptr)
                    sink_->OnFrame(stream,params_.chn);
            }

            ret = HI_MPI_VENC_ReleaseStream(params_.chn, &stream);
            if (HI_SUCCESS != ret)
            {
                log_e("HI_MPI_VENC_ReleaseStream failed with %#x", ret);
                return;
            }
        }

        ret = HI_MPI_VENC_StopRecvPic(params_.chn);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_VENC_StopRecvPic failed with %#x", ret);
            return;
        }
    }));

    init_ = true;
    return KSuccess;
}

void VideoEncode::Close()
{
    if (!init_)
        return;
    int ret;

    log_d("VENC stop,grp:%d,chn:%d", params_.grp, params_.chn);

    run_ = false;
    thread_->join();
    thread_.reset();
    thread_ = nullptr;

    ret = HI_MPI_VENC_UnRegisterChn(params_.chn);
    if (ret != KSuccess)
        log_e("HI_MPI_VENC_UnRegisterChn failed with %#x", ret);
    ret = HI_MPI_VENC_DestroyChn(params_.chn);
    if (ret != KSuccess)
        log_e("HI_MPI_VENC_DestroyChn failed with %#x", ret);
    ret = HI_MPI_VENC_DestroyGroup(params_.grp);
    if (ret != KSuccess)
        log_e("HI_MPI_VENC_DestroyGroup failed with %#x", ret);

    sink_ = nullptr;
    init_ = false;
}

void VideoEncode::SetVideoSink(std::shared_ptr<VideoSink<VENC_STREAM_S>> sink)
{
    std::unique_lock<std::mutex> lock(sink_mux_);
    sink_ = sink;
}

} // namespace rs
