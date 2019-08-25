#pragma once

//self
#include "global.h"
#include "common/video_define.h"

namespace rs
{
class Tw6874
{
public:
    explicit Tw6874();

    virtual ~Tw6874();

    int Initialize();

    void Close();

    void UpdateVIFmt(const VideoInputFormat &fmt);

    void SetVIFmtListener(std::shared_ptr<VIFmtListener> listener);

private:
    std::mutex mux_;
    VideoInputFormat now_fmt_;
    VideoInputFormat run_fmt_;
    std::shared_ptr<VIFmtListener> listener_;
    std::atomic<bool> run_;
    std::unique_ptr<std::thread> thread_;
    bool init_;
};
} // namespace rs