

#include "system/mpp.h"
#include "system/adv7842.h"
#include "system/pciv_comm.h"
#include "system/pciv_trans.h"
#include "system/vi.h"
#include "system/vo.h"
#include "system/vpss.h"
#include "system/venc.h"
#include "common/utils.h"

using namespace rs;

#define CHACK_ERROR(a)                                                                  \
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

int32_t main(int32_t argc, char **argv)
{
	int ret;

	signal(SIGINT, SignalHandler);

	ret = MPPSystem::Instance()->Initialize(RS_MEM_BLK_NUM);
	CHACK_ERROR(ret);

#if CHIP_TYPE == 1
	VIHelper vi_pc(4, 8);
	VideoProcess vpss_pc;
	VideoOutput vo_pc;
	VideoEncode venc_pc;

	//初始化VPSS
	ret = vpss_pc.Initialize({0});
	CHACK_ERROR(ret);
	//配置VPSS通道1
	ret = vpss_pc.SetChnSize(1, {RS_MAX_WIDTH, RS_MAX_HEIGHT});
	CHACK_ERROR(ret);
	//初始化虚拟VO
	ret = vo_pc.Initialize({10, 0, VO_OUTPUT_1080P60});
	CHACK_ERROR(ret);
	//配置虚拟VO开始通道0
	ret = vo_pc.StartChn({{0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT}, 0, 0});
	CHACK_ERROR(ret);
	//初始化VENC
	ret = venc_pc.Initialize({0, 0, RS_MAX_WIDTH, RS_MAX_HEIGHT, 60, 0, 20000, VENC_RC_MODE_H264CBR});
	CHACK_ERROR(ret);
	//绑定VI(4,8)与VPSS(0)
	ret = MPPSystem::Bind<HI_ID_VIU, HI_ID_VPSS>(0, 8, 0, 0);
	CHACK_ERROR(ret);
	//绑定VPSS(0,1)与虚拟VO(10,0)
	ret = MPPSystem::Bind<HI_ID_VPSS, HI_ID_VOU>(0, 1, 10, 0);
	CHACK_ERROR(ret);
	//绑定虚拟VO(10,0)与VENC(0,0)
	ret = MPPSystem::Bind<HI_ID_VOU, HI_ID_GROUP>(10, 0, 0, 0);
	CHACK_ERROR(ret);
#endif

	ret = PCIVComm::Instance()->Initialize();
	CHACK_ERROR(ret);

	pciv::Msg msg;
	uint8_t tmp_buf[1024];
	Buffer<allocator_1k> msg_buf;

	while (g_Run)
	{
		ret = Utils::Recv(PCIVComm::Instance(), RS_PCIV_MASTER_ID, RS_PCIV_CMD_PORT, tmp_buf, sizeof(tmp_buf), msg_buf, g_Run, msg);
		CHACK_ERROR(ret);

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
			CHACK_ERROR(ret);
			Adv7842::Instance()->SetVIFmtListener(&vi_pc);
			break;
		}
		case pciv::Msg::Type::QUERY_ADV7842:
		{
			pciv::QueryVIFmt query;
			memset(&query, 0, sizeof(query));
			Adv7842::Instance()->GetInputFormat(query.fmt);

			msg.type = pciv::Msg::Type::ACK;
			memcpy(msg.data, &query, sizeof(query));
			ret = PCIVComm::Instance()->Send(RS_PCIV_MASTER_ID, RS_PCIV_CMD_PORT, reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
			CHACK_ERROR(ret)
			break;
		}
#endif
		case pciv::Msg::Type::START_TRANS:
		{
			pciv::MemoryInfo mem_info;
			memcpy(&mem_info, msg.data, sizeof(mem_info));
			venc_pc.SetVideoSink(nullptr);
			PCIVTrans::Instance()->Close();
			ret = PCIVTrans::Instance()->Initialize(PCIVComm::Instance(), mem_info);
			CHACK_ERROR(ret);
			venc_pc.SetVideoSink(PCIVTrans::Instance());
			break;
		}
		case pciv::Msg::Type::STOP_TRANS:
		{
			venc_pc.SetVideoSink(nullptr);
			PCIVTrans::Instance()->Close();
			break;
		}
		default:
		{
			log_e("unknow msg type:%d", msg.type);
			break;
		}
		}
	}

	Adv7842::Instance()->Close();
	PCIVTrans::Instance()->Close();
	PCIVComm::Instance()->Close();
#if CHIP_TYPE == 1
	MPPSystem::UnBind<HI_ID_VOU, HI_ID_GROUP>(10, 0, 0, 0);
	MPPSystem::UnBind<HI_ID_VPSS, HI_ID_VOU>(0, 1, 10, 0);
	MPPSystem::UnBind<HI_ID_VIU, HI_ID_VPSS>(0, 8, 0, 0);
	venc_pc.Close();
	vo_pc.Close();
	vpss_pc.Close();
#endif
	MPPSystem::Instance()->Close();
	return KSuccess;
}
