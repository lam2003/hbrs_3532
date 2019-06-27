#pragma once

//self
#include "common/global.h"

namespace rs
{

class MPPSystem
{
public:
    static MPPSystem *Instance();

    virtual ~MPPSystem();

    int32_t Initialize(int blk_num);

    void Close();

    template <MOD_ID_E SRC, MOD_ID_E DST>
    static int32_t Bind(int32_t sdev, int32_t schn, int32_t ddev, int32_t dchn)
    {
        int32_t ret;
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
    static int32_t UnBind(int32_t sdev, int32_t schn, int32_t ddev, int32_t dchn)
    {
        int32_t ret;
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
    static int32_t Bind(MPP_CHN_S *src_chn, MPP_CHN_S *dst_chn)
    {
        int32_t ret;
        ret = HI_MPI_SYS_Bind(src_chn, dst_chn);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_SYS_Bind failed with %#x", ret);
            return KSDKError;
        }
        return KSuccess;
    }

    static int32_t UnBind(MPP_CHN_S *src_chn, MPP_CHN_S *dst_chn)
    {
        int32_t ret;
        ret = HI_MPI_SYS_UnBind(src_chn, dst_chn);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_SYS_UnBind failed with %#x", ret);
            return KSDKError;
        }
        return KSuccess;
    }

    static int32_t ConfigVB(int blk_num);

    static int32_t ConfigSys();

    static void ConfigLogger();

    static int32_t ConfigMem();

private:
    explicit MPPSystem();

private:
    bool init_;
};
} // namespace rs