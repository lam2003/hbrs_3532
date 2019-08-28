//self
#include "system/vi.h"
#include "system/vo.h"
#include "common/err_code.h"

namespace rs
{

using namespace vi;

const VI_DEV_ATTR_S VideoInput::DevAttr_7441_BT1120_1080P = {
    VI_MODE_BT1120_STANDARD,
    VI_WORK_MODE_1Multiplex,
    {0xFF000000, 0xFF0000},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_INPUT_DATA_UVUV,
    {VI_VSYNC_PULSE,
     VI_VSYNC_NEG_HIGH,
     VI_HSYNC_VALID_SINGNAL,
     VI_HSYNC_NEG_HIGH,
     VI_VSYNC_NORM_PULSE,
     VI_VSYNC_VALID_NEG_HIGH,
     {0, 1920, 0, 0, 1080, 0, 0, 0, 0}}};

VideoInput::~VideoInput()
{
    Close();
}

VideoInput::VideoInput() : run_(false),
                           thread_(nullptr),
                           init_(false)
{
}

void VideoInput::SetMask(int32_t dev, VI_DEV_ATTR_S &dev_attr)
{
    switch (dev)
    {
    case 0:
        dev_attr.au32CompMask[0] = 0xFF000000;
        dev_attr.au32CompMask[1] = 0x00FF0000;
        break;
    case 2:
        dev_attr.au32CompMask[0] = 0x0000FF00;
        dev_attr.au32CompMask[1] = 0x000000FF;
        break;
    case 4:
        dev_attr.au32CompMask[0] = 0xFF000000;
        dev_attr.au32CompMask[1] = 0x00FF0000;
        break;
    case 6:
        dev_attr.au32CompMask[0] = 0x0000FF00;
        dev_attr.au32CompMask[1] = 0x000000FF;
        break;
    default:
        RS_ASSERT(0);
    }
}

int32_t VideoInput::StartDev(int32_t dev, int width, int height, bool interlaced)
{
    int32_t ret;
    VI_DEV_ATTR_S dev_attr;

    memset(&dev_attr, 0, sizeof(dev_attr));
    memcpy(&dev_attr, &DevAttr_7441_BT1120_1080P, sizeof(dev_attr));
    dev_attr.enScanMode = (interlaced ? VI_SCAN_INTERLACED : VI_SCAN_PROGRESSIVE);
    dev_attr.stSynCfg.stTimingBlank.u32HsyncAct = width;
    dev_attr.stSynCfg.stTimingBlank.u32VsyncVact = height;

    SetMask(dev, dev_attr);

    ret = HI_MPI_VI_SetDevAttr(dev, &dev_attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VI_SetDevAttr failed with :%#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_VI_EnableDev(dev);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VI_EnableDev failed with %#x", ret);
        return KSDKError;
    }

    return KSuccess;
}

int32_t VideoInput::StartChn(int32_t chn, int width, int height)
{
    int32_t ret;

    RECT_S rect;
    rect.s32X = 0;
    rect.s32Y = 0;
    rect.u32Width = width;
    rect.u32Height = height;

    VI_CHN_ATTR_S attr;
    memset(&attr, 0, sizeof(attr));
    attr.enCapSel = VI_CAPSEL_BOTH;
    attr.stDestSize.u32Width = width;
    attr.stDestSize.u32Height = height;
    attr.enPixFormat = RS_PIXEL_FORMAT;
    attr.bMirror = HI_FALSE;
    attr.bFlip = HI_FALSE;
    attr.bChromaResample = HI_FALSE;
    attr.s32SrcFrameRate = -1;
    attr.s32FrameRate = -1;

    memcpy(&attr.stCapRect, &rect, sizeof(rect));

    ret = HI_MPI_VI_SetChnAttr(chn, &attr);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VI_SetChnAttr failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_VI_EnableChn(chn);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VI_EnableChn failed with %#x", ret);
        return KSDKError;
    }

