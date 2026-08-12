#include "ll_rfid.hpp"
#include "comp.h"
#include "dac.h"
#include "lptim.h"
#include "tim.h"

RfidLowLayer& RfidLowLayer::getInstance() {
    static RfidLowLayer instance;
    return instance;
}

namespace {
    constexpr LPTIM_HandleTypeDef* LPTimer = &hlptim2;
    
    // 基于 4us tick 的周期判定
    constexpr uint32_t kHalfCycMin = 57;
    constexpr uint32_t kHalfCycMax = 90;
    constexpr uint32_t kFullCycMin = 120;
    constexpr uint32_t kFullCycMax = 136;
}

__STATIC_INLINE void switch_stop_mode()
{
    HAL_TIM_OC_Stop(&htim1, TIM_CHANNEL_1);
    HAL_DAC_Stop(&hdac1, DAC_CHANNEL_1);
    HAL_COMP_Stop(&hcomp1);
    HAL_LPTIM_Counter_Stop(LPTimer);

    HAL_TIM_OC_DeInit(&htim1);
    HAL_TIM_Base_DeInit(&htim1);
    HAL_LPTIM_DeInit(LPTimer);
    HAL_COMP_DeInit(&hcomp1);
    HAL_DAC_DeInit(&hdac1);

    // Connect DRV to GND
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}

__STATIC_INLINE void switch_read_mode()
{
    MX_TIM1_Init();
    MX_DAC1_Init();
    MX_COMP1_Init();
    MX_LPTIM1_Init();

    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R, 2);
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
    HAL_LPTIM_Counter_Start(LPTimer, 0xFFFF);
    HAL_TIM_Base_Start(&htim1);
    HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_1);
    HAL_COMP_Start(&hcomp1);
}

void HAL_COMP_TriggerCallback(COMP_HandleTypeDef* hcomp)
{
    if (hcomp == &hcomp1)
    {
        RfidLowLayer::getInstance().on_comp1_trigger();
    }
}

void RfidLowLayer::ll_init()
{
    // 上电默认:天线关闭(DRV 接 GND)。此时外设未初始化,stop 模式的处理对 RESET 状态安全
    set_drive_mode(DriveMode::Stop);
}

void RfidLowLayer::set_drive_mode(DriveMode mode)
{
    LF_MOD_GPIO_Port->BRR = LF_MOD_Pin;
    if (mode == DriveMode::Stop || mode == DriveMode::Emulate)
    {
        // 关闭不必要的读写外设
        // HAL_TIM_Base_Stop_IT(&htim16);
        switch_stop_mode();
        // TIM16 256us 逐位调制
        if (mode == DriveMode::Emulate)
        {
            // HAL_TIM_Base_Start_IT(&htim16);
        }
    }
    else
    {
        switch_read_mode();
    }
}

void RfidLowLayer::start_sample()
{
    sample_data[0] = 0;
    sample_data[1] = 0;
    sample_data[2] = 0;
    sample_data[3] = 0;
    sample_index = 0;
    sample_clock = 0;
    in_sample = true;
    set_drive_mode(DriveMode::Read);
}

void RfidLowLayer::register_sample_full_handler(const std::function<void()>& handler)
{
    sample_full_handler_ = handler;
}

void RfidLowLayer::load_emulator(const uint32_t* raw_mcode)
{
    emulation_circulate_buffer[0] = 0;
    emulation_circulate_buffer[1] = 0;
    emulation_circulate_buffer[2] = 0;
    emulation_circulate_buffer[3] = 0;
    for (uint8_t i = 0; i < 64; i++)
    {
        // bit1 -> (mcode)10 -> (drive)01
        // bit0 -> (mcode)01 -> (drive)10
        const uint32_t bit = (raw_mcode[i / 32] & (1U << (i % 32))) ? 1 : 2;
        emulation_circulate_buffer[i / 16] |= bit << (i * 2) % 32;
    }
}

void RfidLowLayer::clear_emulator()
{
    emulation_circulate_buffer[0] = 0;
    emulation_circulate_buffer[1] = 0;
    emulation_circulate_buffer[2] = 0;
    emulation_circulate_buffer[3] = 0;
}

const uint32_t* RfidLowLayer::sample_buffer() const
{
    return sample_data;
}

void RfidLowLayer::on_comp1_trigger()
{
    if (!in_sample) return;

    uint32_t now = hlptim1.Instance->CNT;
    uint32_t delay = (now < sample_clock) ? now + (0xFFFF - sample_clock) : now - sample_clock;
    if (delay >= kFullCycMin && delay <= kFullCycMax)
    {
        // 完整周期:采样一位
        sample_clock = hlptim1.Instance->CNT;
        const uint32_t status = HAL_COMP_GetOutputLevel(&hcomp1) == COMP_OUTPUT_LEVEL_HIGH ? 0 : 1;
        sample_data[sample_index / 32] |= status << (sample_index % 32);
        if (sample_index >= 127)
        {
            in_sample = false;
            set_drive_mode(DriveMode::Stop);
            if (sample_full_handler_)
            {
                sample_full_handler_();
            }
        }
        else
        {
            sample_index++;
        }
    }
    else if (delay >= kHalfCycMin && delay <= kHalfCycMax)
    {
        // 半周期毛刺,丢弃
    }
    else
    {
        // 周期异常,视为干扰:重置采样指针
        sample_clock = hlptim1.Instance->CNT;
        sample_index = 0;
        sample_data[0] = 0;
        sample_data[1] = 0;
        sample_data[2] = 0;
        sample_data[3] = 0;
    }
}

void RfidLowLayer::on_timer16_elapsed()
{
    if (emulation_circulate_buffer[circulate_buffer_index / 32] & (1U << (circulate_buffer_index % 32)))
        LF_MOD_GPIO_Port->BSRR = LF_MOD_Pin;
    else
        LF_MOD_GPIO_Port->BRR = LF_MOD_Pin;
    circulate_buffer_index++;
    if (circulate_buffer_index >= 128) circulate_buffer_index = 0;
}
