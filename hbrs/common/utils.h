#pragma once

#include <chrono>
#include "common/global.h"

namespace rs
{
class Utils
{
public:
    static SIZE_S GetSize(CaptureMode mode)
    {
        SIZE_S size;

        switch (mode)
        {
        case CAPTURE_MODE_720P:
            size.u32Width = 1280;
            size.u32Height = 720;
            break;
        case CAPTURE_MODE_1080P:
            size.u32Width = 1920;
            size.u32Height = 1080;
            break;
        default:
            RS_ASSERT(0);
        }

        return size;
    }

    static HI_HDMI_VIDEO_FMT_E GetHDMIFmt(VO_INTF_SYNC_E intf_sync)
    {
        HI_HDMI_VIDEO_FMT_E fmt;
        switch (intf_sync)
        {
        case VO_OUTPUT_PAL:
            fmt = HI_HDMI_VIDEO_FMT_PAL;
            break;
        case VO_OUTPUT_1080P24:
            fmt = HI_HDMI_VIDEO_FMT_1080P_24;
            break;
        case VO_OUTPUT_1080P25:
            fmt = HI_HDMI_VIDEO_FMT_1080P_25;
            break;
        case VO_OUTPUT_1080P30:
            fmt = HI_HDMI_VIDEO_FMT_1080P_30;
            break;
        case VO_OUTPUT_1080P50:
            fmt = HI_HDMI_VIDEO_FMT_1080P_50;
            break;
        case VO_OUTPUT_1080P60:
            fmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;
        case VO_OUTPUT_720P50:
            fmt = HI_HDMI_VIDEO_FMT_720P_50;
            break;
        case VO_OUTPUT_720P60:
            fmt = HI_HDMI_VIDEO_FMT_720P_60;
            break;
        case VO_OUTPUT_800x600_60:
            fmt = HI_HDMI_VIDEO_FMT_VESA_800X600_60;
            break;
        case VO_OUTPUT_1024x768_60:
            fmt = HI_HDMI_VIDEO_FMT_VESA_1024X768_60;
            break;
        case VO_OUTPUT_1280x1024_60:
            fmt = HI_HDMI_VIDEO_FMT_VESA_1280X1024_60;
            break;
        case VO_OUTPUT_1366x768_60:
            fmt = HI_HDMI_VIDEO_FMT_VESA_1366X768_60;
            break;
        case VO_OUTPUT_1440x900_60:
            fmt = HI_HDMI_VIDEO_FMT_VESA_1440X900_60;
            break;
        case VO_OUTPUT_1280x800_60:
            fmt = HI_HDMI_VIDEO_FMT_VESA_1280X800_60;
            break;
        default:
            RS_ASSERT(0);
        }
        return fmt;
    }

    static SIZE_S GetSize(VO_INTF_SYNC_E intf_sync)
    {
        SIZE_S size;

        switch (intf_sync)
        {
        case VO_OUTPUT_PAL:
            size.u32Width = 720;
            size.u32Height = 576;
            break;
        case VO_OUTPUT_1080P24:
        case VO_OUTPUT_1080P25:
        case VO_OUTPUT_1080P30:
        case VO_OUTPUT_1080P50:
        case VO_OUTPUT_1080P60:
            size.u32Width = 1920;
            size.u32Height = 1080;
            break;
        case VO_OUTPUT_720P50:
        case VO_OUTPUT_720P60:
            size.u32Width = 1280;
            size.u32Height = 720;
            break;
        case VO_OUTPUT_800x600_60:
            size.u32Width = 800;
            size.u32Height = 600;
            break;
        case VO_OUTPUT_1024x768_60:
            size.u32Width = 1024;
            size.u32Height = 768;
            break;
        case VO_OUTPUT_1280x1024_60:
            size.u32Width = 1280;
            size.u32Height = 1024;
            break;
        case VO_OUTPUT_1366x768_60:
            size.u32Width = 1366;
            size.u32Height = 768;
            break;
        case VO_OUTPUT_1440x900_60:
            size.u32Width = 1400;
            size.u32Height = 900;
            break;
        case VO_OUTPUT_1280x800_60:
            size.u32Width = 1280;
            size.u32Height = 800;
            break;
        default:
            RS_ASSERT(0);
        }
        return size;
    }

    static int GetFrameRate(VO_INTF_SYNC_E intf_sync)
    {
        int frame_rate;

        switch (intf_sync)
        {
        case VO_OUTPUT_PAL:
        case VO_OUTPUT_1080P25:
            frame_rate = 25;
            break;
        case VO_OUTPUT_1080P24:
            frame_rate = 24;
            break;
        case VO_OUTPUT_1080P30:
            frame_rate = 30;
            break;
        case VO_OUTPUT_1080P50:
        case VO_OUTPUT_720P50:
            frame_rate = 50;
            break;
        case VO_OUTPUT_1080P60:
        case VO_OUTPUT_720P60:
        case VO_OUTPUT_800x600_60:
        case VO_OUTPUT_1024x768_60:
        case VO_OUTPUT_1280x1024_60:
        case VO_OUTPUT_1366x768_60:
        case VO_OUTPUT_1440x900_60:
        case VO_OUTPUT_1280x800_60:
            frame_rate = 60;
            break;
        default:
            RS_ASSERT(0);
        }
        return frame_rate;
    }

    static uint64_t GetSteadyMilliSeconds()
    {
        using namespace std::chrono;
        auto now = steady_clock::now();
        auto now_since_epoch = now.time_since_epoch();
        return duration_cast<milliseconds>(now_since_epoch).count();
    }
};
} // namespace rs