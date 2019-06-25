#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>

#include "system/vi.h"
#include "system/mpp.h"
#include "system/vpss.h"
#include "system/vo.h"
#include "common/utils.h"

#include "common/err_code.h"
#include "common/buffer.h"
#include "system/pciv_comm.h"

static bool g_Run = true;

int32_t main(int32_t argc, char **argv)
{
	int32_t ret;
	//全局设置为1080P(支持的最大分辨率),不用改变
	rs::MPPSystem::Instance()->Initialize({CAPTURE_MODE_1080P, 10});

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
			ret = rs::PCIVComm::Instance()->Recv(PCIV_MASTER_ID, rs::PCIVComm::Instance()->GetTransWritePort(), tmp_buf, sizeof(tmp_buf));
			if (ret > 0)
			{
				if (!msg_buf.Append(tmp_buf, ret))
				{
					log_e("buffer fill");
					return KNotEnoughBuf;
				}
			}
			else
			{
				usleep(10000); //10ms
			}
		} while (g_Run && msg_buf.Size() < sizeof(msg));

		if (msg_buf.Size() >= sizeof(msg))
		{

			msg_buf.Get(reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
			msg_buf.Consume(sizeof(msg));

			printf("msg.type:%d\n",msg.type);
		}
	}

	return 0;
}
