#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

#include <adv7842.h>

#include "common/global.h"

namespace rs
{

class Adv7842
{
public:
    virtual ~Adv7842();

    static Adv7842 *Instance();

    int Initialize(ADV7842_CMODE_E mode);

    void Close();

protected:
    explicit Adv7842();

private:
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> run_;
    std::atomic<bool> has_signal_;
    bool init_;
};
} // namespace rs