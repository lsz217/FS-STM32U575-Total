/**
  ******************************************************************************
  * @file   bsp_mq2.c
  * @brief  MQ-2烟雾传感器驱动
  ******************************************************************************
  */
#include "bsp_mq2.h"
#include "bsp_motor.h"
#include <stdio.h>

/*
**********************************************************************
* @fun     :BSP_MQ2_Init
* @brief   :初始化MQ-2传感器GPIO (PB8 输入上拉)
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MQ2_Init(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = MQ2_GPIO_PIN;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(MQ2_GPIO_PORT, &g);
}

/*
**********************************************************************
* @fun     :BSP_MQ2_Read
* @brief   :读取MQ-2传感器状态
* @param   :none
* @return  :0=检测到气体(报警), 1=正常
**********************************************************************
*/
uint8_t BSP_MQ2_Read(void)
{
    return (HAL_GPIO_ReadPin(MQ2_GPIO_PORT, MQ2_GPIO_PIN) == GPIO_PIN_RESET) ? 0 : 1;
}

/*
**********************************************************************
* @fun     :BSP_MQ2_Test
* @brief   :MQ-2传感器阻塞测试（自检→预热→模拟报警→监控）
* @param   :none
* @return  :none
**********************************************************************
*/
void BSP_MQ2_Test(void)
{
    // 蜂鸣器+马达自检
    printf("[MQ-2] Buzzer & Motor self-test...\r\n");
    BEEP_ENABLE();
    BSP_MOTOR_On();
    HAL_Delay(2000);
    BEEP_DISABLE();
    BSP_MOTOR_Off();

    BSP_MQ2_Init();

    printf("[MQ-2] === Sensor Test ===\r\n");
    printf("[MQ-2] Smoke -> PB8 (LOW=alarm)\r\n");
    printf("[MQ-2] Preheating %ds...\r\n", MQ2_PREHEAT_SEC);
    for(int t = MQ2_PREHEAT_SEC; t > 0; t -= 10)
    {
        printf("[MQ-2] Preheating... %3d s\r\n", t);
        HAL_Delay(10000);
    }
    printf("[MQ-2] Preheat done! Simulated test in 10s...\r\n");
    HAL_Delay(10000);

    // 模拟报警测试
    printf("[MQ-2] *** SIMULATED: Smoke ALARM ***\r\n");
    BEEP_ENABLE();
    BSP_MOTOR_On();
    HAL_Delay(3000);
    printf("[MQ-2] *** SIMULATED: Smoke cleared ***\r\n");
    BEEP_DISABLE();
    BSP_MOTOR_Off();
    printf("[MQ-2] Simulated test done. Entering monitoring...\r\n");

    uint8_t last = 1;
    uint32_t tick = 0;
    while(1)
    {
        uint8_t cur = BSP_MQ2_Read();
        tick++;
        if(tick % 30 == 0)
            printf("[MQ-2] PB8=%s\r\n", cur ? "HIGH" : "LOW");

        if(cur == 0)
        {
            if(last != cur)
            {
                printf("[MQ-2] >>> ALARM! Smoke detected! <<<\r\n");
                BEEP_ENABLE();
                BSP_MOTOR_On();
                HAL_Delay(3000);
            }
            BEEP_ENABLE();
            BSP_MOTOR_On();
        }
        else if(last != cur)
        {
            printf("[MQ-2] Smoke cleared.\r\n");
            BEEP_DISABLE();
            BSP_MOTOR_Off();
        }
        last = cur;
        HAL_Delay(300);
    }
}
