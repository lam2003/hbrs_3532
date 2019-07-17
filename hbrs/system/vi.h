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
    std::atomic<bool> run_;
    std::unique_ptr<std::thread> thread_;
    bool init_;

    static const VI_DEV_ATTR_S DevAttr_7441_BT1120_1080P;
};

class VideoOutput;

class VIHelper : public VIFmtListener
{
public:
    explicit VIHelper(int dev, int chn, VideoOutput *vo);

    virtual ~VIHelper();

    void OnChange(const VideoInputFormat &fmt) override;

    void OnStop() override;

private:
    VideoInput vi_;
    int dev_;
    int chn_;
    VideoOutput *vo_;
};
} // namespace rs