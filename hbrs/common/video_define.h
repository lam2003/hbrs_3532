#pragma once

//stl
#include <map>

enum CaptureMode
{
    CAPTURE_MODE_720P,
    CAPTURE_MODE_1080P
};

namespace rs
{
template <typename FrameT>
class VideoSink
{
public:
    virtual ~VideoSink() {}
    virtual void OnFrame(const FrameT &) {}
    virtual void OnFrame(const FrameT &, int32_t chn) {}
};

struct InputFormat
{
    bool has_signal;
    int width;
    int height;
    bool interlaced;
    int frame_rate;
};
} // namespace rs

#define VPSS_MAIN_SCREEN_GRP 10
#define VPSS_ENCODE_CHN 1
#define VPSS_PIP_CHN 2

#define VO_HD_DEV 0
#define VO_CVBS_DEV 3
#define VO_VIR_DEV 10

#define VO_VIR_MODE CAPTURE_MODE_1080P
#define PCIV_WINDOW_SIZE 7340032
#define PCIV_MASTER_ID 0

#define RS_MAX_WIDTH 1920     //最大支持的视频宽度
#define RS_MAX_HEIGHT 1080    //最大支持的视频长度


static std::map<int, int> Slave1_VencChn2VdecChn = { //从片1 VENC通道与VDEC通道映射关系
    {0, 3}};
static std::map<int, int> Slave3_VencChn2VdecChn = { //从片3 VENC通道与VDEC通道映射关系
    {0, 0},
    {1, 1},
    {2, 2}};
