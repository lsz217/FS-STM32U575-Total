# MQ-2 / MQ-3 烟雾&酒精传感器 测试记录

> 日期：2026-05-14

---

## 一、硬件接线

| 传感器 | 引脚 | STM32U575 | 说明 |
|--------|------|-----------|------|
| MQ-2 烟雾 | VCC | 5V | |
| MQ-2 烟雾 | GND | GND | |
| MQ-2 烟雾 | DO | PB8 | 数字输出 (TTL) |
| MQ-3 酒精 | VCC | 5V | |
| MQ-3 酒精 | GND | GND | |
| MQ-3 酒精 | DO | PB9 | 数字输出 (TTL) |

> MQ-2 和 MQ-3 各自独立接 GPIO。5V 供电，DO 输出需电阻分压至 3.3V 再接入 STM32 GPIO。
> AO（模拟输出）不接。

---

## 二、报警输出

| 设备 | 引脚 | 说明 |
|------|------|------|
| 蜂鸣器 | PA15 | `BEEP_ENABLE/DISABLE` 宏控制 |
| MK0161999 震动马达 | PC7 | `EXT_MOTOR_ENABLE/DISABLE` 宏控制 |

> 蜂鸣器和马达同时触发，首次检测到气体时持续 3 秒确认震动，之后保持报警直到气体散去。

---

## 三、测试代码

以下代码放在 `Core/Src/main.c` 的 `USER CODE BEGIN 2` 区域，
W25Q128 Flash 初始化之后、NFC 初始化之前：

```c
//=== MQ-2 & MQ-3 传感器测试 ===
// MQ-2烟雾: VCC→5V, GND→GND, DO→PB8
// MQ-3酒精: VCC→5V, GND→GND, DO→PB9
// 注意: 5V供电,DO需电阻分压至3.3V!
{
    // 蜂鸣器+马达自检
    printf("[MQ-2][MQ-3] Buzzer & Motor self-test...\r\n");
    BEEP_ENABLE();
    EXT_MOTOR_ENABLE();
    HAL_Delay(2000);
    BEEP_DISABLE();
    EXT_MOTOR_DISABLE();

    GPIO_InitTypeDef g = {0};
    // MQ-2 → PB8
    g.Pin = GPIO_PIN_8;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &g);
    // MQ-3 → PB9
    g.Pin = GPIO_PIN_9;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &g);

    printf("[MQ-2][MQ-3] === Sensor Test ===\r\n");
    printf("[MQ-2] Smoke  -> PB8 (LOW=alarm)\r\n");
    printf("[MQ-3] Alcohol-> PB9 (LOW=alarm)\r\n");
    printf("[MQ-2][MQ-3] Preheating 120s...\r\n");
    for(int t = 120; t > 0; t -= 10)
    {
        printf("[MQ-2][MQ-3] Preheating... %3d s\r\n", t);
        HAL_Delay(10000);
    }
    printf("[MQ-2][MQ-3] Preheat done! Simulated test in 10s...\r\n");
    HAL_Delay(10000);

    // 模拟报警测试（无需实际气体即可验证蜂鸣器+马达）
    printf("[MQ-2] *** SIMULATED: Smoke ALARM ***\r\n");
    BEEP_ENABLE();
    EXT_MOTOR_ENABLE();
    HAL_Delay(3000);
    printf("[MQ-2] *** SIMULATED: Smoke cleared ***\r\n");
    BEEP_DISABLE();
    EXT_MOTOR_DISABLE();
    HAL_Delay(1000);
    printf("[MQ-3] *** SIMULATED: Alcohol ALARM ***\r\n");
    BEEP_ENABLE();
    EXT_MOTOR_ENABLE();
    HAL_Delay(3000);
    printf("[MQ-3] *** SIMULATED: Alcohol cleared ***\r\n");
    BEEP_DISABLE();
    EXT_MOTOR_DISABLE();
    printf("[MQ-2][MQ-3] Simulated test done. Entering monitoring...\r\n");

    uint8_t mq2_last = 1, mq3_last = 1;
    uint32_t tick = 0;
    while(1)
    {
        uint8_t mq2 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == GPIO_PIN_RESET) ? 0 : 1;
        uint8_t mq3 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_RESET) ? 0 : 1;
        tick++;
        if(tick % 30 == 0)
            printf("[MQ-2] PB8=%s  [MQ-3] PB9=%s\r\n", mq2?"HIGH":"LOW", mq3?"HIGH":"LOW");

        // MQ-2 检测
        if(mq2 == 0)
        {
            if(mq2_last != mq2)
            {
                printf("[MQ-2] >>> ALARM! Smoke detected! <<<\r\n");
                BEEP_ENABLE();
                EXT_MOTOR_ENABLE();
                HAL_Delay(3000);
            }
            BEEP_ENABLE();
            EXT_MOTOR_ENABLE();
        }
        else if(mq2_last != mq2)
            printf("[MQ-2] Smoke cleared.\r\n");
        mq2_last = mq2;

        // MQ-3 检测
        if(mq3 == 0)
        {
            if(mq3_last != mq3)
            {
                printf("[MQ-3] >>> ALARM! Alcohol detected! <<<\r\n");
                BEEP_ENABLE();
                EXT_MOTOR_ENABLE();
                HAL_Delay(3000);
            }
            BEEP_ENABLE();
            EXT_MOTOR_ENABLE();
        }
        else if(mq3_last != mq3)
            printf("[MQ-3] Alcohol cleared.\r\n");
        mq3_last = mq3;

        // 两者都正常时关闭蜂鸣器和马达
        if(mq2 == 1 && mq3 == 1)
        {
            BEEP_DISABLE();
            EXT_MOTOR_DISABLE();
        }
        HAL_Delay(300);
    }
}
//=== MQ-2 & MQ-3 测试结束 (此后不会执行) ===
```

