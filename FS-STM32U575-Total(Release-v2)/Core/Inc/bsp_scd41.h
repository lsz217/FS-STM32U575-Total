#ifndef __BSP_SCD41_H__
#define __BSP_SCD41_H__

#include "main.h"
#include "i2c.h"

#define SCD41_I2C_ADDR    0x62
#define SCD41_ADDR_WRITE  (SCD41_I2C_ADDR << 1)
#define SCD41_ADDR_READ   ((SCD41_I2C_ADDR << 1) | 0x01)

/* SCD41 I2C 命令 */
#define SCD41_CMD_START_PERIODIC     0x21B1
#define SCD41_CMD_STOP_PERIODIC      0x3F86
#define SCD41_CMD_READ_MEASUREMENT   0xEC05
#define SCD41_CMD_GET_DATA_READY     0xE4B8
#define SCD41_CMD_GET_SERIAL_NUMBER  0x3682
#define SCD41_CMD_PERFORM_SELF_TEST  0x3639
#define SCD41_CMD_START_LOW_POWER    0x21AC
#define SCD41_CMD_REINIT             0x3646
#define SCD41_CMD_SLEEP              0x36E0
#define SCD41_CMD_WAKE_UP            0x36F6
#define SCD41_CMD_SET_TEMP_OFFSET    0x241D
#define SCD41_CMD_GET_TEMP_OFFSET    0x2318
#define SCD41_CMD_SET_ALTITUDE       0x2427
#define SCD41_CMD_GET_ALTITUDE       0x2322
#define SCD41_CMD_SET_PRESSURE       0xE000
#define SCD41_CMD_FACTORY_RESET      0x3632
#define SCD41_CMD_PERSIST_SETTINGS   0x3615

/* 时序 */
#define SCD41_POWERUP_MS    1000
#define SCD41_MEAS_INTERVAL_MS  5000

/* 数据结构 */
typedef struct {
	uint16_t CO2;           // ppm
	float    Temperature;   // °C
	float    Humidity;      // %RH
} SCD41_Data_t;

/* 函数声明 */
uint8_t SCD41_CRC8(const uint8_t *data, uint8_t len);
uint8_t SCD41_SendCmd(uint16_t cmd);
uint8_t SCD41_ReadWords(uint16_t cmd, uint8_t *buf, uint8_t wordCount);
uint8_t SCD41_StartPeriodic(void);
uint8_t SCD41_StopPeriodic(void);
uint8_t SCD41_GetDataReady(uint8_t *ready);
uint8_t SCD41_ReadMeasurement(SCD41_Data_t *data);
uint8_t SCD41_GetSerialNumber(uint64_t *sn);
uint8_t SCD41_Init(void);
void    SCD41_GetData(void);

extern volatile SCD41_Data_t gSCD41_Val;

#endif
