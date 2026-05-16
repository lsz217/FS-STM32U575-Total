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
#include "stm32u5xx_hal.h"

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
#define BLUE_LED_Pin GPIO_PIN_13
#define BLUE_LED_GPIO_Port GPIOC
#define LCD_DCX_Pin GPIO_PIN_4
#define LCD_DCX_GPIO_Port GPIOA
#define PWR_EN_Pin GPIO_PIN_5
#define PWR_EN_GPIO_Port GPIOC
#define ADC_AMP_IN15_Pin GPIO_PIN_0
#define ADC_AMP_IN15_GPIO_Port GPIOB
#define ADC_VDC_IN16_Pin GPIO_PIN_1
#define ADC_VDC_IN16_GPIO_Port GPIOB
#define EXT_ITR_Pin GPIO_PIN_2
#define EXT_ITR_GPIO_Port GPIOB
#define EXT_ITR_EXTI_IRQn EXTI2_IRQn
#define HC595_RCLK_Pin GPIO_PIN_12
#define HC595_RCLK_GPIO_Port GPIOB
#define FAN_Pin GPIO_PIN_6
#define FAN_GPIO_Port GPIOC
#define EXT_MOTOR_Pin GPIO_PIN_7
#define EXT_MOTOR_GPIO_Port GPIOC
#define KEY_Pin GPIO_PIN_8
#define KEY_GPIO_Port GPIOC
#define SCREENKEY_Pin GPIO_PIN_9
#define SCREENKEY_GPIO_Port GPIOC
#define SCREENKEY_EXTI_IRQn EXTI9_IRQn
#define LCD_RST_Pin GPIO_PIN_8
#define LCD_RST_GPIO_Port GPIOA
#define TP_RST_Pin GPIO_PIN_11
#define TP_RST_GPIO_Port GPIOA
#define USER_KEY_Pin GPIO_PIN_12
#define USER_KEY_GPIO_Port GPIOA
#define USER_KEY_EXTI_IRQn EXTI12_IRQn
#define RUN_BEEP_Pin GPIO_PIN_15
#define RUN_BEEP_GPIO_Port GPIOA
#define EXT_FIRE_Pin GPIO_PIN_3
#define EXT_FIRE_GPIO_Port GPIOB
#define EXT_FIRE_EXTI_IRQn EXTI3_IRQn
#define EXT_PIR_Pin GPIO_PIN_4
#define EXT_PIR_GPIO_Port GPIOB
#define EXT_PIR_EXTI_IRQn EXTI4_IRQn
#define TP_INT_Pin GPIO_PIN_5
#define TP_INT_GPIO_Port GPIOB
#define TP_INT_EXTI_IRQn EXTI5_IRQn

/* USER CODE BEGIN Private defines */
#define NFC_BUF_SIZE 256
#define BEEP_ENABLE()  HAL_GPIO_WritePin(RUN_BEEP_GPIO_Port, RUN_BEEP_Pin, GPIO_PIN_SET)
#define BEEP_DISABLE() HAL_GPIO_WritePin(RUN_BEEP_GPIO_Port, RUN_BEEP_Pin, GPIO_PIN_RESET)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
