#pragma once

//self
#include "global.h"
#include "common/video_define.h"

namespace rs
{

class Adv7842
{
public:
    virtual ~Adv7842();

    explicit Adv7842();

    int Initialize(ADV7842_CMODE_E mode);

    void Close();

    void GetInputFormat(VideoInputFormat &fmt);

    void SetVIFmtListener(std::shared_ptr<VIFmtListener> listener);

protected:

private:
    std::mutex mux_;
    VideoInputFormat fmt_;
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> run_;
    std::shared_ptr<VIFmtListener> listener_;
    bool init_;
};
} // namespace rs