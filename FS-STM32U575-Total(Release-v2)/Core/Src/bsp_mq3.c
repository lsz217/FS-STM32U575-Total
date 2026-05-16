/**
  ******************************************************************************
  * @file   bsp_mq3.c
  * @brief  MQ-3酒精传感器驱动
  ******************************************************************************
  */
#include "bsp_mq3.h"
#include "bsp_motor.h"
#include <stdio.h>

/*
**********************************************************************
* @fun     :BSP_MQ3_Init
* @brief   :初始化MQ-3传感器GPIO (PB9 输入上拉)
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MQ3_Init(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = MQ3_GPIO_PIN;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(MQ3_GPIO_PORT, &g);
}

/*
**********************************************************************
* @fun     :BSP_MQ3_Read
* @brief   :读取MQ-3传感器状态
* @param   :none
* @return  :0=检测到气体(报警), 1=正常
**********************************************************************
*/
uint8_t BSP_MQ3_Read(void)
{
    return (HAL_GPIO_ReadPin(MQ3_GPIO_PORT, MQ3_GPIO_PIN) == GPIO_PIN_RESET) ? 0 : 1;
}

/*
**********************************************************************
* @fun     :BSP_MQ3_Test
* @brief   :MQ-3传感器阻塞测试（自检→预热→模拟报警→监控）
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MQ3_Test(void)
{
    // 蜂鸣器+马达自检
    printf("[MQ-3] Buzzer & Motor self-test...\r\n");
    BEEP_ENABLE();
    BSP_MOTOR_On();
    HAL_Delay(2000);
    BEEP_DISABLE();
    BSP_MOTOR_Off();

    BSP_MQ3_Init();

    printf("[MQ-3] === Sensor Test ===\r\n");
    printf("[MQ-3] Alcohol -> PB9 (LOW=alarm)\r\n");
    printf("[MQ-3] Preheating %ds...\r\n", MQ3_PREHEAT_SEC);
    for(int t = MQ3_PREHEAT_SEC; t > 0; t -= 10)
    {
        printf("[MQ-3] Preheating... %3d s\r\n", t);
        HAL_Delay(10000);
    }
    printf("[MQ-3] Preheat done! Simulated test in 10s...\r\n");
    HAL_Delay(10000);

    // 模拟报警测试
    printf("[MQ-3] *** SIMULATED: Alcohol ALARM ***\r\n");
    BEEP_ENABLE();
    BSP_MOTOR_On();
    HAL_Delay(3000);
    printf("[MQ-3] *** SIMULATED: Alcohol cleared ***\r\n");
    BEEP_DISABLE();
    BSP_MOTOR_Off();
    printf("[MQ-3] Simulated test done. Entering monitoring...\r\n");

    uint8_t last = 1;
    uint32_t tick = 0;
    while(1)
    {
        uint8_t cur = BSP_MQ3_Read();
        tick++;
        if(tick % 30 == 0)
            printf("[MQ-3] PB9=%s\r\n", cur ? "HIGH" : "LOW");

        if(cur == 0)
        {
            if(last != cur)
            {
                printf("[MQ-3] >>> ALARM! Alcohol detected! <<<\r\n");
                BEEP_ENABLE();
                BSP_MOTOR_On();
                HAL_Delay(3000);
            }
            BEEP_ENABLE();
            BSP_MOTOR_On();
        }
        else if(last != cur)
        {
            printf("[MQ-3] Alcohol cleared.\r\n");
            BEEP_DISABLE();
            BSP_MOTOR_Off();
        }
        last = cur;
        HAL_Delay(300);
    }
}
