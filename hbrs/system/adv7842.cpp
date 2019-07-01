//self
#include "system/adv7842.h"
#include "common/err_code.h"

namespace rs
{

Adv7842::~Adv7842()
{
    Close();
}

Adv7842::Adv7842() : thread_(nullptr),
                     run_(false),
                     listener_(nullptr),
                     init_(false)
{
}

int Adv7842::Initialize(ADV7842_CMODE_E mode)
{
    if (init_)
        return KInitialized;

    memset(&fmt_, 0, sizeof(fmt_));

    run_ = true;
    thread_ = std::unique_ptr<std::thread>(new std::thread([this, mode]() {
        int ret;
        const char *dev = "/dev/adv7842";

        int fd = open(dev, O_RDWR);
        if (fd < 0)
        {
            log_e("open %s failed,%s", dev, strerror(errno));
            return;
        }

        ret = ioctl(fd, CMD_DEVICE_INIT, mode);
        if (ret < 0)
        {
            log_e("ioctl CMD_DEVICE_INIT failed,%s", strerror(errno));
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
                return;
            }
            htot = bt.hfrontporch + bt.hsync + bt.hbackporch + bt.width;
            vtot = bt.vfrontporch + bt.vsync + bt.vbackporch + bt.height;
            fps = 0;
            if (htot * vtot != 0)
                fps = bt.pixelclock / (htot * vtot);

            VideoInputFormat new_fmt;
            new_fmt.has_signal = true;
            if (bt.width <= 0 || bt.height <= 0 || htot <= 0 || vtot <= 0 || fps <= 0)
                new_fmt.has_signal = false;
            new_fmt.width = bt.width;
            new_fmt.height = bt.height;
            new_fmt.interlaced = bt.interlaced;
            new_fmt.frame_rate = fps;

            if (fmt_ != new_fmt)
            {
                std::unique_lock<std::mutex> lock(mux_);
                fmt_ = new_fmt;
                if (listener_ != nullptr)
                    listener_->OnChange(fmt_);
            }

            usleep(500000); //500ms
        }
        close(fd);
    }));

    init_ = true;
    return KSuccess;
}

void Adv7842::GetInputFormat(VideoInputFormat &fmt)
{
    if (!init_)
        return;
    std::unique_lock<std::mutex> lock(mux_);
    fmt = fmt_;
}

void Adv7842::Close()
{
    if (!init_)
        return;

    run_ = false;
    thread_->join();
    thread_.reset();
    thread_ = nullptr;
    if (listener_ != nullptr)
        listener_->OnStop();
    listener_ = nullptr;

    init_ = false;
}

Adv7842 *Adv7842::Instance()
{
    static Adv7842 *instance = new Adv7842;
    return instance;
}

void Adv7842::SetVIFmtListener(VIFmtListener *listener)
{
    std::unique_lock<std::mutex> lock(mux_);
    listener_ = listener;
}
} // namespace rs