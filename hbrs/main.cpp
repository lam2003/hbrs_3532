
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

static bool g_Run = true;
static std::string g_Opt = "";

static void SignalHandler(int signo)
{
	if (signo == SIGINT || signo == SIGTERM)
	{
		g_Run = false;
	}
}

static int Recv(std::shared_ptr<PCIVComm> pciv_comm, int remote_id, int port, uint8_t *tmp_buf, int32_t buf_len, Buffer<allocator_1k> &msg_buf, bool &run, pciv::Msg &msg)
{
	int ret;
	do
	{
		ret = pciv_comm->Recv(remote_id, port, tmp_buf, buf_len, 500000); //500ms
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
	ConfigLogger("./3532.log", RS_VERSION);

	signal(SIGINT, SignalHandler);
	signal(SIGTERM, SignalHandler);

	MPPSystem::Instance()->Initialize();

	std::shared_ptr<PCIVComm> pciv_comm = std::make_shared<PCIVComm>();
	std::shared_ptr<PCIVTrans> pciv_trans = std::make_shared<PCIVTrans>();

#if CHIP_TYPE == 1
	std::shared_ptr<VIHelper> vi_pc = std::make_shared<VIHelper>(4, 8);
	std::shared_ptr<Adv7842> adv7842 = std::make_shared<Adv7842>();
	std::shared_ptr<VideoProcess> vpss_pc = std::make_shared<VideoProcess>();
	std::shared_ptr<VideoOutput> vo_pc = std::make_shared<VideoOutput>();
	std::shared_ptr<VideoEncode> venc_pc = std::make_shared<VideoEncode>();
#if 0 
	vi_pc->Start(RS_MAX_WIDTH, RS_MAX_HEIGHT, false, 60);
#endif
	adv7842->Initialize(MODE_HDMI);
	adv7842->SetVIFmtListener(vi_pc);

	vpss_pc->Initialize({0});

	vo_pc->Initialize({10, 0, VO_OUTPUT_1080P25});
	vo_pc->StartChannel(0, {0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT}, 0);
	vi_pc->SetVideoOutput(vo_pc);

	venc_pc->Initialize({0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT, 25, 25, 0, 20000, VENC_RC_MODE_H264CBR});

	MPPSystem::Bind<HI_ID_VIU, HI_ID_VPSS>(0, 8, 0, 0);
	MPPSystem::Bind<HI_ID_VPSS, HI_ID_VOU>(0, 4, 10, 0);
	MPPSystem::Bind<HI_ID_VOU, HI_ID_GROUP>(10, 0, 0, 0);

#else
	std::shared_ptr<Tw6874> tw6874_tea_full = std::make_shared<Tw6874>();
	std::shared_ptr<Tw6874> tw6874_stu_full = std::make_shared<Tw6874>();
	std::shared_ptr<Tw6874> tw6874_black_board = std::make_shared<Tw6874>();
	std::shared_ptr<VIHelper> vi_tea_full = std::make_shared<VIHelper>(6, 12);
	std::shared_ptr<VIHelper> vi_stu_full = std::make_shared<VIHelper>(4, 8);
	std::shared_ptr<VIHelper> vi_black_board = std::make_shared<VIHelper>(2, 4);
	std::shared_ptr<VideoProcess> vpss_tea_full = std::make_shared<VideoProcess>();
	std::shared_ptr<VideoProcess> vpss_stu_full = std::make_shared<VideoProcess>();
	std::shared_ptr<VideoProcess> vpss_black_board = std::make_shared<VideoProcess>();
	std::shared_ptr<VideoOutput> vo_tea_full = std::make_shared<VideoOutput>();
	std::shared_ptr<VideoOutput> vo_stu_full = std::make_shared<VideoOutput>();
	std::shared_ptr<VideoOutput> vo_black_board = std::make_shared<VideoOutput>();
	std::shared_ptr<VideoEncode> venc_tea_full = std::make_shared<VideoEncode>();
	std::shared_ptr<VideoEncode> venc_stu_full = std::make_shared<VideoEncode>();
	std::shared_ptr<VideoEncode> venc_black_board = std::make_shared<VideoEncode>();

#if 0
	vi_tea_full->Start(RS_MAX_WIDTH, RS_MAX_HEIGHT, false, 30);

	vi_stu_full->Start(RS_MAX_WIDTH, RS_MAX_HEIGHT, false, 30);

	vi_black_board->Start(RS_MAX_WIDTH, RS_MAX_HEIGHT, false, 30);
#endif
	tw6874_tea_full->Initialize();
	tw6874_tea_full->SetVIFmtListener(vi_tea_full);

	tw6874_stu_full->Initialize();
	tw6874_stu_full->SetVIFmtListener(vi_stu_full);

	tw6874_black_board->Initialize();
	tw6874_black_board->SetVIFmtListener(vi_black_board);

	vpss_tea_full->Initialize({0});

	vpss_stu_full->Initialize({1});

	vpss_black_board->Initialize({2});

	vo_tea_full->Initialize({10, 0, VO_OUTPUT_1080P25});
	vo_tea_full->StartChannel(0, {0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT}, 0);
	vi_tea_full->SetVideoOutput(vo_tea_full);

	vo_stu_full->Initialize({11, 0, VO_OUTPUT_1080P25});
	vo_stu_full->StartChannel(0, {0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT}, 0);
	vi_stu_full->SetVideoOutput(vo_stu_full);

	vo_black_board->Initialize({12, 0, VO_OUTPUT_1080P25});
	vo_black_board->StartChannel(0, {0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT}, 0);
	vi_black_board->SetVideoOutput(vo_black_board);

	venc_tea_full->Initialize({0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT, 25, 25, 0, 20000, VENC_RC_MODE_H264CBR});

	venc_stu_full->Initialize({1, 1, RS_MAX_WIDTH, RS_MAX_HEIGHT, 25, 25, 0, 20000, VENC_RC_MODE_H264CBR});

	venc_black_board->Initialize({2, 2, RS_MAX_WIDTH, RS_MAX_HEIGHT, 25, 25, 0, 20000, VENC_RC_MODE_H264CBR});

	MPPSystem::Bind<HI_ID_VIU, HI_ID_VPSS>(0, 12, 0, 0);
	MPPSystem::Bind<HI_ID_VIU, HI_ID_VPSS>(0, 8, 1, 0);
	MPPSystem::Bind<HI_ID_VIU, HI_ID_VPSS>(0, 4, 2, 0);
	MPPSystem::Bind<HI_ID_VPSS, HI_ID_VOU>(0, 4, 10, 0);
	MPPSystem::Bind<HI_ID_VPSS, HI_ID_VOU>(1, 4, 11, 0);
	MPPSystem::Bind<HI_ID_VPSS, HI_ID_VOU>(2, 4, 12, 0);
	MPPSystem::Bind<HI_ID_VOU, HI_ID_GROUP>(10, 0, 0, 0);
	MPPSystem::Bind<HI_ID_VOU, HI_ID_GROUP>(11, 0, 1, 0);
	MPPSystem::Bind<HI_ID_VOU, HI_ID_GROUP>(12, 0, 2, 0);
#endif

	pciv_comm->Initialize();

	pciv::Msg msg;
	uint8_t tmp_buf[1024];
	Buffer<allocator_1k> msg_buf;

	while (g_Run)
	{
		Recv(pciv_comm, RS_PCIV_MASTER_ID, RS_PCIV_CMD_PORT, tmp_buf, sizeof(tmp_buf), msg_buf, g_Run, msg);

		if (!g_Run)
			break;

		switch (msg.type)
		{
#if CHIP_TYPE == 1
		case pciv::Msg::Type::CONF_ADV7842:
		{
			pciv::Adv7842Conf conf;
			memcpy(&conf, msg.data, sizeof(conf));
			adv7842->Close();
			adv7842->Initialize(static_cast<ADV7842_MODE>(conf.mode));
			adv7842->SetVIFmtListener(vi_pc);
			break;
		}
		case pciv::Msg::Type::QUERY_ADV7842:
		{
			pciv::Adv7842Query query;
			memset(&query, 0, sizeof(query));
			adv7842->GetInputFormat(query.fmt);
			msg.type = pciv::Msg::Type::ACK;
			memcpy(msg.data, &query, sizeof(query));
			pciv_comm->Send(RS_PCIV_MASTER_ID, RS_PCIV_CMD_PORT, reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
			break;
		}
		case pciv::Msg::Type::START_TRANS:
		{
			pciv::MemoryInfo mem_info;
			memcpy(&mem_info, msg.data, sizeof(mem_info));
			venc_pc->SetVideoSink(nullptr);
			pciv_trans->Close();
			pciv_trans->Initialize(pciv_comm, mem_info);
			venc_pc->SetVideoSink(pciv_trans);
			break;
		}
		case pciv::Msg::Type::STOP_TRANS:
		{
			venc_pc->SetVideoSink(nullptr);
			pciv_trans->Close();
			break;
		}
#else
		case pciv::Msg::Type::QUERY_TW6874:
		{
			pciv::Tw6874Query query;
			memcpy(&query, msg.data, sizeof(query));
			tw6874_tea_full->UpdateVIFmt(query.fmts[0]);
			tw6874_stu_full->UpdateVIFmt(query.fmts[1]);
			tw6874_black_board->UpdateVIFmt(query.fmts[2]);
			break;
		}
		case pciv::Msg::Type::START_TRANS:
		{
			pciv::MemoryInfo mem_info;
			memcpy(&mem_info, msg.data, sizeof(mem_info));
			venc_tea_full->SetVideoSink(nullptr);
			venc_stu_full->SetVideoSink(nullptr);
			venc_black_board->SetVideoSink(nullptr);
			pciv_trans->Close();
			pciv_trans->Initialize(pciv_comm, mem_info);
			venc_tea_full->SetVideoSink(pciv_trans);
			venc_stu_full->SetVideoSink(pciv_trans);
			venc_black_board->SetVideoSink(pciv_trans);
			break;
		}
		case pciv::Msg::Type::STOP_TRANS:
		{
			venc_tea_full->SetVideoSink(nullptr);
			venc_stu_full->SetVideoSink(nullptr);
			venc_black_board->SetVideoSink(nullptr);
			pciv_trans->Close();
			break;
		}
#endif
		case pciv::Msg::Type::SHUTDOWN:
		{
			g_Run = false;
			g_Opt = "shutdown";
			break;
		}
		case pciv::Msg::Type::REBOOT:
		{
			g_Run = false;
			g_Opt = "reboot";
			break;
		}
		default:
		{
			log_e("unknow msg type:%d", msg.type);
			break;
		}
		}
	}

	pciv_trans->Close();

	pciv_comm->Close();

	pciv_trans.reset();
	pciv_trans = nullptr;

	pciv_comm.reset();
	pciv_comm = nullptr;

#if CHIP_TYPE == 1
	MPPSystem::UnBind<HI_ID_VOU, HI_ID_GROUP>(10, 0, 0, 0);
	MPPSystem::UnBind<HI_ID_VPSS, HI_ID_VOU>(0, 4, 10, 0);
	MPPSystem::UnBind<HI_ID_VIU, HI_ID_VPSS>(0, 8, 0, 0);

	venc_pc->SetVideoSink(nullptr);
	venc_pc->Close();

	vi_pc->SetVideoOutput(nullptr);
	vo_pc->Close();

	vpss_pc->Close();

	adv7842->SetVIFmtListener(nullptr);
	adv7842->Close();

	vi_pc->Stop();

	venc_pc.reset();
	venc_pc = nullptr;
	vo_pc.reset();
	vo_pc = nullptr;
	vpss_pc.reset();
	vpss_pc = nullptr;
	adv7842.reset();
	adv7842 = nullptr;
	vi_pc.reset();
	vi_pc = nullptr;
#else
	MPPSystem::UnBind<HI_ID_VOU, HI_ID_GROUP>(12, 0, 2, 0);
	MPPSystem::UnBind<HI_ID_VOU, HI_ID_GROUP>(11, 0, 1, 0);
	MPPSystem::UnBind<HI_ID_VOU, HI_ID_GROUP>(10, 0, 0, 0);
	MPPSystem::UnBind<HI_ID_VPSS, HI_ID_VOU>(2, 4, 12, 0);
	MPPSystem::UnBind<HI_ID_VPSS, HI_ID_VOU>(1, 4, 11, 0);
	MPPSystem::UnBind<HI_ID_VPSS, HI_ID_VOU>(0, 4, 10, 0);
	MPPSystem::UnBind<HI_ID_VIU, HI_ID_VPSS>(0, 4, 2, 0);
	MPPSystem::UnBind<HI_ID_VIU, HI_ID_VPSS>(0, 8, 1, 0);
	MPPSystem::UnBind<HI_ID_VIU, HI_ID_VPSS>(0, 12, 0, 0);

	venc_black_board->SetVideoSink(nullptr);
	venc_black_board->Close();

	venc_stu_full->SetVideoSink(nullptr);
	venc_stu_full->Close();

	venc_tea_full->SetVideoSink(nullptr);
	venc_tea_full->Close();

	vi_black_board->SetVideoOutput(nullptr);
	vo_black_board->Close();

	vi_stu_full->SetVideoOutput(nullptr);
	vo_stu_full->Close();

	vi_tea_full->SetVideoOutput(nullptr);
	vo_tea_full->Close();

	vpss_black_board->Close();

	vpss_stu_full->Close();

	vpss_tea_full->Close();

	tw6874_black_board->SetVIFmtListener(nullptr);
	tw6874_black_board->Close();

	tw6874_stu_full->SetVIFmtListener(nullptr);
	tw6874_stu_full->Close();

	tw6874_stu_full->SetVIFmtListener(nullptr);
	tw6874_tea_full->Close();

	vi_black_board->Stop();

	vi_stu_full->Stop();

	vi_tea_full->Stop();

	venc_black_board.reset();
	venc_black_board = nullptr;

	venc_stu_full.reset();
	venc_stu_full = nullptr;

	venc_tea_full.reset();
	venc_tea_full = nullptr;

	vo_black_board.reset();
	vo_black_board = nullptr;

	vo_stu_full.reset();
	vo_stu_full = nullptr;

	vo_tea_full.reset();
	vo_tea_full = nullptr;

	vpss_black_board.reset();
	vpss_black_board = nullptr;

	vpss_stu_full.reset();
	vpss_stu_full = nullptr;

	vpss_tea_full.reset();
	vpss_tea_full = nullptr;

	vi_black_board.reset();
	vi_black_board = nullptr;

	vi_stu_full.reset();
	vi_stu_full = nullptr;

	vi_tea_full.reset();
	vi_tea_full = nullptr;

	tw6874_black_board.reset();
	tw6874_black_board = nullptr;

	tw6874_stu_full.reset();
	tw6874_stu_full = nullptr;

	tw6874_tea_full.reset();
	tw6874_tea_full = nullptr;
#endif
	MPPSystem::Instance()->Close();

	if (g_Opt == "shutdown")
	{
		system("poweroff");
	}
	else if (g_Opt == "reboot")
	{
		system("reboot");
	}

	return KSuccess;
}
