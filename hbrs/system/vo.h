#pragma once

//self
#include "common/global.h"

namespace rs
{
namespace vo
{
struct Params
{
    int32_t dev;
    VO_INTF_TYPE_E intf_type;
    VO_INTF_SYNC_E intf_sync;
};

struct Channel
{
    RECT_S rect;
    int32_t no;
    int32_t level;
};
} // namespace vo

class VideoOutput
{
public:
    explicit VideoOutput();

    virtual ~VideoOutput();

    int32_t Initialize(const vo::Params &params);

    void Close();

    int32_t StartChn(const vo::Channel &chn);

    int32_t StopChn(int32_t chn);

    int32_t StopAllChn();

protected:
    static int32_t StartHDMI(HI_HDMI_ID_E dev, VO_INTF_SYNC_E intf_sync);

    static int32_t StartDevLayer(int32_t dev, VO_INTF_TYPE_E intf_type, VO_INTF_SYNC_E intf_sync);

private:
    vo::Params params_;
    bool init_;

    static const HI_HDMI_ID_E HdmiDev;
};
} // namespace rs