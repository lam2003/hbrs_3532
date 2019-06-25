#pragma once

// #include <vector>

#include "common/global.h"
#include "system/vm.h"

namespace rs
{
namespace vo
{
struct Params
{
    int dev;
    int intf_type;
    int intf_sync;
};

struct Channel
{
    RECT_S rect;
    int no;
    int level;
};
} // namespace vo

class VideoOutput : public Module<vo::Params>
{
public:
    explicit VideoOutput();

    virtual ~VideoOutput();

    int Initialize(const vo::Params &params) override;

    void Close() override;

    int StartChn(const vo::Channel &chn);

    int StopChn(int chn);

protected:
    static int StartHDMI(int dev, int intf_sync);

    static int StartDevLayer(int dev, int intf_type, int intf_sync);

private:
    vo::Params params_;
    bool init_;

    static const HI_HDMI_ID_E HdmiDev;
};
} // namespace rs