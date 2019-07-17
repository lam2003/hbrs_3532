
#include "system/mpp.h"
#include "system/adv7842.h"
#include "system/tw6874.h"
#include "system/pciv_comm.h"
#include "system/pciv_trans.h"
#include "system/vi.h"
#include "system/vo.h"
#include "system/vpss.h"
#include "system/venc.h"
#include "common/buffer.h"
#include "common/logger.h"

using namespace rs;

#define CHECK_ERROR(a)                                                                  \
	if (KSuccess != a)                                                                  \
	{                                                                                   \
		log_e("error:%s", make_error_code(static_cast<err_code>(a)).message().c_str()); \
		return a;                                                                       \
	}

static bool g_Run = true;

static void SignalHandler(int signo)
{
	if (signo == SIGINT)
	{
		g_Run = false;
	}
}

static int Recv(pciv::Context *ctx, int remote_id, int port, uint8_t *tmp_buf, int32_t buf_len, Buffer<allocator_1k> &msg_buf, bool &run, pciv::Msg &msg)
{
	int ret;
	do
	{
		ret = ctx->Recv(remote_id, port, tmp_buf, buf_len, 500000); //500ms
		if (ret > 0)
		{
			if (!msg_buf.Append(tmp_buf, ret))
			{
				log_e("append data to msg buf failed");
				return KNotEnoughBuf;
			}
		}
		else if (ret < 0)
			return ret;

	} while (run && msg_buf.Size() < sizeof(msg));

	if (msg_buf.Size() >= sizeof(msg))
	{
		msg_buf.Get(reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
		msg_buf.Consume(sizeof(msg));
	}

	return KSuccess;
}

int32_t main(int32_t argc, char **argv)
{
	int ret;

	ConfigLogger();

	signal(SIGINT, SignalHandler);

	ret = MPPSystem::Instance()->Initialize();
	CHECK_ERROR(ret);

#if CHIP_TYPE == 1
	VideoProcess vpss_pc;
	VideoOutput vo_pc;
	VideoEncode venc_pc;

	//#####################################################
	//初始化从片1上的一路vi,vi->vpss->vir_vo
	//#####################################################
	ret = vpss_pc.Initialize({0});
	CHECK_ERROR(ret);
	ret = vo_pc.Initialize({10, 0, VO_OUTPUT_1080P25});
	CHECK_ERROR(ret);
	ret = vo_pc.StartChannel(0, {0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT}, 0);
	CHECK_ERROR(ret);
	VIHelper vi_pc(4, 8, &vo_pc);
	ret = MPPSystem::Bind<HI_ID_VIU, HI_ID_VPSS>(0, 8, 0, 0);
	CHECK_ERROR(ret);
	ret = MPPSystem::Bind<HI_ID_VPSS, HI_ID_VOU>(0, 4, 10, 0);
	CHECK_ERROR(ret);
	//#####################################################
	//初始化从片1上的编码器,vir_vo->venc
	//#####################################################
	ret = venc_pc.Initialize({0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT, RS_FRAME_RATE, 0, 20000, VENC_RC_MODE_H264CBR});
	CHECK_ERROR(ret);
	ret = MPPSystem::Bind<HI_ID_VOU, HI_ID_GROUP>(10, 0, 0, 0);
	CHECK_ERROR(ret);
#else
	Tw6874 tw6874_tea_full;
	Tw6874 tw6874_stu_full;
	Tw6874 tw6874_black_board;
	VideoProcess vpss_tea_full;
	VideoProcess vpss_stu_full;
	VideoProcess vpss_black_board;
	VideoOutput vo_tea_full;
	VideoOutput vo_stu_full;
	VideoOutput vo_black_board;
	VideoEncode venc_tea_full;
	VideoEncode venc_stu_full;
	VideoEncode venc_black_board;
	//#####################################################
	//初始化从片3上的三路vi,vi->vpss->vir_vo
	//#####################################################
	ret = vpss_tea_full.Initialize({0});
	CHECK_ERROR(ret);
	ret = vpss_stu_full.Initialize({1});
	CHECK_ERROR(ret);
	ret = vpss_black_board.Initialize({2});
	CHECK_ERROR(ret);
	ret = vo_tea_full.Initialize({10, 0, VO_OUTPUT_1080P25});
	CHECK_ERROR(ret);
	ret = vo_stu_full.Initialize({11, 0, VO_OUTPUT_1080P25});
	CHECK_ERROR(ret);
	ret = vo_black_board.Initialize({12, 0, VO_OUTPUT_1080P25});
	CHECK_ERROR(ret);
	ret = vo_tea_full.StartChannel(0, {0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT}, 0);
	CHECK_ERROR(ret);
	ret = vo_stu_full.StartChannel(0, {0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT}, 0);
	CHECK_ERROR(ret);
	ret = vo_black_board.StartChannel(0, {0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT}, 0);
	CHECK_ERROR(ret);
	VIHelper vi_tea_full(6, 12, &vo_tea_full);
	VIHelper vi_stu_full(4, 8, &vo_stu_full);
	VIHelper vi_black_board(2, 4, &vo_black_board);
	ret = MPPSystem::Bind<HI_ID_VIU, HI_ID_VPSS>(0, 12, 0, 0);
	CHECK_ERROR(ret);
	ret = MPPSystem::Bind<HI_ID_VIU, HI_ID_VPSS>(0, 8, 1, 0);
	CHECK_ERROR(ret);
	ret = MPPSystem::Bind<HI_ID_VIU, HI_ID_VPSS>(0, 4, 2, 0);
	CHECK_ERROR(ret);
	ret = MPPSystem::Bind<HI_ID_VPSS, HI_ID_VOU>(0, 4, 10, 0);
	CHECK_ERROR(ret);
	ret = MPPSystem::Bind<HI_ID_VPSS, HI_ID_VOU>(1, 4, 11, 0);
	CHECK_ERROR(ret);
	ret = MPPSystem::Bind<HI_ID_VPSS, HI_ID_VOU>(2, 4, 12, 0);
	CHECK_ERROR(ret);
	//#####################################################
	//初始化从片3上的tw6874信号接收器
	//#####################################################
	ret = tw6874_tea_full.Initialize();
	CHECK_ERROR(ret);
	ret = tw6874_stu_full.Initialize();
	CHECK_ERROR(ret);
	ret = tw6874_black_board.Initialize();
	CHECK_ERROR(ret);
	tw6874_tea_full.SetVIFmtListener(&vi_tea_full);
	tw6874_stu_full.SetVIFmtListener(&vi_stu_full);
	tw6874_black_board.SetVIFmtListener(&vi_black_board);
	//#####################################################
	//初始化从片3上的编码器 vir_vo->venc
	//#####################################################
	ret = venc_tea_full.Initialize({0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT, RS_FRAME_RATE, 0, 20000, VENC_RC_MODE_H264CBR});
	CHECK_ERROR(ret);
	ret = venc_stu_full.Initialize({1, 1, RS_MAX_WIDTH, RS_MAX_HEIGHT, RS_FRAME_RATE, 0, 20000, VENC_RC_MODE_H264CBR});
	CHECK_ERROR(ret);
	ret = venc_black_board.Initialize({2, 2, RS_MAX_WIDTH, RS_MAX_HEIGHT, RS_FRAME_RATE, 0, 20000, VENC_RC_MODE_H264CBR});
	CHECK_ERROR(ret);
	ret = MPPSystem::Bind<HI_ID_VOU, HI_ID_GROUP>(10, 0, 0, 0);
	CHECK_ERROR(ret);
	ret = MPPSystem::Bind<HI_ID_VOU, HI_ID_GROUP>(11, 0, 1, 0);
	CHECK_ERROR(ret);
	ret = MPPSystem::Bind<HI_ID_VOU, HI_ID_GROUP>(12, 0, 2, 0);
	CHECK_ERROR(ret);
#endif

	ret = PCIVComm::Instance()->Initialize();
	CHECK_ERROR(ret);

	pciv::Msg msg;
	uint8_t tmp_buf[1024];
	Buffer<allocator_1k> msg_buf;

	while (g_Run)
	{
		ret = Recv(PCIVComm::Instance(), RS_PCIV_MASTER_ID, RS_PCIV_CMD_PORT, tmp_buf, sizeof(tmp_buf), msg_buf, g_Run, msg);
		CHECK_ERROR(ret);

		if (!g_Run)
			break;

		switch (msg.type)
		{
#if CHIP_TYPE == 1
		case pciv::Msg::Type::CONF_ADV7842:
		{
			pciv::Adv7842Conf conf;
			memcpy(&conf, msg.data, sizeof(conf));
			Adv7842::Instance()->Close();
			ret = Adv7842::Instance()->Initialize(static_cast<ADV7842_MODE>(conf.mode));
			CHECK_ERROR(ret);
			Adv7842::Instance()->SetVIFmtListener(&vi_pc);
			break;
		}
		case pciv::Msg::Type::QUERY_ADV7842:
		{
			pciv::Adv7842Query query;
			memset(&query, 0, sizeof(query));
			Adv7842::Instance()->GetInputFormat(query.fmt);

			msg.type = pciv::Msg::Type::ACK;
			memcpy(msg.data, &query, sizeof(query));
			ret = PCIVComm::Instance()->Send(RS_PCIV_MASTER_ID, RS_PCIV_CMD_PORT, reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
			CHECK_ERROR(ret)
			break;
		}
		case pciv::Msg::Type::START_TRANS:
		{
			pciv::MemoryInfo mem_info;
			memcpy(&mem_info, msg.data, sizeof(mem_info));
			venc_pc.SetVideoSink(nullptr);
			PCIVTrans::Instance()->Close();
			ret = PCIVTrans::Instance()->Initialize(PCIVComm::Instance(), mem_info);
			CHECK_ERROR(ret);
			venc_pc.SetVideoSink(PCIVTrans::Instance());
			break;
		}
		case pciv::Msg::Type::STOP_TRANS:
		{
			venc_pc.SetVideoSink(nullptr);
			PCIVTrans::Instance()->Close();
			break;
		}
#else
		case pciv::Msg::Type::QUERY_TW6874:
		{
			pciv::Tw6874Query query;
			memcpy(&query, msg.data, sizeof(query));
			tw6874_tea_full.UpdateVIFmt(query.fmts[0]);
			tw6874_stu_full.UpdateVIFmt(query.fmts[1]);
			tw6874_black_board.UpdateVIFmt(query.fmts[2]);
			break;
		}
		case pciv::Msg::Type::START_TRANS:
		{
			pciv::MemoryInfo mem_info;
			memcpy(&mem_info, msg.data, sizeof(mem_info));
			venc_tea_full.SetVideoSink(nullptr);
			venc_stu_full.SetVideoSink(nullptr);
			venc_black_board.SetVideoSink(nullptr);
			PCIVTrans::Instance()->Close();
			ret = PCIVTrans::Instance()->Initialize(PCIVComm::Instance(), mem_info);
			CHECK_ERROR(ret);
			venc_tea_full.SetVideoSink(PCIVTrans::Instance());
			venc_stu_full.SetVideoSink(PCIVTrans::Instance());
			venc_black_board.SetVideoSink(PCIVTrans::Instance());
			break;
		}
		case pciv::Msg::Type::STOP_TRANS:
		{
			venc_tea_full.SetVideoSink(nullptr);
			venc_stu_full.SetVideoSink(nullptr);
			venc_black_board.SetVideoSink(nullptr);
			PCIVTrans::Instance()->Close();
			break;
		}
#endif
		default:
		{
			log_e("unknow msg type:%d", msg.type);
			break;
		}
		}
	}

	PCIVTrans::Instance()->Close();
	PCIVComm::Instance()->Close();
#if CHIP_TYPE == 1
	MPPSystem::UnBind<HI_ID_VOU, HI_ID_GROUP>(10, 0, 0, 0);
	venc_pc.Close();
	Adv7842::Instance()->Close();
	MPPSystem::UnBind<HI_ID_VPSS, HI_ID_VOU>(0, 4, 10, 0);
	MPPSystem::UnBind<HI_ID_VIU, HI_ID_VPSS>(0, 8, 0, 0);
	vo_pc.Close();
	vpss_pc.Close();
#else
	MPPSystem::UnBind<HI_ID_VOU, HI_ID_GROUP>(12, 0, 2, 0);
	MPPSystem::UnBind<HI_ID_VOU, HI_ID_GROUP>(11, 0, 1, 0);
	MPPSystem::UnBind<HI_ID_VOU, HI_ID_GROUP>(10, 0, 0, 0);
	venc_black_board.Close();
	venc_stu_full.Close();
	venc_tea_full.Close();
	tw6874_black_board.Close();
	tw6874_stu_full.Close();
	tw6874_tea_full.Close();
	MPPSystem::UnBind<HI_ID_VPSS, HI_ID_VOU>(2, 4, 12, 0);
	MPPSystem::UnBind<HI_ID_VPSS, HI_ID_VOU>(1, 4, 11, 0);
	MPPSystem::UnBind<HI_ID_VPSS, HI_ID_VOU>(0, 4, 10, 0);
	MPPSystem::UnBind<HI_ID_VIU, HI_ID_VPSS>(0, 4, 2, 0);
	MPPSystem::UnBind<HI_ID_VIU, HI_ID_VPSS>(0, 8, 1, 0);
	MPPSystem::UnBind<HI_ID_VIU, HI_ID_VPSS>(0, 12, 0, 0);
	vo_black_board.Close();
	vo_stu_full.Close();
	vo_tea_full.Close();
	vpss_black_board.Close();
	vpss_stu_full.Close();
	vpss_tea_full.Close();
#endif
	MPPSystem::Instance()->Close();
	return KSuccess;
}
