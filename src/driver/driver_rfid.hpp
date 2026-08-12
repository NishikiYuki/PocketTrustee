#ifndef POCKETTRUSTEE_DRIVER_RFID_HPP
#define POCKETTRUSTEE_DRIVER_RFID_HPP

#include "app_ptt.hpp"
#include <functional>

class IRfidLowLayer {
public:
    enum class DriveMode {
        Stop,       // 天线关闭,DRV 接 GND
        Read,       // 125kHz 读卡(COMP1+LPTIM1 采样)
        Emulate,    // 模拟发射(TIM16 调制 LF_MOD)
    };

    virtual ~IRfidLowLayer() = default;

    /** 上电初始化:天线默认关闭。每次开机可安全调用 */
    virtual void ll_init() = 0;
    virtual void set_drive_mode(DriveMode mode) = 0;
    /** 清空采样缓冲并进入读卡模式 */
    virtual void start_sample() = 0;
    /** 采样满(128 位)回调,ISR 上下文调用 */
    virtual void register_sample_full_handler(const std::function<void()>& handler) = 0;
    /** 载入卡原始数据(64 位位序)到发射缓冲 */
    virtual void load_emulator(const uint32_t* raw_mcode) = 0;
    virtual void clear_emulator() = 0;
    /** 只读访问采样缓冲(4 words = 128 位) */
    virtual const uint32_t* sample_buffer() const = 0;
};

#endif //POCKETTRUSTEE_DRIVER_RFID_HPP
