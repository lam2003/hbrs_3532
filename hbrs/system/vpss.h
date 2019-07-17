#pragma once

//self
#include "global.h"
#include "common/video_define.h"

namespace rs
{

namespace vpss
{
struct Params
{
    int32_t grp;
};
} // namespace vpss

class VideoProcess
{

public:
    explicit VideoProcess();

    virtual ~VideoProcess();

    int32_t Initialize(const vpss::Params &params);

    void Close();

    int StartUserChannel(int chn, const SIZE_S &size);

    int StopUserChannal(int chn);

    int SetFrameRateControl(int src_frame_rate, int dst_frame_rate);

private:
    vpss::Params params_;
    bool init_;
}; // namespace rs
} // namespace rs