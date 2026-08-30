#include "ll_clock.hpp"
#include "lptim.h"
#include "FreeRTOS.h"
#include "task.h"

static constexpr LPTIM_HandleTypeDef* _LPTimer = &hlptim1;
static constexpr float _LPTimer_Clock = 32768.0f;
static constexpr float _LPTimer_Prescaler = 128.0f;
static constexpr float _LPTimer_Timebase = 1.0f / (_LPTimer_Clock / _LPTimer_Prescaler);
static constexpr float _LPTimer_1msStansFor = 1.0f / _LPTimer_Timebase * 1e-3f;
static constexpr uint32_t _LPTimer_MaximunMiliseconds = static_cast<uint32_t>(0xFFFF * _LPTimer_Timebase * 1e3f);

namespace {
    __IO uint32_t lptimer_tick_last;
    // __IO CoreClock::CoreSleep mode_ = CoreClock::CoreSleep::WfiOnly;

    void IdealTask(void*) {
        for (;;) {
            vTaskDelay(0);
        }
    }

    StackType_t IdealTaskStack[128];
    StaticTask_t IdealTaskTaskTCB;
    TaskHandle_t IdealTaskHandle = nullptr;
}

void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef* hlptim) {
    if (hlptim == _LPTimer) {
        HAL_LPTIM_TimeOut_Stop_IT(_LPTimer);
    }
}

void CoreClock::SetSleepMode(CoreSleep mode) {
    // mode_ = mode;
    // having an ideal task is way effective than a condition branch
    // who know why :)
    if (mode == CoreSleep::WfiOnly && !IdealTaskHandle) {
        IdealTaskHandle = xTaskCreateStatic(
            IdealTask, "ideal", 128,
            nullptr, 1, IdealTaskStack, &IdealTaskTaskTCB
        );
    }
    else if (mode == CoreSleep::AllowStop2 && IdealTaskHandle) {
        vTaskDelete(IdealTaskHandle);
        IdealTaskHandle = nullptr;
    }
}

extern "C" {
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime) {
    if (eTaskConfirmSleepModeStatus() != eAbortSleep) {
        DEBUG_INFO("Sleep For %lu", xExpectedIdleTime);

        if (xExpectedIdleTime < _LPTimer_MaximunMiliseconds - 50) {
            auto count = static_cast<uint16_t>(static_cast<float>(xExpectedIdleTime) * _LPTimer_1msStansFor);

            lptimer_tick_last = static_cast<uint32_t>(static_cast<float>(count) * _LPTimer_Timebase * 1e3f);

            HAL_LPTIM_TimeOut_Start_IT(_LPTimer, 0xFFFF, count);
        }
        else {
            // sleep for long enough time and timer is not needed
            lptimer_tick_last = 0;
        }

        __asm volatile( "cpsid i" ::: "memory" );
        __asm volatile( "dsb" );
        __asm volatile( "isb" );

        HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);

        CoreClock::SystemClockConfig(CoreClock::CoreSpeedHigh);

        /* Re-enable interrupts to allow the interrupt that brought the MCU
        out of sleep mode to execute immediately.  see comments above
        __disable_interrupt() call above. */
        __asm volatile( "cpsie i" ::: "memory" );
        __asm volatile( "dsb" );
        __asm volatile( "isb" );

        /* Disable interrupts again because the clock is about to be stopped
        and interrupts that execute while the clock is stopped will increase
        any slippage between the time maintained by the RTOS and calendar
        time. */
        __asm volatile( "cpsid i" ::: "memory" );
        __asm volatile( "dsb" );
        __asm volatile( "isb" );

        vTaskStepTick(lptimer_tick_last);
        uwTick += lptimer_tick_last;

        /* Exit with interrupts enabled. */
        __asm volatile( "cpsie i" ::: "memory" );
    }
}
}
