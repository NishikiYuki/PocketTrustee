#ifndef POCKETTRUSTEE_LL_RFID_HPP
#define POCKETTRUSTEE_LL_RFID_HPP

#include "main.h"
#include "driver_rfid.hpp"
#include <functional>

class RfidLowLayer final : public IRfidLowLayer {
public:
    static RfidLowLayer& getInstance();

    void ll_init() override;
    void set_drive_mode(DriveMode mode) override;
    void start_sample() override;
    void register_sample_full_handler(const std::function<void()>& handler) override;
    void load_emulator(const uint32_t* raw_mcode) override;
    void clear_emulator() override;
    const uint32_t* sample_buffer() const override;

    /** COMP1 边沿触发(ISR):125kHz 读卡采样 */
    void on_comp1_trigger();
    /** TIM16 溢出(ISR):模拟发射逐位调制 */
    void on_timer16_elapsed();

private:
    RfidLowLayer() = default;

    std::function<void()> sample_full_handler_;

    // 模拟发射缓冲:每卡位转 2 个调制位(bit1->01, bit0->10),共 128 位
    uint32_t emulation_circulate_buffer[4] {};
    uint8_t circulate_buffer_index = 0;

    // 读卡采样状态(ISR 与主线程共享)
    volatile bool in_sample = false;
    uint8_t sample_index = 0;
    uint32_t sample_clock = 0;
    uint32_t sample_data[4] {};
};

#endif //POCKETTRUSTEE_LL_RFID_HPP
