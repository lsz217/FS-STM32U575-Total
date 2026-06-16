# 低功耗模式（Stop 0）开发记录

**日期**: 2026-06-15

---

## 1. 功能概述

通过 USER_KEY 按键（PA12）切换 STM32U575 的 Stop 0 低功耗模式：
- **按下 USER_KEY**：进入低功耗模式，页面切换到功耗监控页（SwipeContainer Page 1）
- **再次按下 USER_KEY**：退出低功耗模式，页面回到主页（SwipeContainer Page 0）

---

## 2. 涉及文件

| 文件 | 修改内容 |
|------|----------|
| `Core/Src/main.c` | USER_KEY EXTI 回调切换逻辑、Stop 0 入口/恢复、200ms 守护 |
| `Core/Src/rtc.c` | RTC WakeUp 定时器配置（LSI, 100ms 周期） |
| `Core/Inc/rtc.h` | RTC_WakeUp_Config / RTC_WakeUp_IRQ_Handler 声明 |
| `Core/Inc/user_app.h` | 添加 `g_low_power_mode`、`g_power_sleep_ratio` 外部声明 |
| `TouchGFX/gui/include/gui/model/ModelListener.hpp` | 添加 `switchToPowerPage()` / `switchToHomePage()` 虚函数 |
| `TouchGFX/gui/src/model/Model.cpp` | 检测 `g_low_power_mode` 变化，触发页面切换 |
| `TouchGFX/gui/include/gui/homepage_screen/HomePagePresenter.hpp` | 添加 switchToPowerPage / switchToHomePage |
| `TouchGFX/gui/src/homepage_screen/HomePagePresenter.cpp` | Presenter 转发页面切换调用 |
| `TouchGFX/gui/include/gui/homepage_screen/HomePageView.hpp` | 添加 switchToPowerPage / switchToHomePage |
| `TouchGFX/gui/src/homepage_screen/HomePageView.cpp` | 实现 SwipeContainer 页面切换 |

---

## 3. 核心实现

### 3.1 Stop 0 入口 + 200ms 守护 (main.c)

```c
// Sleep/Stop: WFI (normal) / Stop 0 (when g_low_power_mode=1)
{
    static uint32_t s_sleep_cnt = 0, s_total = 0, s_tick = 0;
    static uint32_t last_stop_wake = 0;
    s_total++;
    if (!had_work) {
        s_sleep_cnt++;
        if (g_low_power_mode && (HAL_GetTick() - last_stop_wake > 200)) {
            // ---- Stop 0 ----
            HAL_TIM_Base_Stop_IT(&htim16);
            HAL_TIM_Base_Stop_IT(&htim17);
            RTC_WakeUp_Config(200);
            HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);
            // restore after wakeup
            for (volatile uint32_t _ti = 0; _ti < 100; _ti++) { HAL_IncTick(); }
            HAL_ICACHE_Enable();
            HAL_TIM_Base_Start_IT(&htim16);
            HAL_TIM_Base_Start_IT(&htim17);
            HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&gStruADC, ADC_CONVERTED_DATA_BUFFER_SIZE);
            Update_Backlight(gBacklightVal);
            last_stop_wake = HAL_GetTick();
        } else {
            __WFI();
        }
    }
    if (HAL_GetTick() - s_tick >= 1000) {
        g_power_sleep_ratio = (uint8_t)(s_sleep_cnt * 100 / s_total);
        s_sleep_cnt = 0; s_total = 0; s_tick = HAL_GetTick();
    }
}
```

### 3.2 USER_KEY EXTI 切换逻辑 (main.c)

```c
if((GPIO_Pin == USER_KEY_Pin) && !HAL_GPIO_ReadPin(USER_KEY_GPIO_Port,USER_KEY_Pin))
{
    g_low_power_mode = g_low_power_mode ? 0 : 1;  // 切换
}
```

### 3.3 RTC WakeUp 定时器 (rtc.c)

