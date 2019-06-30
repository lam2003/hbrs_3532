#pragma once

//self
#include "global.h"
#include "common/video_define.h"

namespace rs
{
namespace vi
{
struct Params
{
    int32_t dev;
    int32_t chn;
    int width;
    int height;
    bool interlaced;
};

} // namespace vi

class VideoInput
{
public:
    explicit VideoInput();

    virtual ~VideoInput();

    int32_t Initialize(const vi::Params &params);

    void Close();

protected:
    static int32_t StartDev(int32_t dev, int width, int height, bool interlaced);

    static void SetMask(int32_t dev, VI_DEV_ATTR_S &dev_attr);

    static int32_t StartChn(int32_t chn, int width, int height);

private:
    vi::Params params_;
    bool init_;

    static const VI_DEV_ATTR_S DevAttr_7441_BT1120_1080P;
};

class VIHelper : public VIFmtListener
{

public:
    VIHelper(int dev, int chn) : dev_(dev), chn_(chn) {}
    ~VIHelper() = default;

    void OnChange(const VideoInputFormat &fmt) override
    {
        log_d("vi[%d][%d]has_signal:%d,fmt.width:%d,height:%d,fps:%d,interlaced:%d",
              dev_,
              chn_,
              fmt.has_signal,
              fmt.width,
              fmt.height,
              fmt.frame_rate,
              fmt.interlaced);

        vi_.Close();
        if (fmt.has_signal)
            vi_.Initialize({dev_, chn_, fmt.width, fmt.height, fmt.interlaced});
    }

    void OnStop() override
    {
        vi_.Close();
    }

private:
    int dev_;
    int chn_;
    VideoInput vi_;
};
} // namespace rs