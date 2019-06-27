#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>

#include "system/vi.h"
#include "system/mpp.h"
#include "system/vpss.h"
#include "system/vo.h"
#include "system/venc.h"
#include "system/adv7842.h"
#include "system/pciv_trans.h"
#include "common/utils.h"

#include "common/err_code.h"
#include "common/buffer.h"
#include "system/pciv_comm.h"

static bool g_Run = true;

class Test : public rs::VideoSink<VENC_STREAM_S>
{
public:
	void OnFrame(const VENC_STREAM_S &st, int chn)
	{
		printf("##############\n");
	}
};

int32_t
main(int32_t argc, char **argv)
{
	int32_t ret;
	//全局设置为1080P(支持的最大分辨率),不用改变
	rs::MPPSystem::Instance()->Initialize(10);
	rs::Adv7842::Instance()->Initialize(MODE_HDMI);
// printf("sleep10\n");
// 	sleep(10);
// 	printf("closing...\n");
// 	rs::Adv7842::Instance()->Close();
// 	rs::MPPSystem::Instance()->Close();
// 	printf("closed\n");
	rs::VideoInput vi;
	vi.Initialize({6, 12, CAPTURE_MODE_1080P});

	rs::VideoProcess vpss;
	vpss.Initialize({0, CAPTURE_MODE_1080P});

	rs::MPPSystem::Bind<HI_ID_VIU, HI_ID_VPSS>(0, 12, 0, 0);

	rs::VideoEncode venc;
	venc.Initialize({0, 0, 1920, 1080, 30, 2, 8000, VENC_RC_MODE_H264CBR});

	rs::MPPSystem::Bind<HI_ID_VPSS, HI_ID_GROUP>(0, 1, 0, 0);

	//初始化PCIV信令模块
	ret = rs::PCIVComm::Instance()->Initialize();
	if (ret != KSuccess)
	{
		log_e("error:%s", make_error_code(static_cast<err_code>(ret)).message().c_str());
		return ret;
	}	


	rs::pciv::Msg msg;
	uint8_t tmp_buf[1024];
	rs::Buffer<rs::allocator_1k> msg_buf;
	while (g_Run)
	{
		do
		{
			ret = rs::PCIVComm::Instance()->Recv(PCIV_MASTER_ID, rs::PCIVComm::Instance()->GetCMDPort(), tmp_buf, sizeof(tmp_buf),500000);
			if (ret > 0)
			{
				if (!msg_buf.Append(tmp_buf, ret))
				{
					log_e("buffer fill");
					return KNotEnoughBuf;
				}
			}
		} while (g_Run && msg_buf.Size() < sizeof(msg));

		if (msg_buf.Size() >= sizeof(msg))
		{

			msg_buf.Get(reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
			msg_buf.Consume(sizeof(msg));

			printf("msg.type:%d\n", msg.type);

			switch (msg.type)
			{
			case rs::pciv::Msg::Type::NOTIFY_MEMORY:
			{
				rs::pciv::MemoryInfo mem_info;
				memcpy(&mem_info, msg.data, sizeof(mem_info));
				venc.RemoveAllVideoSink();
				rs::PCIVTrans::Instance()->Close();
				rs::PCIVTrans::Instance()->Initialize(rs::PCIVComm::Instance(), mem_info);
				venc.AddVideoSink(rs::PCIVTrans::Instance());
				break;
			}
			}
		}
	}

	while (1)
		;
	sleep(1000);

	return 0;
}

