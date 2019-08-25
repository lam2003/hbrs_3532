//self
#include "system/mpp.h"
#include "common/utils.h"

namespace rs
{

MPPSystem::MPPSystem() : init_(false) {}

MPPSystem::~MPPSystem()
{
    Close();
}

void MPPSystem::Close()
{
    if (!init_)
        return;

    int ret;
    log_d("MPP stop");
    ret = HI_MPI_SYS_Exit();
    if (ret != KSuccess)
        log_e("HI_MPI_SYS_Exit failed with %#x", ret);

    ret = HI_MPI_VB_Exit();
    if (ret != KSuccess)
        log_e("HI_MPI_VB_Exit failed with %#x", ret);

    init_ = false;
}

MPPSystem *MPPSystem::Instance()
{
    static MPPSystem *instance = new MPPSystem;
    return instance;
}

int32_t MPPSystem::Initialize()
{
    if (init_)
        return KInitialized;

    int32_t ret;
    log_d("MPP start");
    ret = HI_MPI_SYS_Exit();
    if (ret != KSuccess)
    {
        log_e("HI_MPI_SYS_Exit failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_VB_Exit();
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VB_Exit failed with %#x", ret);
        return KSDKError;
    }

    ret = ConfigVB();
    if (ret != KSuccess)
        return ret;

    ret = ConfigSys();
    if (ret != KSuccess)
        return ret;

    ret = ConfigMem();
    if (ret != KSuccess)
        return ret;

    init_ = true;

    return KSuccess;
}

int32_t MPPSystem::ConfigSys()
{
    int32_t ret;

    MPP_SYS_CONF_S conf;
    memset(&conf, 0, sizeof(MPP_SYS_CONF_S));
    conf.u32AlignWidth = RS_ALIGN_WIDTH;
    ret = HI_MPI_SYS_SetConf(&conf);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_SYS_SetConf failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_SYS_Init();
    if (ret != KSuccess)
    {
        log_e("HI_MPI_SYS_Init failed with %#x", ret);
        return KSDKError;
    }

    return KSuccess;
}

int MPPSystem::ConfigMem()
{
    int ret;

    MPP_CHN_S chn_vi;
    MPP_CHN_S chn_vo;
    MPP_CHN_S chn_vpss;
    MPP_CHN_S grp_chn;
    MPP_CHN_S venc_chn;
    MPP_CHN_S chn_rgn;
    MPP_CHN_S vdec_chn;

    /*VI,VDEC最大通道数为32*/
    for (int i = 0; i < 32; i++)
    {
        chn_vi.enModId = HI_ID_VIU;
        chn_vi.s32DevId = 0;
        chn_vi.s32ChnId = i;

        vdec_chn.enModId = HI_ID_VDEC;
        vdec_chn.s32DevId = 0;
        vdec_chn.s32ChnId = i;

        const char *mmz_name = (i % 2 == 0 ? nullptr : "ddr1");

        /*vi*/
        ret = HI_MPI_SYS_SetMemConf(&chn_vi, mmz_name);
        if (ret)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }

        /*vdec*/
        ret = HI_MPI_SYS_SetMemConf(&vdec_chn, mmz_name);
        if (ret)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }
    }

    /*vpss,grp,venc最大通道为64*/
    for (int i = 0; i < 64; i++)
    {
        chn_vpss.enModId = HI_ID_VPSS;
        chn_vpss.s32DevId = i;
        chn_vpss.s32ChnId = 0;

        grp_chn.enModId = HI_ID_GROUP;
        grp_chn.s32DevId = i;
        grp_chn.s32ChnId = 0;

        venc_chn.enModId = HI_ID_VENC;
        venc_chn.s32DevId = 0;
        venc_chn.s32ChnId = i;

        const char *mmz_name = (i % 2 == 0 ? nullptr : "ddr1");

        /*vpss*/
        ret = HI_MPI_SYS_SetMemConf(&chn_vpss, mmz_name);
        if (ret)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }

