#ifndef POCKETTRUSTEE_APP_PTT_HPP
#define POCKETTRUSTEE_APP_PTT_HPP

#include "app_ptt_utility.hpp"

namespace PocketTrustee {
    class IDevice {
    public:
        virtual ~IDevice() = default;
        virtual void Initialize() = 0;
        virtual std::string_view Name() const noexcept = 0;
    };
}

/** 通用命令返回结构:err=业务错误码, err_fs=文件系统错误码, msg=错误描述 */
struct PKT_ERR
{
    int err;
    int err_fs;
    const char* msg;
};

/** 设备固件版本(hello 命令上报),由 app 层持有 */
constexpr uint8_t version_c1 = 1;
constexpr uint8_t version_c2 = 1;
constexpr uint8_t version_c3 = 0;

#endif //POCKETTRUSTEE_APP_PTT_HPP
