/**
  ******************************************************************************
  * @file   bsp_mq3.h
  * @brief  MQ-3酒精传感器驱动
  ******************************************************************************
  */
#ifndef __BSP_MQ3_H__
#define __BSP_MQ3_H__

#include "main.h"

#define MQ3_GPIO_PORT       GPIOB
#define MQ3_GPIO_PIN        GPIO_PIN_9
#define MQ3_PREHEAT_SEC     120

void BSP_MQ3_Init(void);
uint8_t BSP_MQ3_Read(void);
void BSP_MQ3_Test(void);

#endif /* __BSP_MQ3_H__ */
