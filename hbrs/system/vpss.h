#pragma once

#include "common/global.h"
#include "system/vm.h"

namespace rs
{

namespace vpss
{
struct Params
{
    int32_t grp;
    CaptureMode mode;
};

} // namespace vpss

class VideoProcess : public Module<vpss::Params>
{

public:
    explicit VideoProcess();

    virtual ~VideoProcess();

    int32_t Initialize(const vpss::Params &params) override;

    void Close() override;

    int32_t SetChnSize(int32_t chn,const SIZE_S &size);

protected:
    static int32_t SetChnMode(int32_t grp, int32_t chn, const SIZE_S &size);

private:
    vpss::Params params_;
    bool init_;
};
} // namespace rs