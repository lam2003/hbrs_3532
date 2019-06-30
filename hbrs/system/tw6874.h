#pragma once

//self
#include "global.h"
#include "system/pciv_comm.h"

namespace rs
{
class Tw6874
{
public:
    static Tw6874 *Instance();

    virtual ~Tw6874();

    int Initialize();

    void CLose();

    void UpdateQuery();
protected:
    explicit Tw6874();

private:
};
} // namespace rs