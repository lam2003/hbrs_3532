#include "common/global.h"
#include "system/vm.h"

#pragma once

namespace rs
{

namespace vi
{
struct Params
{
    int dev;
    int chn;
    CaptureMode mode;
};

} // namespace vi

class VideoInput : public Module<vi::Params>
{
public:
    explicit VideoInput();

    virtual ~VideoInput();

    int Initialize(const vi::Params &params) override;

    void Close() override;

protected:
    static int StartDev(int dev, CaptureMode mode);

    static void SetMask(int dev, VI_DEV_ATTR_S &dev_attr);

    static int StartChn(int chn, CaptureMode mode);

private:
    vi::Params params_;
    bool init_;

    static const VI_DEV_ATTR_S DevAttr_7441_BT1120_720P;
    static const VI_DEV_ATTR_S DevAttr_7441_BT1120_1080P;
};
} // namespace rs