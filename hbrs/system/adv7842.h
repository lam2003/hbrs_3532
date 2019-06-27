#pragma once
//stl
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
//driver
#include <adv7842.h>
//self
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

    int GetInputFormat(InputFormat &fmt);

protected:
    explicit Adv7842();

private:
    std::mutex mux_;
    InputFormat fmt_;
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> run_;
    bool init_;
};
} // namespace rs