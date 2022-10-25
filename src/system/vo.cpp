//self
#include "system/vo.h"
#include "common/utils.h"
#include "common/err_code.h"

namespace rs
{

using namespace vo;

const HI_HDMI_ID_E VideoOutput::HdmiDev = HI_HDMI_ID_0;

VideoOutput::VideoOutput() : init_(false)
{
}

VideoOutput::~VideoOutput()
{
    Close();
}

int32_t VideoOutput::Initialize(const Params &params)
{
    if (init_)
        return KInitialized;

    int32_t ret;

    log_d("VO start,dev:%d,intf_type:%d,intf_sync:%d", params.dev, params.intf_type, params.intf_sync);

    params_ = params;

    if (params_.intf_type & VO_INTF_HDMI)
    {
        ret = StartHDMI(HdmiDev, params_.intf_sync);
        if (ret != KSuccess)
            return ret;
    }

    ret = StartDevLayer(params_.dev, params_.intf_type, params_.intf_sync);
    if (ret != KSuccess)
        return ret;

    init_ = true;

    return KSuccess;
}

int32_t VideoOutput::StartHDMI(HI_HDMI_ID_E dev, VO_INTF_SYNC_E intf_sync)
{
    int32_t ret;

    HI_HDMI_INIT_PARA_S param;
    HI_HDMI_ATTR_S attr;
    HI_HDMI_VIDEO_FMT_E fmt;

    memset(&param, 0, sizeof(param));
    param.enForceMode = HI_HDMI_FORCE_HDMI;
    param.pCallBackArgs = NULL;
    param.pfnHdmiEventCallback = NULL;

    ret = HI_MPI_HDMI_Init(&param);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_HDMI_Init failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_HDMI_Open(dev);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_HDMI_Open failed with %#x", ret);
        return KSDKError;
    }

    memset(&attr, 0, sizeof(attr));
    ret = HI_MPI_HDMI_GetAttr(dev, &attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_HDMI_GetAttr failed with %#x", ret);
        return KSDKError;
    }

    fmt = Utils::GetHDMIFmt(intf_sync);

    attr.bEnableHdmi = HI_TRUE;
    attr.bEnableVideo = HI_TRUE;
    attr.enVideoFmt = fmt;
    attr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
    attr.enDeepColorMode = HI_HDMI_DEEP_COLOR_OFF;
    attr.bxvYCCMode = HI_FALSE;
    attr.bEnableAudio = HI_FALSE;
    attr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
    attr.bIsMultiChannel = HI_FALSE;
    attr.enBitDepth = HI_HDMI_BIT_DEPTH_16;
    attr.bEnableAviInfoFrame = HI_TRUE;
    attr.bEnableAudInfoFrame = HI_TRUE;
    attr.bEnableSpdInfoFrame = HI_FALSE;
    attr.bEnableMpegInfoFrame = HI_FALSE;
    attr.bDebugFlag = HI_FALSE;
    attr.bHDCPEnable = HI_FALSE;
    attr.b3DEnable = HI_FALSE;

    ret = HI_MPI_HDMI_SetAttr(dev, &attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_HDMI_SetAttr failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_HDMI_Start(dev);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_HDMI_Start failed with %#x", ret);
        return KSDKError;
    }
    return KSuccess;
}

int32_t VideoOutput::StartDevLayer(int32_t dev, VO_INTF_TYPE_E intf_type, VO_INTF_SYNC_E intf_sync)
{
    int32_t ret;

    VO_PUB_ATTR_S pub_attr;
    memset(&pub_attr, 0, sizeof(pub_attr));
    pub_attr.enIntfType = intf_type;
    pub_attr.enIntfSync = intf_sync;
    pub_attr.u32BgColor = 0x00000;
    pub_attr.bDoubleFrame = HI_FALSE;
    ret = HI_MPI_VO_SetPubAttr(dev, &pub_attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VO_SetPubAttr failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_VO_Enable(dev);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VO_Enable failed with %#x", ret);
        return KSDKError;
    }

    VO_VIDEO_LAYER_ATTR_S layer_attr;
    memset(&layer_attr, 0, sizeof(layer_attr));
    layer_attr.enPixFormat = RS_PIXEL_FORMAT;
    layer_attr.u32DispFrmRt = Utils::GetFrameRate(intf_sync);

    SIZE_S size = Utils::GetSize(intf_sync);
    layer_attr.stDispRect.s32X = 0;
    layer_attr.stDispRect.s32Y = 0;
    layer_attr.stDispRect.u32Width = size.u32Width;
    layer_attr.stDispRect.u32Height = size.u32Height;
    layer_attr.stImageSize.u32Width = size.u32Width;
    layer_attr.stImageSize.u32Height = size.u32Height;

    ret = HI_MPI_VO_SetVideoLayerAttr(dev, &layer_attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VO_SetVideoLayerAttr failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_VO_EnableVideoLayer(dev);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VO_EnableVideoLayer failed with %#x", ret);
        return KSDKError;
    }

    return KSuccess;
}

