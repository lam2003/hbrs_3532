//self
#include "system/tw6874.h"
#include "common/err_code.h"

namespace rs
{
Tw6874::Tw6874() : listener_(nullptr),
                   run_(false),
                   thread_(nullptr),
                   init_(false)
{
}

Tw6874::~Tw6874()
{
    Close();
}

int Tw6874::Initialize()
{
    if (init_)
        return KInitialized;

    memset(&now_fmt_, 0, sizeof(now_fmt_));
    memset(&run_fmt_, 0, sizeof(run_fmt_));

    run_ = true;
    thread_ = std::unique_ptr<std::thread>(new std::thread([this]() {
        while (run_)
        {
            {
                std::unique_lock<std::mutex> lock(mux_);
                if (run_fmt_ != now_fmt_)
                {
                    run_fmt_ = now_fmt_;
                    if (listener_ != nullptr)
                        listener_->OnChange(run_fmt_);
                }
            }
            usleep(500000); //500ms
        }
    }));

    init_ = true;
    return KSuccess;
}

void Tw6874::Close()
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

void Tw6874::UpdateVIFmt(const VideoInputFormat &fmt)
{
    std::unique_lock<std::mutex> lock(mux_);
    now_fmt_ = fmt;
}

void Tw6874::SetVIFmtListener(VIFmtListener *listener)
{
    std::unique_lock<std::mutex> lock(mux_);
    listener_ = listener;
}
} // namespace rs