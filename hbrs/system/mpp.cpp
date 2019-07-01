//self
#include "system/mpp.h"

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

int32_t MPPSystem::Initialize(int blk_num)
{
    if (init_)
        return KInitialized;

    int32_t ret;

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

    ConfigLogger();

    ret = ConfigVB(blk_num);
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

int32_t MPPSystem::ConfigVB(int blk_num)
{
    int32_t ret;

    uint32_t blk_size = (((RS_MAX_WIDTH + (RS_MAX_WIDTH % RS_ALIGN_WIDTH)) * (RS_MAX_HEIGHT + (RS_MAX_HEIGHT % RS_ALIGN_WIDTH))) * 3) / 2;

    VB_CONF_S conf;
    memset(&conf, 0, sizeof(conf));
    conf.u32MaxPoolCnt = 128;
    conf.astCommPool[0].u32BlkSize = blk_size;
    conf.astCommPool[0].u32BlkCnt = blk_num / 2;

    conf.astCommPool[1].u32BlkSize = blk_size;
    conf.astCommPool[1].u32BlkCnt = blk_num / 2;
    strcpy(conf.astCommPool[1].acMmzName, "ddr1");

    conf.astCommPool[2].u32BlkSize = RS_PCIV_WINDOW_SIZE / 2;
    conf.astCommPool[2].u32BlkCnt = 1;

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

void MPPSystem::ConfigLogger()
{
    setbuf(stdout, NULL);
    elog_init();
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TIME | ELOG_FMT_DIR |
                                     ELOG_FMT_LINE | ELOG_FMT_FUNC);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    elog_set_text_color_enabled(true);
    elog_start();
}

int32_t MPPSystem::ConfigMem()
{
    int ret;

    MPP_CHN_S chn_vi;
    MPP_CHN_S chn_vo;
    MPP_CHN_S chn_vpss;
    MPP_CHN_S chn_grp;
    MPP_CHN_S chn_venc;
    MPP_CHN_S chn_rgn;
    MPP_CHN_S chn_dec;

    for (int i = 0; i < 4; i++)
    {
        chn_vi.enModId = HI_ID_VIU;
        chn_vi.s32DevId = 0;
        chn_vi.s32ChnId = i * 4;

        const char *mmz_name = (i % 2 == 0 ? nullptr : "ddr1");
        ret = HI_MPI_SYS_SetMemConf(&chn_vi, mmz_name);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }
    }

    for (int i = 0; i < VDEC_MAX_CHN_NUM; i++)
    {
        chn_dec.enModId = HI_ID_VDEC;
        chn_dec.s32DevId = 0;
        chn_dec.s32ChnId = i;

        const char *mmz_name = (i % 2 == 0 ? nullptr : "ddr1");
        ret = HI_MPI_SYS_SetMemConf(&chn_dec, mmz_name);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }
    }

    for (int i = 0; i < VENC_MAX_GRP_NUM; i++)
    {
        chn_venc.enModId = HI_ID_VENC;
        chn_venc.s32DevId = 0;
        chn_venc.s32ChnId = i;

        chn_grp.enModId = HI_ID_GROUP;
        chn_grp.s32DevId = i;
        chn_grp.s32ChnId = 0;

        const char *mmz_name = (i % 2 == 0 ? nullptr : "ddr1");
        ret = HI_MPI_SYS_SetMemConf(&chn_venc, mmz_name);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }
        ret = HI_MPI_SYS_SetMemConf(&chn_grp, mmz_name);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }
    }

    for (int i = 0; i < VPSS_MAX_GRP_NUM; i++)
    {
        chn_vpss.enModId = HI_ID_VPSS;
        chn_vpss.s32DevId = i;
        chn_vpss.s32ChnId = 0;
        const char *mmz_name = (i % 2 == 0 ? nullptr : "ddr1");
        ret = HI_MPI_SYS_SetMemConf(&chn_vpss, mmz_name);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }
    }

    for (int i = 0; i < VO_MAX_DEV_NUM; i++)
    {
        chn_vo.enModId = HI_ID_VOU;
        chn_vo.s32DevId = i;
        chn_vo.s32ChnId = 0;
        const char *mmz_name = (i % 2 == 0 ? nullptr : "ddr1");
        ret = HI_MPI_SYS_SetMemConf(&chn_vo, mmz_name);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }
    }

    {
        chn_rgn.enModId = HI_ID_RGN;
        chn_rgn.s32DevId = 0;
        chn_rgn.s32ChnId = 0;
        const char *mmz_name = nullptr;
        ret = HI_MPI_SYS_SetMemConf(&chn_rgn, mmz_name);
        if (ret != KSuccess)
        {
            log_e("HI_MPI_SYS_SetMemConf failed with %#x", ret);
            return KSDKError;
        }
    }

    return KSuccess;
}

} // namespace rs