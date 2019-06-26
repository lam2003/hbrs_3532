#pragma once

enum CaptureMode
{
    CAPTURE_MODE_720P,
    CAPTURE_MODE_1080P
};

namespace rs
{
template <typename FrameT>
class VideoSink
{
public:
    virtual ~VideoSink() {}
    virtual void OnFrame(const FrameT &) {}
    virtual void OnFrame(const FrameT &, int32_t chn) {}
};
} // namespace rs

#define VPSS_MAIN_SCREEN_GRP 10
#define VPSS_ENCODE_CHN 1
#define VPSS_PIP_CHN 2

#define VO_HD_DEV 0
#define VO_CVBS_DEV 3
#define VO_VIR_DEV 10

#define VO_VIR_MODE CAPTURE_MODE_1080P
#define PCIV_WINDOW_SIZE 7340032
#define PCIV_MASTER_ID 0