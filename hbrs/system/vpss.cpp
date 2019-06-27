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
    Close();
}

int32_t VideoProcess::Initialize(const Params &params)
{
    if (init_)
        return KInitialized;

    int32_t ret;

    params_ = params;

    VPSS_GRP_ATTR_S attr;
    memset(&attr, 0, sizeof(attr));

    attr.u32MaxH = RS_MAX_HEIGHT;
    attr.u32MaxW = RS_MAX_WIDTH;
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

    for (int32_t i = 0; i < VPSS_MAX_CHN_NUM; i++)
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

int32_t VideoProcess::SetChnMode(int32_t grp, int32_t chn, const SIZE_S &size, HI_VPSS_CHN_MODE_E mode)
{
    int32_t ret;
    VPSS_CHN_MODE_S chn_mode;
    memset(&chn_mode, 0, sizeof(chn_mode));
    chn_mode.enChnMode = mode;
    chn_mode.u32Width = size.u32Width;
    chn_mode.u32Height = size.u32Height;
    chn_mode.bDouble = HI_FALSE;
    chn_mode.enPixelFormat = RS_PIXEL_FORMAT;
    ret = HI_MPI_VPSS_SetChnMode(grp, chn, &chn_mode);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VPSS_SetChnMode failed with %#x", ret);
        return KSDKError;
    }
    return KSuccess;
}

int32_t VideoProcess::SetChnSize(int32_t chn, const SIZE_S &size, HI_VPSS_CHN_MODE_E mode)
{
    if (!init_)
        return KUnInitialized;

    int32_t ret;
    ret = SetChnMode(params_.grp, chn, size, mode);
    if (ret != KSuccess)
        return ret;

    return KSuccess;
}

void VideoProcess::Close()
{
    if (!init_)
        return;
    int ret;

    ret = HI_MPI_VPSS_StopGrp(params_.grp);
    if (ret != KSuccess)
        log_e("HI_MPI_VPSS_StopGrp failed with %#x", ret);

    for (int i = 0; i < VPSS_MAX_CHN_NUM; i++)
    {
        ret = HI_MPI_VPSS_DisableChn(params_.grp, i);
        if (ret != KSuccess)
            log_e("HI_MPI_VPSS_DisableChn failed with %#x", ret);
    }

    ret = HI_MPI_VPSS_DestroyGrp(params_.grp);
    if (ret != KSuccess)
        log_e("HI_MPI_VPSS_DestroyGrp failed with %#x", ret);

    init_ = false;
}

} // namespace rs