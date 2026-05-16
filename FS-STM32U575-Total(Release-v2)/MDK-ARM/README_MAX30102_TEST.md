# MAX30102 心率传感器测试方案

## 1. 修改概览

| 文件 | 改动类型 | 目的 |
| :--- | :--- | :--- |
| `Core/Src/main.c` | 新增启动测试代码 | 启动阶段自检 MAX30102 硬件 |
| `Core/Src/user_app.c` | 新增宏定义 | 开启串口调试打印 (DEBUG_MODE) |

---

## 2. 软件修改详情

### A. `Core/Src/user_app.c`
在文件顶部（约第 12 行）定义调试宏，以启用驱动内部隐藏的 `printf` 输出：
```c
#include "user_app.h"
#define DEBUG_MODE  // 调试模式：开启心率/血氧/温度等 printf 输出
/***************************心率血氧传感器***************************/
```
**作用**：开启后，串口将实时输出原始 PPG 波形采样点、心率（HR）、血氧（SpO2）及芯片温度。

### B. `Core/Src/main.c`
在 `USER CODE BEGIN 2` 区域（ADC 校准之后）添加硬件自检逻辑，不依赖 GUI 页面：
```c
    //=== MAX30102 心率传感器测试 === 
    printf("[SYS] Boot OK, USART1 ready.\r\n"); 
    printf("[MAX30102] Testing sensor...\r\n"); 
    if(maxim_max30102_init()) 
    { 
        uint8_t part_id = 0; 
        maxim_max30102_read_reg(REG_PART_ID, &part_id); 
        printf("[MAX30102] Init OK, PART_ID=0x%02X (expect 0x15)\r\n", part_id); 
        int8_t int_temp; 
        uint8_t frac_temp; 
        if(maxim_max30102_read_temperature(&int_temp, &frac_temp)) 
        { 
            float chip_temp = int_temp + (float)frac_temp / 16.0f; 
            printf("[MAX30102] Chip Temp=%.1fC\r\n", chip_temp); 
        } 
    } 
    else 
        printf("[MAX30102] Init FAILED! Check wiring.\r\n"); 
    //=== 测试结束 === 
```
**验证内容**：
1. `maxim_max30102_init()`：确认 I2C 通信及寄存器配置是否成功。
2. `PART_ID`：确认器件 ID 是否为 `0x15`。
3. `Temperature`：读取芯片内部温度，验证数据读取链路。

---

## 3. 期望串口输出

### 启动阶段（开机即输出）
```text
[SYS] Boot OK, USART1 ready.
[MAX30102] Testing sensor...
[MAX30102] Init OK, PART_ID=0x15 (expect 0x15)
[MAX30102] Chip Temp=28.3C
```

### 测量阶段（需在 GUI 中触发心率测量）
```text
Temp=30.1, HR=72, SpO2=98
```

## 4. 测试完成后清理
为保持代码整洁，测试通过后建议：
1. 注释或删除 `user_app.c` 中的 `#define DEBUG_MODE`。
2. 删除 `main.c` 中的测试代码块。