        /*grp*/
        ret = HI_MPI_SYS_SetMemConf(&grp_chn, mmz_name);
        if (ret)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }

        /*venc*/
        ret = HI_MPI_SYS_SetMemConf(&venc_chn, mmz_name);
        if (ret)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }
    }

    /*配置RGN内存*/
    chn_rgn.enModId = HI_ID_RGN;
    chn_rgn.s32DevId = 0;
    chn_rgn.s32ChnId = 0;
    ret = HI_MPI_SYS_SetMemConf(&chn_rgn, "ddr1");
    if (ret)
    {
        log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
        return KSDKError;
    }

    /*配置VO内存*/
    chn_vo.enModId = HI_ID_VOU;
    chn_vo.s32DevId = 0;
    chn_vo.s32ChnId = 0;
    ret = HI_MPI_SYS_SetMemConf(&chn_vo, "ddr1");
    if (ret)
    {
        log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
        return KSDKError;
    }

    chn_vo.enModId = HI_ID_VOU;
    chn_vo.s32DevId = 10;
    chn_vo.s32ChnId = 0;
    ret = HI_MPI_SYS_SetMemConf(&chn_vo, "ddr1");
    if (ret)
    {
        log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
        return KSDKError;
    }
    chn_vo.enModId = HI_ID_VOU;
    chn_vo.s32DevId = 11;
    chn_vo.s32ChnId = 0;
    ret = HI_MPI_SYS_SetMemConf(&chn_vo, "ddr1");
    if (ret)
    {
        log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
        return KSDKError;
    }

    chn_vo.enModId = HI_ID_VOU;
    chn_vo.s32DevId = 12;
    chn_vo.s32ChnId = 0;
    ret = HI_MPI_SYS_SetMemConf(&chn_vo, "ddr1");
    if (ret)
    {
        log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
        return KSDKError;
    }

    return KSuccess;
}

int32_t MPPSystem::ConfigVB()
{
    int32_t ret;

    uint32_t blk_size = Utils::Align(RS_MAX_WIDTH) * Utils::Align(RS_MAX_HEIGHT) * 1.5;

    VB_CONF_S conf;
    memset(&conf, 0, sizeof(conf));
    conf.u32MaxPoolCnt = 128;

    //ddr0 video buffer
    conf.astCommPool[0].u32BlkSize = blk_size;
    conf.astCommPool[0].u32BlkCnt = 30;
    memset(conf.astCommPool[0].acMmzName, 0, sizeof(conf.astCommPool[1].acMmzName));
    //ddr0 hist buffer
    conf.astCommPool[1].u32BlkSize = (196 * 4);
    conf.astCommPool[1].u32BlkCnt = 30;
    memset(conf.astCommPool[1].acMmzName, 0, sizeof(conf.astCommPool[2].acMmzName));

    //ddr1 video buffer
    conf.astCommPool[2].u32BlkSize = blk_size;
    conf.astCommPool[2].u32BlkCnt = 30;
    strcpy(conf.astCommPool[2].acMmzName, "ddr1");
    //ddr1 hist buffer
    conf.astCommPool[3].u32BlkSize = (196 * 4);
    conf.astCommPool[3].u32BlkCnt = 30;
    strcpy(conf.astCommPool[3].acMmzName, "ddr1");

    //ddr0 pciv buffer
    conf.astCommPool[4].u32BlkSize = RS_PCIV_WINDOW_SIZE / 2;
    conf.astCommPool[4].u32BlkCnt = 1;
    memset(conf.astCommPool[4].acMmzName, 0, sizeof(conf.astCommPool[4].acMmzName));

    ret = HI_MPI_VB_SetConf(&conf);
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VB_SetConf failed with %#x", ret);
        return KSDKError;
    }

    ret = HI_MPI_VB_Init();
    if (ret != KSuccess)
    {
        log_e("HI_MPI_VB_Init failed with %#x", ret);
        return KSDKError;
    }

    return KSuccess;
}
} // namespace rs