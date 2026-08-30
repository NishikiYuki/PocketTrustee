/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "comp.h"
#include "crc.h"
#include "dac.h"
#include "dma.h"
#include "lptim.h"
#include "rng.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void MPU_Config();
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_COMP1_Init();
  MX_DAC1_Init();
  MX_RNG_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_USB_PCD_Init();
  MX_USART3_UART_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_LPTIM2_Init();
  MX_LPTIM1_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */

  __attribute__((section("._user_heap2_region"))) static uint8_t _userHeapRegion1[ SRAM2_SIZE ];
  static uint8_t _userHeapRegion2[ 0x5000 ];
  static const HeapRegion_t xHeapRegions[] =
  {
    {_userHeapRegion1, SRAM2_SIZE},
    {_userHeapRegion2, sizeof(_userHeapRegion2)},
    { NULL, 0 }
  };
  vPortDefineHeapRegions(xHeapRegions);
  // MPU_Config();

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV8;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */
void MPU_Config()
{
  if (MPU->TYPE == 0) return;
  int nRegion = MPU->TYPE >> 8 & 0xFF;
  // Disable executable for SRAM1 and SRAM2
  MPU_Region_InitTypeDef configs[] = {
      {
          .Enable = MPU_REGION_ENABLE,
          .BaseAddress = SRAM1_BASE,
          .Size = MPU_REGION_SIZE_64KB,
          .SubRegionDisable = 0,
          .TypeExtField = MPU_TEX_LEVEL0,
          .AccessPermission = MPU_REGION_FULL_ACCESS,
          .DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE,
          .IsShareable = MPU_ACCESS_SHAREABLE,
          .IsCacheable = MPU_ACCESS_CACHEABLE,
          .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
      },
    {
          .Enable = MPU_REGION_ENABLE,
          .BaseAddress = SRAM2_BASE,
          .Size = MPU_REGION_SIZE_16KB,
          .SubRegionDisable = 0,
          .TypeExtField = MPU_TEX_LEVEL0,
          .AccessPermission = MPU_REGION_FULL_ACCESS,
          .DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE,
          .IsShareable = MPU_ACCESS_SHAREABLE,
          .IsCacheable = MPU_ACCESS_CACHEABLE,
          .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
        },
      {
          .Enable = MPU_REGION_ENABLE,
          .BaseAddress = FLASH_BASE,
          .Size = MPU_REGION_SIZE_256KB,
          .SubRegionDisable = 0,
          .TypeExtField = MPU_TEX_LEVEL0,
          .AccessPermission = MPU_REGION_FULL_ACCESS,
          .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
          .IsShareable = MPU_ACCESS_NOT_SHAREABLE,
          .IsCacheable = MPU_ACCESS_CACHEABLE,
          .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
      },
      {
          .Enable = MPU_REGION_ENABLE,
          .BaseAddress = PERIPH_BASE,
          .Size = MPU_REGION_SIZE_512MB,
          .SubRegionDisable = 0,
          .TypeExtField = MPU_TEX_LEVEL0,
          .AccessPermission = MPU_REGION_FULL_ACCESS,
          .DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE,
          .IsShareable = MPU_ACCESS_SHAREABLE,
          .IsCacheable = MPU_ACCESS_NOT_CACHEABLE,
          .IsBufferable = MPU_ACCESS_BUFFERABLE,
      }
  };
  HAL_MPU_Disable();
  for (int i = 0; i < nRegion; i++) {
    if (i < sizeof(configs) / sizeof(configs[0])) {
      configs[i].Number = i;
      HAL_MPU_ConfigRegion(configs + i);
    } else {
      MPU->RNR = i;
      MPU->RBAR = 0;
      MPU->RASR = 0;
    }
  }
  SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
  SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
  NVIC_SetPriority(MemoryManagement_IRQn, -1);
  NVIC_SetPriority(BusFault_IRQn, -1);

  HAL_MPU_Enable(0);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