---

## 四、遇到的问题 & 解决方案

### 问题 1：蜂鸣器不响

- **现象**：PA15 ODR 寄存器在 0x8000 ↔ 0x0000 之间正常切换，但蜂鸣器无声
- **诊断**：打印 GPIOA 全部配置寄存器，MODER=输出模式、OTYPER=推挽、ODR 随写操作正确变化
- **根因**：PA15 是有源蜂鸣器，但 STM32 GPIO 最大驱动电流 ~8mA，蜂鸣器需 20-40mA，GPIO 直驱推不动；开发板上蜂鸣器通道可能存在跳线或三极管驱动的额外条件
- **解决**：重新验证后发现 PA15 蜂鸣器可通过 BEEP_ENABLE/DISABLE 宏正常工作，最终采用蜂鸣器 + 震动马达双重报警

### 问题 2：`HAL_TIM_Base_Stop_IT` 导致 HAL_Delay 卡住

- **现象**：预热倒数打印了第一行 120s 后就不再更新，程序卡死
- **根因**：TIM16/TIM17 在 main() 中 `USER CODE BEGIN 2` 位置尚未启动（`HAL_TIM_Base_Start_IT` 在更后面），此时调用 `HAL_TIM_Base_Stop_IT` 导致定时器状态异常，连带影响 SysTick
- **解决**：删除 `HAL_TIM_Base_Stop_IT` 调用。改用马达报警后不存在 TIM16 冲突问题，无需停止任何定时器

### 问题 3：printf `\r\n` 转义序列处理

- **现象**：编译报错 `expected ')'`，`\r\n` 被当成了真正的换行符而非 C 转义序列
- **根因**：Python 脚本生成 C 代码时，格式串文本末尾多了一个 `"`，导致 C 字符串被提前关闭，`\r\n` 跑到字符串外面
- **解决**：确保 `\r\n` 转义序列在 C 格式字符串**内部**，即 `"...text...\r\n");` 而非 `"...text..."\r\n);`

### 问题 4：MQ-2 上电误报

- **现象**：预热完成后立即报警，但周围没有可燃气体
- **根因**：MQ-2 传感器初次使用需要 24-48 小时老化（aging），且灵敏度电位器可能扭到最灵敏位置
- **解决**：
  1. 灵敏度电位器**逆时针**扭到最钝（降低灵敏度）
  2. 上电预热至少 2 分钟后使用
  3. 新传感器建议持续上电 24 小时老化

### 问题 5：马达震动不明显

- **现象**：传感器触发后马达 300ms 脉冲太短，感觉不到震动
- **解决**：首次检测到气体时马达 + 蜂鸣器持续 3 秒确认震动，报警期间保持常开

### 问题 6：无实际气体时无法测试报警

- **解决**：预热结束后增加 10 秒倒计时，自动执行模拟报警测试：先模拟烟雾报警 3 秒，再模拟酒精报警 3 秒，无需实际气体即可验证蜂鸣器和马达是否正常工作

---

## 五、最终测试结果

```
W25Q128 OK,flash ID:EF4018
[MQ-2][MQ-3] Buzzer & Motor self-test...
[MQ-2][MQ-3] === Sensor Test ===
[MQ-2] Smoke  -> PB8 (LOW=alarm)
[MQ-3] Alcohol-> PB9 (LOW=alarm)
[MQ-2][MQ-3] Preheating 120s...
[MQ-2][MQ-3] Preheating... 120 s
...
[MQ-2][MQ-3] Preheat done! Simulated test in 10s...
[MQ-2] *** SIMULATED: Smoke ALARM ***
[MQ-2] *** SIMULATED: Smoke cleared ***
[MQ-3] *** SIMULATED: Alcohol ALARM ***
[MQ-3] *** SIMULATED: Alcohol cleared ***
[MQ-2][MQ-3] Simulated test done. Entering monitoring...
[MQ-2] PB8=HIGH  [MQ-3] PB9=HIGH
[MQ-2] PB8=HIGH  [MQ-3] PB9=HIGH
```

- 蜂鸣器+马达自检 ✅
- 传感器预热 ✅
- 模拟报警测试（蜂鸣+马达） ✅
- 串口状态输出 ✅
- 报警检测逻辑 ✅

## 六、使用说明

1. 上电后蜂鸣器 + 马达自检 2 秒
2. MQ-2/MQ-3 上电后需预热 120 秒（程序自动倒计时）
3. 预热完成后 10 秒自动执行模拟报警测试（烟雾 3s → 酒精 3s），无需实际气体
4. 模拟测试完成后进入监控模式，每 ~9s 打印 PB8/PB9 状态
5. PB8/PB9=LOW → 蜂鸣器鸣叫 + 马达震动（首次 3 秒确认，之后持续报警）
6. PB8/PB9=HIGH → 蜂鸣器停止 + 马达停止
7. 灵敏度用电位器调节：逆时针=最钝，顺时针=最灵敏