int VideoOutput::StartChannel(int chn, const RECT_S &rect, int level)
{
    if (!init_)
        return KUnInitialized;

    int32_t ret;

    VO_CHN_ATTR_S attr;
    memset(&attr, 0, sizeof(attr));
    attr.stRect.s32X = rect.s32X;
    attr.stRect.s32Y = rect.s32Y;
    attr.stRect.u32Width = rect.u32Width;
    attr.stRect.u32Height = rect.u32Height;
    attr.u32Priority = level;
    attr.bDeflicker = HI_FALSE;

    ret = HI_MPI_VO_SetChnAttr(params_.dev, chn, &attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VO_SetChnAttr failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_VO_EnableChn(params_.dev, chn);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VO_EnableChn failed with %#x", ret);
        return KSDKError;
    }

    return KSuccess;
}

int VideoOutput::SetChannelFrameRate(int chn, int frame_rate)
{
    if (!init_)
        return KUnInitialized;

    int ret;
    ret = HI_MPI_VO_SetChnFrameRate(params_.dev, chn, frame_rate);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VO_EnableChn failed with %#x", ret);
        return KSDKError;
    }

    return KSuccess;
}

int VideoOutput::StopChannel(int chn)
{
    if (!init_)
        return KUnInitialized;

    int ret;

    ret = HI_MPI_VO_DisableChn(params_.dev, chn);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VO_DisableChn failed with %#x", ret);
        return KSDKError;
    }
    return KSuccess;
}

int VideoOutput::StopAllChn()
{
    if (!init_)
        return KUnInitialized;

    for (int i = 0; i < VO_MAX_CHN_NUM; i++)
    {
#if 1
        HI_MPI_VO_DisableChn(params_.dev, i);
#else
        int ret = HI_MPI_VO_DisableChn(params_.dev, i);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_VO_DisableChn failed with %#x", ret);
            return KSDKError;
        }
#endif
    }

    return KSuccess;
}

void VideoOutput::Close()
{
    if (!init_)
        return;

    int ret;
    log_d("VO stop,dev:%d", params_.dev);

    StopAllChn();

    ret = HI_MPI_VO_DisableVideoLayer(params_.dev);
    if (ret != KSuccess)
        log_e("HI_MPI_VO_DisableVideoLayer failed with %#x", ret);

    ret = HI_MPI_VO_Disable(params_.dev);
    if (ret != KSuccess)
        log_e("HI_MPI_VO_Disable failed with %#x", ret);

    if (params_.intf_type & VO_INTF_HDMI)
    {
        ret = HI_MPI_HDMI_Stop(HdmiDev);
        if (ret != KSuccess)
            log_e("HI_MPI_HDMI_Stop failed with %#x", ret);
        ret = HI_MPI_HDMI_Close(HdmiDev);
        if (ret != KSuccess)
            log_e("HI_MPI_HDMI_Close failed with %#x", ret);
        ret = HI_MPI_HDMI_DeInit();
        if (ret != KSuccess)
            log_e("HI_MPI_HDMI_DeInit failed with %#x", ret);
    }

    init_ = false;
}

int VideoOutput::ClearDispBuffer(int chn)
{
    if (!init_)
        return KUnInitialized;

    int ret;
    ret = HI_MPI_VO_ClearChnBuffer(params_.dev, chn, HI_TRUE);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VO_ClearChnBuffer failed with %#x", ret);
        return KSDKError;
    }

    return KSuccess;
}
} // namespace rs