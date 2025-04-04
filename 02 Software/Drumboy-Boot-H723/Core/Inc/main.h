/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define CT2_D0_Pin GPIO_PIN_2
#define CT2_D0_GPIO_Port GPIOE
#define CT2_D0_EXTI_IRQn EXTI2_IRQn
#define CT2_D2_Pin GPIO_PIN_4
#define CT2_D2_GPIO_Port GPIOE
#define CT2_D2_EXTI_IRQn EXTI4_IRQn
#define CT2_D3_Pin GPIO_PIN_5
#define CT2_D3_GPIO_Port GPIOE
#define CT2_D3_EXTI_IRQn EXTI9_5_IRQn
#define CT2_D4_Pin GPIO_PIN_6
#define CT2_D4_GPIO_Port GPIOE
#define CT2_D4_EXTI_IRQn EXTI9_5_IRQn
#define SDMMC2_DETECT_Pin GPIO_PIN_13
#define SDMMC2_DETECT_GPIO_Port GPIOC
#define SYNC_IN_RX_Pin GPIO_PIN_6
#define SYNC_IN_RX_GPIO_Port GPIOF
#define SYNC_IN_TX_Pin GPIO_PIN_7
#define SYNC_IN_TX_GPIO_Port GPIOF
#define SYNC_IN_Pin GPIO_PIN_8
#define SYNC_IN_GPIO_Port GPIOF
#define SYNC_IN_EXTI_IRQn EXTI9_5_IRQn
#define BUTTON_Pin GPIO_PIN_10
#define BUTTON_GPIO_Port GPIOF
#define BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define CT0_SCL_Pin GPIO_PIN_0
#define CT0_SCL_GPIO_Port GPIOA
#define CT0_SDO_Pin GPIO_PIN_1
#define CT0_SDO_GPIO_Port GPIOA
#define CT0_SDO_EXTI_IRQn EXTI1_IRQn
#define CT1_SCL_Pin GPIO_PIN_2
#define CT1_SCL_GPIO_Port GPIOA
#define CT1_SDO_Pin GPIO_PIN_3
#define CT1_SDO_GPIO_Port GPIOA
#define CT1_SDO_EXTI_IRQn EXTI3_IRQn
#define CT2_D1_Pin GPIO_PIN_7
#define CT2_D1_GPIO_Port GPIOA
#define CT2_D1_EXTI_IRQn EXTI9_5_IRQn
#define LCD_D0_Pin GPIO_PIN_0
#define LCD_D0_GPIO_Port GPIOB
#define LCD_D1_Pin GPIO_PIN_1
#define LCD_D1_GPIO_Port GPIOB
#define LCD_D2_Pin GPIO_PIN_2
#define LCD_D2_GPIO_Port GPIOB
#define LCD_D10_Pin GPIO_PIN_10
#define LCD_D10_GPIO_Port GPIOB
#define LCD_D11_Pin GPIO_PIN_11
#define LCD_D11_GPIO_Port GPIOB
#define LCD_D12_Pin GPIO_PIN_12
#define LCD_D12_GPIO_Port GPIOB
#define LCD_D13_Pin GPIO_PIN_13
#define LCD_D13_GPIO_Port GPIOB
#define LCD_D14_Pin GPIO_PIN_14
#define LCD_D14_GPIO_Port GPIOB
#define LCD_D15_Pin GPIO_PIN_15
#define LCD_D15_GPIO_Port GPIOB
#define LED0_Pin GPIO_PIN_11
#define LED0_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_12
#define LED1_GPIO_Port GPIOD
#define LED2_Pin GPIO_PIN_13
#define LED2_GPIO_Port GPIOD
#define MIDI_TX_Pin GPIO_PIN_6
#define MIDI_TX_GPIO_Port GPIOC
#define MIDI_RX_Pin GPIO_PIN_9
#define MIDI_RX_GPIO_Port GPIOA
#define SYNC_OUT_Pin GPIO_PIN_10
#define SYNC_OUT_GPIO_Port GPIOA
#define SYNC_OUT_RX_Pin GPIO_PIN_11
#define SYNC_OUT_RX_GPIO_Port GPIOA
#define SYNC_OUT_TX_Pin GPIO_PIN_12
#define SYNC_OUT_TX_GPIO_Port GPIOA
#define LCD_CS_Pin GPIO_PIN_2
#define LCD_CS_GPIO_Port GPIOD
#define LCD_RD_Pin GPIO_PIN_3
#define LCD_RD_GPIO_Port GPIOD
#define LCD_WR_Pin GPIO_PIN_4
#define LCD_WR_GPIO_Port GPIOD
#define LCD_RS_Pin GPIO_PIN_5
#define LCD_RS_GPIO_Port GPIOD
#define LCD_RESET_Pin GPIO_PIN_13
#define LCD_RESET_GPIO_Port GPIOG
#define LCD_BL_Pin GPIO_PIN_14
#define LCD_BL_GPIO_Port GPIOG
#define LCD_D3_Pin GPIO_PIN_3
#define LCD_D3_GPIO_Port GPIOB
#define LCD_D4_Pin GPIO_PIN_4
#define LCD_D4_GPIO_Port GPIOB
#define LCD_D5_Pin GPIO_PIN_5
#define LCD_D5_GPIO_Port GPIOB
#define LCD_D6_Pin GPIO_PIN_6
#define LCD_D6_GPIO_Port GPIOB
#define LCD_D7_Pin GPIO_PIN_7
#define LCD_D7_GPIO_Port GPIOB
#define LCD_D8_Pin GPIO_PIN_8
#define LCD_D8_GPIO_Port GPIOB
#define LCD_D9_Pin GPIO_PIN_9
#define LCD_D9_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
