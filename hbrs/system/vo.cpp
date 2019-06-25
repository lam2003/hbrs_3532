#include "system/vo.h"
#include "common/utils.h"

namespace rs
{

using namespace vo;

const HI_HDMI_ID_E VideoOutput::HdmiDev = HI_HDMI_ID_0;

VideoOutput::VideoOutput() : init_(false)
{
}

VideoOutput::~VideoOutput()
{
}

int VideoOutput::Initialize(const Params &params)
{
    if (init_)
        return KInitialized;

    int ret;

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

int VideoOutput::StartHDMI(int dev, int intf_sync)
{
    int ret;

    HI_HDMI_INIT_PARA_S param;
    HI_HDMI_ATTR_S attr;
    HI_HDMI_VIDEO_FMT_E fmt;

    param.enForceMode = HI_HDMI_FORCE_HDMI;
    param.pCallBackArgs = NULL;
    param.pfnHdmiEventCallback = NULL;

    ret = HI_MPI_HDMI_Init(&param);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_HDMI_Init failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_HDMI_Open(static_cast<HI_HDMI_ID_E>(dev));
    if (ret != KSuccess)
    {
        log_e("HI_MPI_HDMI_Open failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_HDMI_GetAttr(static_cast<HI_HDMI_ID_E>(dev), &attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_HDMI_GetAttr failed with %#x", ret);
        return KSDKError;
    }

    fmt = Utils::GetHDMIFmt(static_cast<VO_INTF_SYNC_E>(intf_sync));

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

    ret = HI_MPI_HDMI_SetAttr(static_cast<HI_HDMI_ID_E>(dev), &attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_HDMI_SetAttr failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_HDMI_Start(static_cast<HI_HDMI_ID_E>(dev));
    if (ret != KSuccess)
    {
        log_e("HI_MPI_HDMI_Start failed with %#x", ret);
        return KSDKError;
    }
    return KSuccess;
}

int VideoOutput::StartDevLayer(int dev, int intf_type, int intf_sync)
{
    log_d("vo[%d] intf_type:%d intf_sync:%d", dev, intf_type, intf_sync);

    int ret;

    VO_PUB_ATTR_S pub_attr;
    memset(&pub_attr, 0, sizeof(pub_attr));
    pub_attr.enIntfType = static_cast<VO_INTF_TYPE_E>(intf_type);
    pub_attr.enIntfSync = static_cast<VO_INTF_SYNC_E>(intf_sync);
    pub_attr.u32BgColor = 0x0ffff;
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
    layer_attr.u32DispFrmRt = Utils::GetFrameRate(static_cast<VO_INTF_SYNC_E>(intf_sync));
    SIZE_S size = Utils::GetSize(static_cast<VO_INTF_SYNC_E>(intf_sync));
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

int VideoOutput::StartChn(const Channel &chn)
{
    if (!init_)
        return KUnInitialized;

    int ret;

    VO_CHN_ATTR_S attr;
    memset(&attr, 0, sizeof(attr));
    attr.stRect.s32X = chn.rect.s32X;
    attr.stRect.s32Y = chn.rect.s32Y;
    attr.stRect.u32Width = chn.rect.u32Width;
    attr.stRect.u32Height = chn.rect.u32Height;
    attr.u32Priority = chn.level;
    attr.bDeflicker = HI_FALSE;

    

    ret = HI_MPI_VO_SetChnAttr(params_.dev, chn.no, &attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VO_SetChnAttr failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_VO_EnableChn(params_.dev, chn.no);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VO_EnableChn failed with %#x", ret);
        return KSDKError;
    }

    return KSuccess;
}

void VideoOutput::Close()
{
}
} // namespace rs