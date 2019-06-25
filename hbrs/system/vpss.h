#pragma once

#include "common/global.h"
#include "system/vm.h"

namespace rs
{

namespace vpss
{
struct Params
{
    int grp;
    CaptureMode mode;
};

} // namespace vpss

class VideoProcess : public Module<vpss::Params>
{

public:
    explicit VideoProcess();

    virtual ~VideoProcess();

    int Initialize(const vpss::Params &params) override;

    void Close() override;

    int SetChnSize(int chn,const SIZE_S &size);

protected:
    static int SetChnMode(int grp, int chn, const SIZE_S &size);

private:
    vpss::Params params_;
    bool init_;
};
} // namespace rs