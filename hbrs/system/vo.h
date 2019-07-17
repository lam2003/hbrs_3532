#pragma once

//self
#include "global.h"
#include "common/video_define.h"

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
} // namespace vo

class VideoOutput
{
public:
    explicit VideoOutput();

    virtual ~VideoOutput();

    int32_t Initialize(const vo::Params &params);

    void Close();

    int32_t StopAllChn();

    int StartChannel(int chn, const RECT_S &rect, int level);

    int StopChannel(int chn);

    int ClearDispBuffer(int chn);

    int SetChannelFrameRate(int chn, int frame_rate);

    static int SetSceneMode(VideoOutput &vo, int mode);

protected:
    static int32_t StartHDMI(HI_HDMI_ID_E dev, VO_INTF_SYNC_E intf_sync);

    static int32_t StartDevLayer(int32_t dev, VO_INTF_TYPE_E intf_type, VO_INTF_SYNC_E intf_sync);

private:
    vo::Params params_;
    bool init_;

    static const HI_HDMI_ID_E HdmiDev;
};
} // namespace rs