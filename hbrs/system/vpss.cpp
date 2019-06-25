#include "system/vpss.h"
#include "common/utils.h"

namespace rs
{

using namespace vpss;

VideoProcess::VideoProcess() : init_(false)
{
}

VideoProcess::~VideoProcess()
{
}

int VideoProcess::Initialize(const Params &params)
{
    if (init_)
        return KInitialized;

    int ret;

    params_ = params;

    VPSS_GRP_ATTR_S attr;
    SIZE_S size = Utils::GetSize(params_.mode);
    attr.u32MaxH = size.u32Height;
    attr.u32MaxW = size.u32Width;
    attr.bDrEn = HI_FALSE;
    attr.bDbEn = HI_FALSE;
    attr.bIeEn = HI_FALSE;
    attr.bNrEn = HI_FALSE;
    attr.bHistEn = HI_FALSE;
    attr.enDieMode = VPSS_DIE_MODE_AUTO;
    attr.enPixFmt = RS_PIXEL_FORMAT;

    ret = HI_MPI_VPSS_CreateGrp(params_.grp, &attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VPSS_CreateGrp failed with %#x", ret);
        return KSDKError;
    }

    for (int i = 0; i < VPSS_MAX_CHN_NUM; i++)
    {
        ret = HI_MPI_VPSS_EnableChn(params_.grp, i);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_VPSS_EnableChn failed with %#x", ret);
            return KSDKError;
        }
    }

    ret = HI_MPI_VPSS_StartGrp(params_.grp);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VPSS_StartGrp failed with %#x", ret);
        return KSDKError;
    }

    init_ = true;

    return KSuccess;
}

int VideoProcess::SetChnMode(int grp, int chn, const SIZE_S &size)
{
    int ret;
    VPSS_CHN_MODE_S mode;
    memset(&mode, 0, sizeof(mode));
    mode.enChnMode = VPSS_CHN_MODE_USER;
    mode.u32Width = size.u32Width;
    mode.u32Height = size.u32Height;
    mode.bDouble = HI_FALSE;
    mode.enPixelFormat = RS_PIXEL_FORMAT;
    ret = HI_MPI_VPSS_SetChnMode(grp, chn, &mode);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VPSS_SetChnMode failed with %#x", ret);
        return KSDKError;
    }
    return KSuccess;
}

int VideoProcess::SetChnSize(int chn, const SIZE_S &size)
{
    if (!init_)
        return KUnInitialized;

    int ret;
    ret = SetChnMode(params_.grp, chn, size);
    if (ret != KSuccess)
        return ret;

    return KSuccess;
}

void VideoProcess::Close()
{
}

}; // namespace rs