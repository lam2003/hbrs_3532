#pragma once

namespace rs
{
template <typename ParamsT>
class Module
{
public:
    virtual ~Module() {}
    virtual int32_t Initialize(const ParamsT &) = 0;
    virtual void Close() = 0;
};
}; // namespace rs