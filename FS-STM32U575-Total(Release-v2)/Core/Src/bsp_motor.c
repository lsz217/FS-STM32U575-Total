/**
  ******************************************************************************
  * @file   bsp_motor.c
  * @brief  震动马达驱动 (PC7)
  ******************************************************************************
  */
#include "bsp_motor.h"
#include <stdio.h>

/*
**********************************************************************
* @fun     :BSP_MOTOR_Init
* @brief   :初始化马达GPIO（已在MX_GPIO_Init配置，此处确保关闭）
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MOTOR_Init(void)
{
    BSP_MOTOR_Off();
}

/*
**********************************************************************
* @fun     :BSP_MOTOR_On
* @brief   :开启马达震动
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MOTOR_On(void)
{
    HAL_GPIO_WritePin(MOTOR_GPIO_PORT, MOTOR_GPIO_PIN, GPIO_PIN_SET);
}

/*
**********************************************************************
* @fun     :BSP_MOTOR_Off
* @brief   :关闭马达震动
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MOTOR_Off(void)
{
    HAL_GPIO_WritePin(MOTOR_GPIO_PORT, MOTOR_GPIO_PIN, GPIO_PIN_RESET);
}

/*
**********************************************************************
* @fun     :BSP_MOTOR_Toggle
* @brief   :翻转马达状态
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MOTOR_Toggle(void)
{
    HAL_GPIO_TogglePin(MOTOR_GPIO_PORT, MOTOR_GPIO_PIN);
}

/*
**********************************************************************
* @fun     :BSP_MOTOR_Test
* @brief   :马达自检（震动2秒）
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MOTOR_Test(void)
{
    printf("[MOTOR] Self-test...\r\n");
    BSP_MOTOR_On();
    HAL_Delay(2000);
    BSP_MOTOR_Off();
    printf("[MOTOR] Self-test done.\r\n");
}
