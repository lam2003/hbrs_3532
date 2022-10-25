#pragma once

//self
#include "global.h"

enum err_code
{
    KSuccess = 0,
    KInitialized = -1,
    KSDKError = -2,
    KUnInitialized = -3,
    KParamsError = -4,
    KSystemError = -5,
    KNotEnoughBuf = -6,
    KTimeout = -7
};

class RSErrorCategory : public std::error_category
{
    const char *name() const noexcept override
    {
        return "rs";
    }
    std::string message(int32_t i) const override
    {
        switch (static_cast<err_code>(i))
        {
        case err_code::KSuccess:
            return "success";
        case err_code::KInitialized:
            return "duplicate initialize";
        case err_code::KSDKError:
            return "hisi sdk error";
        case err_code::KUnInitialized:
            return "uninitialized";
        case err_code::KParamsError:
            return "params error";
        case err_code::KSystemError:
            return "system error";
        case err_code::KNotEnoughBuf:
            return "not enough buffer";
        case err_code::KTimeout:
            return "timeout";
        default:
            return "unknow error";
        }
    }
};

namespace std
{
template <>
struct is_error_code_enum<err_code> : std::true_type
{
};
} // namespace std

template <class T>
struct is_error_code_enum : std::false_type
{
};

static RSErrorCategory rs_error_category;

inline std::error_code
make_error_code(err_code code) noexcept
{
    return {static_cast<int32_t>(code), rs_error_category};
}