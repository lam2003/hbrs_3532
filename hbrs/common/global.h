#pragma once
//hisi sdk
#include <hifb.h>
#include <hi_comm_aenc.h>
#include <hi_comm_aio.h>
#include <hi_comm_hdmi.h>
#include <hi_comm_pciv.h>
#include <hi_comm_vb.h>
#include <hi_comm_vdec.h>
#include <hi_comm_venc.h>
#include <hi_comm_vi.h>
#include <hi_comm_vo.h>
#include <hi_comm_vpss.h>
#include <hi_common.h>
#include <hi_debug.h>
#include <hi_mcc_usrdev.h>
#include <mpi_aenc.h>
#include <mpi_ai.h>
#include <mpi_ao.h>
#include <mpi_hdmi.h>
#include <mpi_pciv.h>
#include <mpi_sys.h>
#include <mpi_vb.h>
#include <mpi_vdec.h>
#include <mpi_venc.h>
#include <mpi_vi.h>
#include <mpi_vo.h>
#include <mpi_vpss.h>
//stdlib
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//system
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
//logger
#include <elog.h>
//self
#include "common/video_define.h"
#include "common/err_code.h"

#define RS_ALIGN_WIDTH 64                               //图像对齐大小
#define RS_PIXEL_FORMAT PIXEL_FORMAT_YUV_SEMIPLANAR_420 //图像像素格式
#define RS_PCIV_WINDOW_SIZE 7340032                     //PCIV窗口大小
#define RS_PCIV_MASTER_ID 0                             //PCIV主片地址
#define RS_MAX_WIDTH 1920                               //最大支持的视频宽度
#define RS_MAX_HEIGHT 1080                              //最大支持的视频长度
#define RS_PCIV_CMD_PORT 0                              //PCIV命令端口
#define RS_PCIV_TRANS_READ_PORT 1                       //PCIV传输读端口
#define RS_PCIV_TRANS_WRITE_PORT 2                      //PCIV传输写端口
#define RS_MEM_BLK_NUM 20                               //系统VB内存块分配数量

#define RS_ASSERT(cond)     \
    while (!(cond))         \
    {                       \
        log_e("%s", #cond); \
        exit(1);            \
    }
