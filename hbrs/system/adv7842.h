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

    static Adv7842 *Instance();

    int Initialize(ADV7842_CMODE_E mode);

    void Close();

    void GetInputFormat(VideoInputFormat &fmt);

    void SetVIFmtListener(VIFmtListener *listener);

protected:
    explicit Adv7842();

private:
    std::mutex mux_;
    VideoInputFormat fmt_;
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> run_;
    VIFmtListener *listener_;
    bool init_;
};
} // namespace rs