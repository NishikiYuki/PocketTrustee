/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

#include "stm32l4xx_ll_crc.h"
#include "stm32l4xx_ll_dma.h"
#include "stm32l4xx_ll_rng.h"
#include "stm32l4xx_ll_rtc.h"
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_gpio.h"

#include "stm32l4xx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common_log.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KUP_Pin GPIO_PIN_13
#define KUP_GPIO_Port GPIOC
#define KUP_EXTI_IRQn EXTI15_10_IRQn
#define HF_EN1_Pin GPIO_PIN_0
#define HF_EN1_GPIO_Port GPIOC
#define HF_S2_Pin GPIO_PIN_1
#define HF_S2_GPIO_Port GPIOC
#define HF_S1_Pin GPIO_PIN_2
#define HF_S1_GPIO_Port GPIOC
#define HF_S3_Pin GPIO_PIN_3
#define HF_S3_GPIO_Port GPIOC
#define FIN_WKP_Pin GPIO_PIN_0
#define FIN_WKP_GPIO_Port GPIOA
#define LF_MOD_Pin GPIO_PIN_5
#define LF_MOD_GPIO_Port GPIOA
#define PN_PD_Pin GPIO_PIN_1
#define PN_PD_GPIO_Port GPIOB
#define DDC_Pin GPIO_PIN_14
#define DDC_GPIO_Port GPIOB
#define DRST_Pin GPIO_PIN_6
#define DRST_GPIO_Port GPIOC
#define DBUSY_Pin GPIO_PIN_7
#define DBUSY_GPIO_Port GPIOC
#define FLG_CHG_Pin GPIO_PIN_15
#define FLG_CHG_GPIO_Port GPIOA
#define FLG_CHG_EXTI_IRQn EXTI15_10_IRQn
#define FLG_USB_Pin GPIO_PIN_10
#define FLG_USB_GPIO_Port GPIOC
#define FLG_USB_EXTI_IRQn EXTI15_10_IRQn
#define FCS_Pin GPIO_PIN_6
#define FCS_GPIO_Port GPIOB
#define KDN_Pin GPIO_PIN_8
#define KDN_GPIO_Port GPIOB
#define KDN_EXTI_IRQn EXTI9_5_IRQn
#define KEN_Pin GPIO_PIN_9
#define KEN_GPIO_Port GPIOB
#define KEN_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
