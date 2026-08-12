#include "ll_rtc.hpp"
#include "app_ptt_utility.hpp"
#include "rtc.h"

using namespace PocketTrustee;

RtcLowLayer& RtcLowLayer::getInstance() {
    static RtcLowLayer instance;
    return instance;
}

void RtcLowLayer::ll_init() {
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
    LL_RCC_EnableRTC();
}

void RtcLowLayer::ll_reset() {
    LL_RTC_InitTypeDef RTC_InitStruct = {0};
    RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
    RTC_InitStruct.AsynchPrescaler = 127;
    RTC_InitStruct.SynchPrescaler = 255;
    LL_RTC_Init(RTC, &RTC_InitStruct);
}

int RtcLowLayer::write_backup_ram(uint8_t offset, int value) {
    if (offset > LL_RTC_BKP_DR31) {
        return -1;
    }
    HAL_PWR_EnableBkUpAccess();
    LL_RTC_BAK_SetRegister(RTC, offset, value);
    HAL_PWR_DisableBkUpAccess();
    return 0;
}

int RtcLowLayer::read_backup_ram(uint8_t offset, int* p_value) {
    if (offset > LL_RTC_BKP_DR31) {
        return -1;
    }
    HAL_PWR_EnableBkUpAccess();
    *p_value = (int)LL_RTC_BAK_GetRegister(RTC, offset);
    HAL_PWR_DisableBkUpAccess();
    return 0;
}

void RtcLowLayer::set_time(DateTime dt) {
    LL_RTC_TimeTypeDef RTC_TimeStruct = {
        .TimeFormat = LL_RTC_TIME_FORMAT_AM_OR_24,
        .Hours = dt.hour,
        .Minutes = dt.minute,
        .Seconds = dt.second,
    };
    LL_RTC_DateTypeDef RTC_DateStruct = {
        .WeekDay = LL_RTC_WEEKDAY_MONDAY,
        .Month = dt.month,
        .Day = dt.day,
        .Year = static_cast<uint8_t>(dt.year - 2000),
    };
    HAL_PWR_EnableBkUpAccess();
    LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BIN, &RTC_TimeStruct);
    LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BIN, &RTC_DateStruct);
    HAL_PWR_DisableBkUpAccess();
}

DateTime RtcLowLayer::get_time() {
    DateTime dt {};
    dt.year = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC)) + 2000;
    dt.month = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetMonth(RTC));
    dt.day = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetDay(RTC));
    dt.hour = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
    dt.minute = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
    dt.second = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));
    return dt;
}
