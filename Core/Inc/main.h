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
#include "stm32f1xx_hal.h"

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
#define LED_13_Pin GPIO_PIN_13
#define LED_13_GPIO_Port GPIOC
#define PWM_Killswitch_Pin GPIO_PIN_4
#define PWM_Killswitch_GPIO_Port GPIOA
#define PWM_Killswitch_EXTI_IRQn EXTI4_IRQn
#define PWM_Pitch_Pin GPIO_PIN_5
#define PWM_Pitch_GPIO_Port GPIOA
#define PWM_Pitch_EXTI_IRQn EXTI9_5_IRQn
#define PWM_Thrust_Pin GPIO_PIN_6
#define PWM_Thrust_GPIO_Port GPIOA
#define PWM_Thrust_EXTI_IRQn EXTI9_5_IRQn
#define PWM_Yaw_Pin GPIO_PIN_7
#define PWM_Yaw_GPIO_Port GPIOA
#define PWM_Yaw_EXTI_IRQn EXTI9_5_IRQn
#define PWM_Roll_Pin GPIO_PIN_8
#define PWM_Roll_GPIO_Port GPIOA
#define PWM_Roll_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
