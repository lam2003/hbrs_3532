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
    int32_t dev;
    int32_t intf_type;
    int32_t intf_sync;
};

struct Channel
{
    RECT_S rect;
    int32_t no;
    int32_t level;
};
} // namespace vo

class VideoOutput : public Module<vo::Params>
{
public:
    explicit VideoOutput();

    virtual ~VideoOutput();

    int32_t Initialize(const vo::Params &params) override;

    void Close() override;

    int32_t StartChn(const vo::Channel &chn);

    int32_t StopChn(int32_t chn);

protected:
    static int32_t StartHDMI(int32_t dev, int32_t intf_sync);

    static int32_t StartDevLayer(int32_t dev, int32_t intf_type, int32_t intf_sync);

private:
    vo::Params params_;
    bool init_;

    static const HI_HDMI_ID_E HdmiDev;
};
} // namespace rs