```c
// RTC_WAKEUPCLOCK_RTCCLK_DIV16: RTCCLK/16, LSI=32000Hz -> 2000Hz
// 100ms = 200 ticks
#define RTC_WAKEUP_100MS  200

void RTC_WakeUp_Config(uint32_t ticks)
{
    if (ticks == 0) ticks = RTC_WAKEUP_100MS;
    HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, ticks, RTC_WAKEUPCLOCK_RTCCLK_DIV16, 0);
}
```

### 3.4 页面切换 (Model.cpp)

```c
{
    static uint8_t _prev_lp = 0;
    if (g_low_power_mode && !_prev_lp) modelListener->switchToPowerPage();
    if (!g_low_power_mode && _prev_lp) modelListener->switchToHomePage();
    _prev_lp = g_low_power_mode;
}
```

### 3.5 功耗监控显示 (HomePageView.cpp)

```c
void HomePageView::updatePowerInfo(uint16_t currentVal, uint16_t voltageVal, uint8_t sleepRatio)
{
    float current_mA = currentVal * 100.0f * 3.3f / 4095.0f;
    float voltage_V  = voltageVal * 3.3f / 4095.0f;
    // 显示电流、电压、CPU 休眠占比
    ...
}
```

---

## 4. 遇到的问题与解决方案

### 问题 1: NVIC_SystemReset() 导致启动卡死

**现象**: 按下 USER_KEY 退出低功耗时，系统确实复位了，但复位后显示屏无输出，触摸无响应，串口无输出。

**原因**: `NVIC_SystemReset()` 在 EXTI 中断回调中（Stop 模式唤醒期间）被调用。MCU 复位了，但 ESP8266 等外部模块未断电复位，仍保持之前的状态。MCU 复位后 `ESP8266_Init()` 向状态异常的 ESP8266 发送 AT 指令，ESP8266 无法正确响应，导致初始化阻塞，整个启动流程卡死。

**解决**: 放弃 `NVIC_SystemReset()` 方案，改用简单的切换逻辑（toggle `g_low_power_mode`）。按一次进入低功耗，再按一次退出。不再复位 MCU，避免了 ESP8266 重新初始化的问题。

```c
// 错误做法 ❌
if (g_low_power_mode) {
    NVIC_SystemReset();  // ISR 中复位，外设状态不一致
}

// 正确做法 ✅
g_low_power_mode = g_low_power_mode ? 0 : 1;  // 切换即可
```

### 问题 2: 退出低功耗后任务饥饿（timer starvation）

**现象**: 退出 Stop 0 后，系统界面刷新极慢（~40s 一次），触摸几乎无响应。

**原因**: 每次 Stop 0 唤醒后，TIM16/TIM17 被停止并重新从 0 开始计数。TIM17 周期为 100ms，但主循环在 `had_work == 0` 时立即重新进入 Stop/WFI，导致 TIM17 永远无法达到 100ms 触发任务调度。

**解决**: 添加 200ms 守护计时器（`last_stop_wake`）。每次 Stop 唤醒后的 200ms 内，只使用普通 WFI（不进入 Stop），给 TIM16/TIM17 足够时间触发并调度任务。

### 问题 3: 电流/电压显示变化不明显

**现象**: 进入低功耗后，功耗监控页的电流和电压值变化不大。

**原因**: ADC 测量的是整个扩展板的总电流/电压，包括 ESP8266（WiFi 持续工作）、LCD 背光、传感器等。STM32 本身的功耗变化（Stop 0）占总功耗的比例很小，因此总数值变化不明显。

**建议**: 如需直观体现低功耗效果，可在进入低功耗时调低背光亮度（如降至 10%），这比 Stop 0 本身对功耗的影响更直观可见。

---

## 5. 退出低功耗后传感器/界面冻结 —— 根因与修复 (2026-06-16)

### 根因 0（核心）：Stop 0 唤醒后 PLL 时钟未恢复 🔴🔴🔴

**这是最关键的问题。** STM32U575 进入 Stop 0 后，HSE 和 PLL 均被停止。Wakeup 后 CPU 默认运行在 HSI（16MHz），但原来的系统时钟是 PLL 160MHz（HSE 8MHz → PLL ×40 / 2 = 160MHz）。

