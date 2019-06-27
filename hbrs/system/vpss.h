#pragma once
//self
#include "common/global.h"

namespace rs
{

namespace vpss
{
struct Params
{
    int32_t grp;
};
} // namespace vpss

class VideoProcess
{

public:
    explicit VideoProcess();

    virtual ~VideoProcess();

    int32_t Initialize(const vpss::Params &params);

    void Close();

    int32_t SetChnSize(int32_t chn, const SIZE_S &size, HI_VPSS_CHN_MODE_E mode = VPSS_CHN_MODE_USER);

protected:
    static int32_t SetChnMode(int32_t grp, int32_t chn, const SIZE_S &size, HI_VPSS_CHN_MODE_E mode);

private:
    vpss::Params params_;
    bool init_;
};
} // namespace rs