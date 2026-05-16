# SCD41 / 震动马达 元器件测试记录

> 日期：2026-05-06

---

## 关键技巧：绕过 GUI 页面限制直接在串口输出传感器数据

### 背景

项目的传感器数据采集由任务调度器（TIM16 + TIM17）管理，任务使能标志 `gTaskEnMark` 由 TouchGFX 界面控制：

| 传感器 | 读取函数 | 使能标志 | 触发条件 |
|--------|----------|----------|----------|
| SCD41 | `Update_AppPageInfo()` | `UPDATE_APP_TASK_EN` | 仅 ApplicationPage 界面 |

**如果不在对应 GUI 页面，任务永远不执行，串口永远没输出。**

### 绕过方法

在 `main.c` 的 `USER CODE BEGIN 2` 区域（外设初始化完成后、主循环前），**直接调用传感器驱动函数**，完全不走任务调度器。

---

## 一、SCD41 CO2 传感器

### 硬件接线

| SCD41 | STM32U575 |
|-------|-----------|
| VCC | 3.3V |
| GND | GND |
| SCL | PB6 (I2C1_SCL) |
| SDA | PB7 (I2C1_SDA) |

> ⚠️ SCD41 峰值电流 ~200mA，供电需足够裕量。建议模块 VDD-GND 间加 100μF + 0.1μF 电容。

### 驱动文件

- `Core/Inc/bsp_scd41.h` — 命令宏、数据结构、函数声明
- `Core/Src/bsp_scd41.c` — CRC8、I2C 收发、数据解析

### 发现的问题 & 解决方案

#### 问题 1：上电后立即通信失败

- **现象**：`[SCD41] Step1 FAIL: No device at 0x62!`
- **根因**：SCD41 上电后需 **1000ms** 才能响应 I2C 命令。MCU 启动快，还没等到就发了读序列号请求
- **解决**：通信前先 `HAL_Delay(1000)`

#### 问题 2：数据就绪检测 Bug

- **现象**：`[SCD41] Step4 FAIL: No data within 6s.`
- **根因**：`SCD41_GetDataReady()` 只检查了 MSB 字节 `buf[0]`，数据就绪时状态值 0x0001，MSB 为 0x00，永远检测不到
- **解决**：合并两个字节判断低 11 位

```c
// 修复前
*ready = (buf[0] & 0x07) ? 1 : 0;

// 修复后
uint16_t status = ((uint16_t)buf[0] << 8) | buf[1];
*ready = (status & 0x07FF) ? 1 : 0; // 低 11 位非 0 表示数据就绪
```

#### 问题 3：Init 使用了不必要的 reinit 命令

- **根因**：`SCD41_Init()` 调用了 0x3646（reinit），SCD41 上电后无需此操作
- **解决**：改为 `HAL_Delay(1000)` + `SCD41_StartPeriodic()`

##### 启动测试代码（Core/Src/main.c，USER CODE BEGIN 2 区域）

```c
//=== SCD41 CO2 传感器测试 ===
printf("[SCD41] Testing sensor on I2C1 addr 0x62...\r\n");
{
    printf("[SCD41] Waiting 1s for power-up...\r\n");
    HAL_Delay(1000);
    printf("[SCD41] Step1: Reading serial number...\r\n");
    uint64_t sn = 0;
    if(SCD41_GetSerialNumber(&sn))
    {
        printf("[SCD41] Step1 PASS: SN=%012llX\r\n", sn);
        // ... Init, wait 5s, read measurement ...
    }
}
//=== 测试结束 ===
```

### 最终测试结果

```
[SCD41] Step1 PASS: SN=FA7115073BC0
[SCD41] Step2 PASS. Waiting first data (~5s)...
[SCD41] Step3: CO2=427ppm Temp=27.3C Hum=47.0%
```

✅ 通过

---

## 二、MK0161999 震动马达

### 硬件接线

| MK0161999 | STM32U575 |
|-----------|-----------|
| VCC | 3.3V |
| GND | GND |
| IN | PC7 |

### GPIO 配置

PC7 在 CubeMX 中配置为推挽输出（GPIO Output Push-Pull），默认低电平。无需定时器 PWM，纯高低电平即可驱动。

### 已有驱动宏

```c
// Core/Inc/user_app.h
#define EXT_MOTOR_ENABLE()    HAL_GPIO_WritePin(EXT_MOTOR_GPIO_Port, EXT_MOTOR_Pin, GPIO_PIN_SET)
#define EXT_MOTOR_DISABLE()   HAL_GPIO_WritePin(EXT_MOTOR_GPIO_Port, EXT_MOTOR_Pin, GPIO_PIN_RESET)
```

- `EXT_MOTOR_ENABLE()` — PC7 输出高电平，马达振动
- `EXT_MOTOR_DISABLE()` — PC7 输出低电平，马达停止

### 测试代码（Core/Src/main.c，USER CODE BEGIN 2 区域）

```c
//=== MK0161999 震动马达测试 ===
printf("[MOTOR] Test start: Motor ON (1s)...\r\n");
EXT_MOTOR_ENABLE();
HAL_Delay(1000);
EXT_MOTOR_DISABLE();
printf("[MOTOR] Motor OFF.\r\n");
printf("[MOTOR] Test done. Check if motor vibrated.\r\n");
//=== 测试结束 ===
```

测试逻辑：拉高 PC7 → 延时 1 秒 → 拉低 PC7。通过串口打印配合手动观察确认马达是否振动。

### 测试结果

```
[MOTOR] Test start: Motor ON (1s)...
[MOTOR] Motor OFF.
[MOTOR] Test done. Check if motor vibrated.
```

✅ 通过（马达振动 1 秒后停止）

---

## I2C1 总线设备总览（相关部分）

| 地址 | 设备 | 功能 | 测试结果 |
|------|------|------|----------|
| 0x62 | SCD41 | CO2/温湿度 | ✅ CO2=427ppm |

## 总结

| 元器件 | 接口 | 引脚/地址 | 测试结果 |
|--------|------|-----------|----------|
| SCD41 | I2C1 | 0x62 | ✅ CO2=427ppm |
| MK0161999 | GPIO | PC7 | ✅ 振动正常 |

### 永久修复

| 文件 | 修改 | 原因 |
|------|------|------|
| `Core/Src/bsp_scd41.c` `SCD41_GetDataReady()` | MSB → 双字节合并 | 数据就绪检测 Bug |
| `Core/Src/bsp_scd41.c` `SCD41_Init()` | reinit → 1000ms delay | SCD41 上电无需 reinit |