`SystemClock_Config()` 只在 `main()` 初始化时调用了一次。Stop 0 唤醒后**从未重新配置 PLL**，导致：
- CPU 运行在 16MHz 而非 160MHz（慢了 10 倍）
- 所有 APB 外设（TIM16/TIM17/UART/SPI 等）时钟比例错误
- SysTick 中断间隔变成 10ms 而非 1ms
- 串口波特率偏移 10 倍，输出乱码或完全无法识别
- 屏幕刷新从 50Hz 降到约 5Hz，看起来"卡死"

**修复**：在 Stop 0 恢复路径中添加 `SystemClock_Config()` 调用，重新使能 HSE 和 PLL，恢复 160MHz 系统时钟。

```
HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);
// restore after wakeup
HAL_IncTick x100;
HAL_ICACHE_Enable();
SystemClock_Config();   // <--- 新增：恢复 PLL 160MHz
HAL_TIM_Base_Start_IT(&htim16);
HAL_TIM_Base_Start_IT(&htim17);
...
```

### 根因 1：RTC WakeUp 定时器未关闭

进入 Stop 0 时通过 `RTC_WakeUp_Config(200)` 使能了 RTC 唤醒定时器。退出低功耗模式（`g_low_power_mode=0`）后，该定时器从未被禁用，持续以 ~100ms 间隔产生 RTC 中断，干扰正常时序。

**修复**：
- `rtc.c` 新增 `RTC_WakeUp_Deactivate()` 函数，调用 `HAL_RTCEx_DeactivateWakeUpTimer()`
- `main.c` 退出低功耗的三条路径都会调用此函数

### 根因 2：ADC DMA 在 Stop 0 恢复中被反复重启

每次 Stop 0 唤醒后调用 `HAL_ADC_Start_DMA()` 重启 ADC DMA，但进入 Stop 0 前未先停止。在已运行的 DMA 上反复启动会导致 HAL 状态机错误，ADC DMA 转换完成回调不再触发，`gTaskStateBit.ADCC` 永远为 0，依赖它的 `Update_AppPageInfo()` 和 `Update_ChipInfo()` 无法更新。

**修复**：
- 进入 Stop 0 前先调用 `HAL_ADC_Stop_DMA(&hadc1)` 停止 DMA
- 退出低功耗时执行 `HAL_ADC_Stop_DMA` + `HAL_ADC_Start_DMA` 完整重启

### 根因 3：I2C 外设 Stop 0 后状态可能异常

Stop 0 期间 I2C 时钟停止，多次 Stop 0 进入/退出后 I2C 外设状态机可能异常，导致 SHT20/SCD41/AP3216C/MPU6050/FT6336 等 I2C 传感器读取失败。

**修复**：
- 退出低功耗时调用 `MX_I2C1_Init()` 重新初始化 I2C1 外设

### 根因 4：缺少退出低功耗的统一清理逻辑

当 `g_low_power_mode` 从 1 变为 0 时（由 EXTI 回调触发），没有任何显式清理操作。新增 `s_was_low_power` 状态追踪变量，在三条可能的退出路径上执行清理：

| 退出路径 | 场景 | 清理动作 |
|----------|------|----------|
| Stop 0 恢复后检测 | EXTI 在 Stop 0 期间唤醒，`g_low_power_mode` 已变 0 | `RTC_WakeUp_Deactivate()` |
| WFI 路径检测 | `g_low_power_mode` 在 WFI 期间被切换 | RTC 禁用 + ADC 重启 + I2C 重初始化 |
| had_work 路径检测 | `g_low_power_mode` 在任务执行期间被切换 | RTC 禁用 + ADC 重启 + I2C 重初始化 |

### 涉及文件修改

| 文件 | 修改内容 |
|------|----------|
| `Core/Src/rtc.c` | 新增 `RTC_WakeUp_Deactivate()` |
| `Core/Inc/rtc.h` | 新增 `RTC_WakeUp_Deactivate()` 声明 |
| `Core/Src/main.c` | Sleep 块重构：新增 `s_was_low_power` 追踪、ADC Stop/Start 配对、三条退出路径清理、**Stop 0 恢复中添加 `SystemClock_Config()`** |
