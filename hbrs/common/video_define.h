#pragma once

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

struct VideoInputFormat
{
    bool has_signal;
    int width;
    int height;
    bool interlaced;
    int frame_rate;
};
} // namespace rs



