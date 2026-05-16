/**
  ******************************************************************************
  * @file   bsp_mq2.h
  * @brief  MQ-2烟雾传感器驱动
  ******************************************************************************
  */
#ifndef __BSP_MQ2_H__
#define __BSP_MQ2_H__

#include "main.h"

#define MQ2_GPIO_PORT       GPIOB
#define MQ2_GPIO_PIN        GPIO_PIN_8
#define MQ2_PREHEAT_SEC     120

void BSP_MQ2_Init(void);
uint8_t BSP_MQ2_Read(void);
void BSP_MQ2_Test(void);

#endif /* __BSP_MQ2_H__ */
