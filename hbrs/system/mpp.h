#pragma once

#include "common/global.h"
#include "system/vi.h"
#include "system/vpss.h"
#include "system/vo.h"

namespace rs
{

namespace mpp
{
struct Params
{
    CaptureMode mode;
    int block_num;
};
} // namespace mpp

class MPPSystem : public Module<mpp::Params>
{
public:
    static MPPSystem *Instance();

    virtual ~MPPSystem();

    int Initialize(const mpp::Params &params) override;

    void Close() override;

    static int Bind(MPP_CHN_S *src_chn, MPP_CHN_S *dst_chn)
    {
        int ret;
        ret = HI_MPI_SYS_Bind(src_chn, dst_chn);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_SYS_Bind failed with %#x", ret);
            return KSDKError;
        }
        return KSuccess;
    }

    static int UnBind(MPP_CHN_S *src_chn, MPP_CHN_S *dst_chn)
    {
        int ret;
        ret = HI_MPI_SYS_UnBind(src_chn, dst_chn);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_SYS_UnBind failed with %#x", ret);
            return KSDKError;
        }
        return KSuccess;
    }

    template <MOD_ID_E SRC, MOD_ID_E DST>
    static int Bind(int sdev, int schn, int ddev, int dchn)
    {
        int ret;
        MPP_CHN_S src_chn, dst_chn;
        src_chn.s32DevId = sdev;
        src_chn.s32ChnId = schn;
        src_chn.enModId = SRC;
        dst_chn.s32DevId = ddev;
        dst_chn.s32ChnId = dchn;
        dst_chn.enModId = DST;
        ret = Bind(&src_chn, &dst_chn);
        if (ret != KSuccess)
            return ret;
        return KSuccess;
    }

    template <MOD_ID_E SRC, MOD_ID_E DST>
    static int UnBind(int sdev, int schn, int ddev, int dchn)
    {
        int ret;
        MPP_CHN_S src_chn, dst_chn;
        src_chn.s32DevId = sdev;
        src_chn.s32ChnId = schn;
        src_chn.enModId = SRC;
        dst_chn.s32DevId = ddev;
        dst_chn.s32ChnId = dchn;
        dst_chn.enModId = DST;
        ret = UnBind(&src_chn, &dst_chn);
        if (ret != KSuccess)
            return ret;
        return KSuccess;
    }

protected:
    static int ConfigVB(CaptureMode mode, int block_num);

    static int ConfigSys(int align_width);

    static void ConfigLogger();

    static int ConfigMem();

private:
    explicit MPPSystem();

private:
    mpp::Params params_;
    bool init_;
};
} // namespace rs