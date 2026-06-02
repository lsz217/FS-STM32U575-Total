/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : STM32TouchController.cpp
  ******************************************************************************
  * This file was created by TouchGFX Generator 4.21.3. This file is only
  * generated once! Delete this file from your project and re-generate code
  * using STM32CubeMX or change this file manually to update it.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* USER CODE BEGIN STM32TouchController */

#include <STM32TouchController.hpp>

void STM32TouchController::init()
{
    /**
     * Initialize touch controller and driver
     *
     */
}
extern "C"
{
	#include "bsp_ft6336.h"
}
extern "C" FT6336_TouchPointType tp;
extern volatile uint32_t ft6336_on_touch_count;
//
bool STM32TouchController::sampleTouch(int32_t& x, int32_t& y)
{
    // 调试：每次调用递增（在C代码中打印）
    g_sample_touch_calls++;

    uint16_t xDiff = 0,yDiff = 0;
    static uint16_t pI_Touch_X = 0, pI_Touch_Y = 0;

    if (ft6336_on_touch_count)
    {
        uint8_t id1, touch_count;
        uint16_t tx, ty;

        // 优先尝试批量读取（5字节一次），失败则回退逐字节读取
        uint8_t buf[5];
        if (FT6336_readBytes(0x02, buf, 5))
        {
            touch_count = buf[0] & 0x0F;
            if (touch_count == 0) { ft6336_on_touch_count = 0; return false; }
            id1 = (buf[3] >> 4) & 0x01;
            tx  = ((buf[1] & 0x0F) << 8) | buf[2];
            ty  = ((buf[3] & 0x0F) << 8) | buf[4];
        }
        else
        {
            // 回退：逐字节读取（兼容模式）
            id1 = FT6336_read_touch1_id();
            tx  = FT6336_read_touch1_x();
            ty  = FT6336_read_touch1_y();
        }

        tp.tp[id1].status = (tp.tp[id1].status == release) ? touch : stream;
        tp.tp[id1].x = tx;
        tp.tp[id1].y = ty;
        tp.tp[~id1 & 0x01].status = release;

        if (tp.tp[0].status != release)
        {
            xDiff = tp.tp[0].x > pI_Touch_X ? (tp.tp[0].x - pI_Touch_X): (pI_Touch_X - tp.tp[0].x);
            yDiff = tp.tp[0].y > pI_Touch_Y ? (tp.tp[0].y - pI_Touch_Y): (pI_Touch_Y - tp.tp[0].y);
            if ((xDiff + yDiff) > 5)
            {
                pI_Touch_X = tp.tp[0].x;
                pI_Touch_Y = tp.tp[0].y;
            }
            x = pI_Touch_Y;
            y = 240 - pI_Touch_X;
        }

        ft6336_on_touch_count = 0;
        return true;
    }
    return false;
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
