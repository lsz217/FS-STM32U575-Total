/**
  ******************************************************************************
  * @file   bsp_motor.h
  * @brief  震动马达驱动 (PC7)
  ******************************************************************************
  */
#ifndef __BSP_MOTOR_H__
#define __BSP_MOTOR_H__

#include "main.h"

#define MOTOR_GPIO_PORT     GPIOC
#define MOTOR_GPIO_PIN      GPIO_PIN_7

void BSP_MOTOR_Init(void);
void BSP_MOTOR_On(void);
void BSP_MOTOR_Off(void);
void BSP_MOTOR_Toggle(void);
void BSP_MOTOR_Test(void);

#endif /* __BSP_MOTOR_H__ */
