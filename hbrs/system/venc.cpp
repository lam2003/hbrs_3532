//self
#include "system/venc.h"
#include "common/err_code.h"
#include "common/buffer.h"

namespace rs
{
using namespace venc;

VideoEncode::VideoEncode() : init_(false)
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

    init_ = true;
    return KSuccess;
}

void VideoEncode::Close()
{
    if (!init_)
        return;
    int ret;

    log_d("VENC stop,grp:%d,chn:%d", params_.grp, params_.chn);

    ret = HI_MPI_VENC_UnRegisterChn(params_.chn);
    if (ret != KSuccess)
        log_e("HI_MPI_VENC_UnRegisterChn failed with %#x", ret);
    ret = HI_MPI_VENC_DestroyChn(params_.chn);
    if (ret != KSuccess)
        log_e("HI_MPI_VENC_DestroyChn failed with %#x", ret);
    ret = HI_MPI_VENC_DestroyGroup(params_.grp);
    if (ret != KSuccess)
        log_e("HI_MPI_VENC_DestroyGroup failed with %#x", ret);

    init_ = false;
}

} // namespace rs
