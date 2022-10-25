#pragma once

//self
#include "global.h"
#include "common/video_define.h"

namespace rs
{

namespace venc
{
struct Params
{
    int32_t grp;
    int32_t chn;
    int32_t width;
    int32_t height;
    int32_t src_frame_rate;
    int32_t dst_frame_rate;
    int32_t profile;
    int32_t bitrate;
    VENC_RC_MODE_E mode;
};

} // namespace venc

class VideoEncode
{
public:
    explicit VideoEncode();

    virtual ~VideoEncode();

    int32_t Initialize(const venc::Params &params);

    void Close();

private:
    venc::Params params_;
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> run_;
    bool init_;
};
} // namespace rs