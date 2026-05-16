/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "mpu6050.h"
#include "i2c.h"
/* Private function prototypes -----------------------------------------------*/

int16_t Accx,Accy,Accz;
//**************************************
//
//**************************************
void InitMPU6050(void)
{
	I2c_Write_one_Byte( 0x00,PWR_MGMT_1, SlaveAddress);	//
	I2c_Write_one_Byte( 0x07,SMPLRT_DIV,SlaveAddress);
	I2c_Write_one_Byte( 0x06,CONFIG,SlaveAddress);
	I2c_Write_one_Byte( 0x18,GYRO_CONFIG,SlaveAddress);
	I2c_Write_one_Byte( 0x01,ACCEL_CONFIG,SlaveAddress);
  HAL_Delay(10);
  mpu6050_verify(&Accx, &Accy, &Accz); //
}
void  mpu6050_verify(int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t H,L;
	I2c_Read_one_Byte(&H, ACCEL_XOUT_H, SlaveAddress);
	I2c_Read_one_Byte(&L, ACCEL_XOUT_L, SlaveAddress);
        *x = (H<<8)+L ;
  I2c_Read_one_Byte(&H, ACCEL_YOUT_H, SlaveAddress);
	I2c_Read_one_Byte(&L, ACCEL_YOUT_L, SlaveAddress);
        *y = (H<<8)+L;
  I2c_Read_one_Byte(&H, ACCEL_ZOUT_H, SlaveAddress);
	I2c_Read_one_Byte(&L, ACCEL_ZOUT_L, SlaveAddress);
        *z = (H<<8)+L;
}
//**************************************

//**************************************    
void mpu6050_ACCEL(int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t H,L;
	I2c_Read_one_Byte(&H, ACCEL_XOUT_H, SlaveAddress);
	I2c_Read_one_Byte(&L, ACCEL_XOUT_L, SlaveAddress);
        *x = (H<<8)+L ;
        *x -= Accx;
  I2c_Read_one_Byte(&H, ACCEL_YOUT_H, SlaveAddress);
	I2c_Read_one_Byte(&L, ACCEL_YOUT_L, SlaveAddress);
        *y = (H<<8)+L;
        *y -= Accy;
  I2c_Read_one_Byte(&H, ACCEL_ZOUT_H, SlaveAddress);
	I2c_Read_one_Byte(&L, ACCEL_ZOUT_L, SlaveAddress);
        *z = (H<<8)+L;
        *z -= Accz;
//	return (H<<8)+L;   
}



void mpu6050_GYRO(int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t H,L;
	I2c_Read_one_Byte(&H, GYRO_XOUT_H, SlaveAddress);
	I2c_Read_one_Byte(&L, GYRO_XOUT_L, SlaveAddress);
        *x = (H<<8)+L ;
  I2c_Read_one_Byte(&H, GYRO_YOUT_H, SlaveAddress);
	I2c_Read_one_Byte(&L, GYRO_YOUT_L, SlaveAddress);
        *y = (H<<8)+L;
  I2c_Read_one_Byte(&H, GYRO_ZOUT_H, SlaveAddress);
	I2c_Read_one_Byte(&L, GYRO_ZOUT_L, SlaveAddress);
        *z = (H<<8)+L;
}



