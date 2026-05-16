/**
  ******************************************************************************
  * @file   bsp_max30102.h
  * @brief  MAX30102心率血氧传感器驱动 (I2C1 0x57)
  ******************************************************************************
  */
#ifndef __BSP_MAX30102_H__
#define __BSP_MAX30102_H__

#include "main.h"
#include "max30102.h"

#define MAX30102_I2C_ADDR       0x57
#define MAX30102_FIFO_DEPTH     32

typedef struct
{
    int32_t heart_rate;      // 心率 (bpm)
    int32_t spo2;            // 血氧 (%)
    float   temperature;     // 体表温度 (C)
    int8_t  hr_valid;        // 心率有效
    int8_t  spo2_valid;      // 血氧有效
} MAX30102_Data_t;

void     BSP_MAX30102_Init(void);
uint8_t  BSP_MAX30102_ReadID(void);
uint8_t  BSP_MAX30102_ReadFIFO(uint32_t *pRed, uint32_t *pIR);
void     BSP_MAX30102_ReadTemperature(float *pTemp);
void     BSP_MAX30102_Reset(void);
void     BSP_MAX30102_Test(void);

#endif /* __BSP_MAX30102_H__ */
