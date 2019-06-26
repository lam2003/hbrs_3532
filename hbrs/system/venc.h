
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <memory>

#include "common/global.h"
#include "system/vm.h"

#pragma once

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
    int32_t frame_rate;
    int32_t profile;
    int32_t bitrate;
    VENC_RC_MODE_E mode;
};

} // namespace venc

class VideoEncode : public Module<venc::Params>
{
public:
    explicit VideoEncode();

    virtual ~VideoEncode();

    int32_t Initialize(const venc::Params &params) override;

    void Close() override;

    void AddVideoSink(VideoSink<VENC_STREAM_S> *video_sink);

    void RemoveAllVideoSink();

private:
    venc::Params params_;
    std::vector<VideoSink<VENC_STREAM_S> *> video_sinks_;
    std::mutex video_sinks_mux_;
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> run_;
    bool init_;

    static const int PacketBufferSize;
};
} // namespace rs