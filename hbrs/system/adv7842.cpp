#include "system/adv7842.h"

namespace rs
{

static const char *const input_color_space_txt[16] = {
    "RGB limited range (16-235)",
    "RGB full range (0-255)",
    "YCbCr Bt.601 (16-235)",
    "YCbCr Bt.709 (16-235)",
    "xvYCC Bt.601",
    "xvYCC Bt.709",
    "YCbCr Bt.601 (0-255)",
    "YCbCr Bt.709 (0-255)",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "automatic"};

Adv7842::~Adv7842()
{
}

Adv7842::Adv7842() : thread_(nullptr),
                     run_(false),
                     has_signal_(false)
{
}

int Adv7842::Initialize(ADV7842_CMODE_E mode)
{
    if (init_)
        return KInitialized;
    run_ = true;
    thread_ = std::unique_ptr<std::thread>(new std::thread([this, mode]() {
        const char *dev = "/dev/adv7842";
        int fd = open(dev, O_RDWR);
        if (fd < 0)
        {
            log_e("open %s failed,%s", dev, strerror(errno));
            log_e("error:%s", make_error_code(static_cast<err_code>(KSystemError)).message().c_str());
            return;
        }
        int ret = ioctl(fd, CMD_DEVICE_INIT, mode);
        if (ret < 0)
        {
            log_e("ioctl CMD_DEVICE_INIT failed,%s", strerror(errno));
            log_e("error:%s", make_error_code(static_cast<err_code>(KSystemError)).message().c_str());
            return;
        }

        bt_timings bt;
        int htot, vtot, fps;
        while (run_)
        {

            memset(&bt, 0, sizeof(bt));
            ret = ioctl(fd, CMD_QUERY_TIMINGS, &bt);
            if (ret < 0)
            {
                log_e("ioctl CMD_QUERY_TIMINGS failed,%s", strerror(errno));
                log_e("error:%s", make_error_code(static_cast<err_code>(KSystemError)).message().c_str());
                return;
            }
            htot = bt.hfrontporch + bt.hsync + bt.hbackporch + bt.width;
            vtot = bt.vfrontporch + bt.vsync + bt.vbackporch + bt.height;
            fps = 0;
            if (htot * vtot != 0)
                fps = bt.pixelclock / (htot * vtot);

            if (bt.width <= 0 || bt.height <= 0 || htot <= 0 || vtot <= 0 || fps <= 0)
                has_signal_ = false;

            log_e("device detect timing: %dx%d@%d%s (%dx%d) colorspace:%d %s",
                  bt.width, bt.height, fps, bt.interlaced ? "i" : "p", htot, vtot,
                  bt.hdmi_colorspace, input_color_space_txt[bt.hdmi_colorspace]);
            usleep(1000000); //1s
        }
    }));
    return KSuccess;
}

void Adv7842::Close()
{
}

Adv7842 *Adv7842::Instance()
{
    static Adv7842 *instance = new Adv7842;
    return instance;
}

} // namespace rs