    return KSuccess;
}

int32_t VideoInput::Initialize(const Params &params)
{
    if (init_)
        return KInitialized;

    log_d("VI start,dev:%d,chn:%d,width:%d,height:%d,interlaced:%d",
          params.dev,
          params.chn,
          params.width,
          params.height,
          params.interlaced);
    params_ = params;

    run_ = true;
    thread_ = std::unique_ptr<std::thread>(new std::thread([this]() {
        int ret;
    again:
        ret = StartDev(params_.dev, params_.width, params_.height, params_.interlaced);
        if (ret != KSuccess)
            return;

        ret = StartChn(params_.chn, params_.width, params_.height);
        if (ret != KSuccess)
            return;

        bool first = true;
        uint32_t int_cnt = 0;
        VI_CHN_STAT_S stat;
        while (run_)
        {

            ret = HI_MPI_VI_Query(params_.chn, &stat);
            if (ret != KSuccess)
            {
                log_e("HI_MPI_VI_Query failed with %#x", ret);
                return;
            }

            int frame_rate_delta = abs(stat.u32FrmRate - 25);
            if (first)
            {
                first = false;
            }
            else if (int_cnt == stat.u32IntCnt || (stat.u32FrmRate != 0 && frame_rate_delta > 5))
            {
                ret = HI_MPI_VI_DisableChn(params_.chn);
                if (ret != KSuccess)
                {
                    log_e("HI_MPI_VI_DisableChn failed with %#x", ret);
                    return;
                }
                ret = HI_MPI_VI_DisableDev(params_.dev);
                if (ret != KSuccess)
                {
                    log_e("HI_MPI_VI_DisableDev failed with %#x", ret);
                    return;
                }
                goto again;
            }

            int_cnt = stat.u32IntCnt;
            int wait_time = 10;

            while (run_ && wait_time--)
                usleep(500000); //500ms
        }

        ret = HI_MPI_VI_DisableChn(params_.chn);
        if (ret != KSuccess)
            log_e("HI_MPI_VI_DisableChn failed with %#x", ret);

        ret = HI_MPI_VI_DisableDev(params_.dev);
        if (ret != KSuccess)
            log_e("HI_MPI_VI_DisableDev failed with %#x", ret);
    }));

    init_ = true;
    return KSuccess;
}

void VideoInput::Close()
{
    if (!init_)
        return;

    log_d("VI stop,dev:%d,chn:%d", params_.dev, params_.chn);
    run_ = false;
    thread_->join();
    thread_.reset();
    thread_ = nullptr;

    init_ = false;
}

void VIHelper::OnChange(const VideoInputFormat &fmt)
{
    log_d("VI signal change,dev:%d,chn:%d,has_signal:%d,width:%d,height:%d,interlaced:%d",
          dev_,
          chn_,
          fmt.has_signal,
          fmt.width,
          fmt.height,
          fmt.interlaced);

    vi_.Close();
    if (fmt.has_signal)
    {
        vi_.Initialize( {dev_, chn_, fmt.width, fmt.height, fmt.interlaced});
        return;
    }

    std::unique_lock<std::mutex> lock(mux_);
    if (vo_ != nullptr)
        vo_->ClearDispBuffer(0);
}

VIHelper::VIHelper(int dev, int chn) : dev_(dev),
                                       chn_(chn),
                                       vo_(nullptr)
{
}

VIHelper::~VIHelper()
{
    vi_.Close();
    vo_.reset();
    vo_ = nullptr;
}

void VIHelper::SetVideoOutput(std::shared_ptr<VideoOutput> vo)
{
    std::unique_lock<std::mutex> lock(mux_);
    vo_ = vo;
}

void VIHelper::Start(int width, int height, bool interlaced)
{
    Params params = {dev_, chn_, width, height, interlaced};
    vi_.Initialize(params);
}

void VIHelper::Stop()
{
    vi_.Close();
}

} // namespace rs