/**
  ******************************************************************************
  * @file   bsp_scd41.c
  * @brief  SCD41 CO2 传感器驱动 (I2C1, 0x62)
  ******************************************************************************
  */
#include "bsp_scd41.h"
#include <stdio.h>

volatile SCD41_Data_t gSCD41_Val;

/* CRC-8: 多项式 0x31, 初始值 0xFF */
uint8_t SCD41_CRC8(const uint8_t *data, uint8_t len)
{
	uint8_t crc = 0xFF;
	for (uint8_t i = 0; i < len; i++)
	{
		crc ^= data[i];
		for (uint8_t b = 0; b < 8; b++)
		{
			if (crc & 0x80)
				crc = (uint8_t)(crc << 1) ^ 0x31;
			else
				crc <<= 1;
		}
	}
	return crc;
}

/* 发送 2 字节命令 */
uint8_t SCD41_SendCmd(uint16_t cmd)
{
	uint8_t buf[2];
	buf[0] = (cmd >> 8) & 0xFF;
	buf[1] = cmd & 0xFF;
	if (HAL_I2C_Master_Transmit(&hi2c1, SCD41_ADDR_WRITE, buf, 2, 100) != HAL_OK)
		return 0;
	return 1;
}

/* 发送命令并读取 wordCount 个 word（每个 word 含 2 字节数据 + 1 字节 CRC） */
uint8_t SCD41_ReadWords(uint16_t cmd, uint8_t *buf, uint8_t wordCount)
{
	if (!SCD41_SendCmd(cmd))
		return 0;

	HAL_Delay(1);

	if (HAL_I2C_Master_Receive(&hi2c1, SCD41_ADDR_READ, buf, wordCount * 3, 200) != HAL_OK)
		return 0;

	/* CRC 校验 */
	for (uint8_t i = 0; i < wordCount; i++)
	{
		uint8_t crc = SCD41_CRC8(&buf[i * 3], 2);
		if (crc != buf[i * 3 + 2])
			return 0;
	}
	return 1;
}

/* 查询数据就绪状态 */
uint8_t SCD41_GetDataReady(uint8_t *ready)
{
	uint8_t buf[3];
	if (!SCD41_ReadWords(SCD41_CMD_GET_DATA_READY, buf, 1))
		return 0;
	uint16_t status = ((uint16_t)buf[0] << 8) | buf[1];
	*ready = (status & 0x07FF) ? 1 : 0; // 低 11 位非 0 表示数据就绪
	return 1;
}

/* 启动周期性测量（5 秒间隔） */
uint8_t SCD41_StartPeriodic(void)
{
	return SCD41_SendCmd(SCD41_CMD_START_PERIODIC);
}

/* 停止周期性测量 */
uint8_t SCD41_StopPeriodic(void)
{
	return SCD41_SendCmd(SCD41_CMD_STOP_PERIODIC);
}

/* 读取一次测量结果 */
uint8_t SCD41_ReadMeasurement(SCD41_Data_t *data)
{
	uint8_t buf[9];
	if (!SCD41_ReadWords(SCD41_CMD_READ_MEASUREMENT, buf, 3))
		return 0;

	uint16_t co2_raw  = ((uint16_t)buf[0] << 8) | buf[1];
	uint16_t temp_raw = ((uint16_t)buf[3] << 8) | buf[4];
	uint16_t hum_raw  = ((uint16_t)buf[6] << 8) | buf[7];

	data->CO2         = co2_raw;
	data->Temperature = -45.0f + 175.0f * (float)temp_raw / 65536.0f;
	data->Humidity    = 100.0f * (float)hum_raw / 65536.0f;
	return 1;
}

/* 读取序列号 */
uint8_t SCD41_GetSerialNumber(uint64_t *sn)
{
	uint8_t buf[9];
	if (!SCD41_ReadWords(SCD41_CMD_GET_SERIAL_NUMBER, buf, 3))
		return 0;

	*sn =  ((uint64_t)buf[0] << 40) | ((uint64_t)buf[1] << 32)
	     | ((uint64_t)buf[3] << 24) | ((uint64_t)buf[4] << 16)
	     | ((uint64_t)buf[6] << 8)  |  (uint64_t)buf[7];
	return 1;
}

/* 初始化 SCD41 */
uint8_t SCD41_Init(void)
{
	uint8_t ret;
	/* SCD4x 上电后需等 1000ms 自动进入 idle 状态 */
	HAL_Delay(1000);
	/* 尝试启动周期性测量，最多重试3次 */
	for (int i = 0; i < 3; i++)
	{
		ret = SCD41_StartPeriodic();
		if (ret)
			return 1;
		HAL_Delay(100);
	}
	printf("[SCD41] Init failed (sensor not found)\r\n");
	return 0;
}

/* 获取最新数据（存入全局变量 gSCD41_Val） */
void SCD41_GetData(void)
{
	uint8_t ready = 0;
	if (!SCD41_GetDataReady(&ready) || !ready)
		return;

	SCD41_ReadMeasurement((SCD41_Data_t *)&gSCD41_Val);
}
