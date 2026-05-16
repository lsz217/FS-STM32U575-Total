/**
  ******************************************************************************
  * @file   bsp_max30102.c
  * @brief  MAX30102心率血氧传感器驱动 (I2C1 0x57)
  ******************************************************************************
  */
#include "bsp_max30102.h"
#include "i2c.h"
#include <stdio.h>

/*
**********************************************************************
* @fun     :BSP_MAX30102_Init
* @brief   :初始化MAX30102传感器
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MAX30102_Init(void)
{
    maxim_max30102_init();
}

/*
**********************************************************************
* @fun     :BSP_MAX30102_ReadID
* @brief   :读取MAX30102芯片ID (应为0x15)
* @param   :none
* @return  :芯片Part ID
**********************************************************************
*/
uint8_t BSP_MAX30102_ReadID(void)
{
    uint8_t id = 0;
    maxim_max30102_read_reg(REG_PART_ID, &id);
    return id;
}

/*
**********************************************************************
* @fun     :BSP_MAX30102_ReadFIFO
* @brief   :从FIFO读取一对红光+红外采样值
* @param   :pRed - 红光数据, pIR - 红外数据
* @return  :1=读取成功, 0=FIFO无新数据
**********************************************************************
*/
uint8_t BSP_MAX30102_ReadFIFO(uint32_t *pRed, uint32_t *pIR)
{
    uint8_t status = 0;
    maxim_max30102_read_reg(REG_INTR_STATUS_1, &status);
    if (status & 0xC0)
    {
        maxim_max30102_read_fifo(pRed, pIR);
        return 1;
    }
    return 0;
}

/*
**********************************************************************
* @fun     :BSP_MAX30102_ReadTemperature
* @brief   :读取MAX30102芯片温度（体表参考温度）
* @param   :pTemp - 温度值 (C)
* @return  :none
**********************************************************************
*/
void BSP_MAX30102_ReadTemperature(float *pTemp)
{
    int8_t  temp_int = 0;
    uint8_t temp_frac = 0;
    maxim_max30102_read_temperature(&temp_int, &temp_frac);
    *pTemp = (float)temp_int + ((float)temp_frac) / 16.0f;
}

/*
**********************************************************************
* @fun     :BSP_MAX30102_Reset
* @brief   :复位MAX30102传感器
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MAX30102_Reset(void)
{
    maxim_max30102_reset();
}

/*
**********************************************************************
* @fun     :BSP_MAX30102_Test
* @brief   :MAX30102阻塞测试（初始化→读ID→读温度→读FIFO采样）
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MAX30102_Test(void)
{
    printf("[MAX30102] === Sensor Test ===\r\n");

    printf("[MAX30102] Initializing...\r\n");
    if (!maxim_max30102_init())
    {
        printf("[MAX30102] ERROR: Init failed!\r\n");
        return;
    }

    uint8_t id = BSP_MAX30102_ReadID();
    printf("[MAX30102] Part ID: 0x%02X (expected 0x15)\r\n", id);

    float temp;
    BSP_MAX30102_ReadTemperature(&temp);
    printf("[MAX30102] Temperature: %.2f C\r\n", temp);

    printf("[MAX30102] Reading FIFO samples for 5s...\r\n");
    for (int i = 0; i < 250; i++)
    {
        uint32_t red = 0, ir = 0;
        if (BSP_MAX30102_ReadFIFO(&red, &ir))
        {
            printf("[MAX30102] RED=%5u  IR=%5u\r\n", (unsigned)red, (unsigned)ir);
        }
        HAL_Delay(20);
    }

    printf("[MAX30102] Test done.\r\n");
}
