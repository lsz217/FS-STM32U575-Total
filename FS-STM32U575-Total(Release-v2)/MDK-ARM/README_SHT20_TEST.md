# SHT20 温湿度传感器硬件验证测试方案

## 1. 测试目标
在现有 STM32U575 项目中对 SHT20 温湿度传感器进行硬件验证测试，确认元器件及 I2C 通信正常工作，为后续自制扩展板设计提供硬件依据。

## 2. 现有开发条件
- **驱动支持**：项目已集成完整驱动 `bsp_sht20.c` / `bsp_sht20.h`。
- **I2C 配置**：使用 I2C1（PB6-SCL, PB7-SDA），Fast Mode。
- **调试支持**：USART1 `printf` 重定向已就绪。
- **任务调度**：系统每 300ms 自动调用 `Update_AppPageInfo()` -> `BSP_SHT20_GetData()`。

## 3. 硬件连接说明
| SHT20 引脚 | STM32 引脚 | 功能说明 |
| :--- | :--- | :--- |
| **VCC** | 3.3V | 传感器供电 |
| **GND** | GND | 电源地 |
| **SCL** | PB6 | I2C1 时钟线 |
| **SDA** | PB7 | I2C1 数据线 |

> **注意**：SHT20 的 SCL/SDA 需要 4.7kΩ 上拉电阻到 3.3V（若 I2C 总线上已有上拉则可共用）。

## 4. 软件调试方案
通过在驱动层和启动阶段增加串口打印，实时观测数据，不依赖 UI 界面切换。

### A. 修改驱动文件：`Core/Src/bsp_sht20.c`
1. **启用调试开关**：在文件顶部添加宏定义：
   ```c
   #define SHT20_DEBUG_PRINTF
   #include <stdio.h>
   ```
2. **添加打印逻辑**：在 `BSP_SHT20_GetData()` 函数末尾嵌入输出：
   ```c
   #ifdef SHT20_DEBUG_PRINTF 
       printf("[SHT20] Raw: T=0x%04X RH=0x%04X | Temp=%.1fC Hum=%.1f%%\r\n", 
              pTem, pHum, gTemRH_Val.Tem, gTemRH_Val.Hum); 
   #endif 
   ```

### B. 修改主控文件：`Core/Src/main.c`
1. **添加声明**：在私有变量区添加温湿度结构体声明。
   ```c
   /* USER CODE BEGIN PV */
   extern volatile SHT20_TemRH_Val gTemRH_Val;
   /* USER CODE END PV */
   ```
2. **启动自检测试**：在 `USER CODE BEGIN 2` 末尾添加自检代码，确保开机即测试：
   ```c
   //=== SHT20 传感器测试 ===
   printf("[SYS] Boot OK, USART1 ready.\r\n");
   printf("[SHT20] Starting sensor test...\r\n");
   BSP_SHT20_GetData();  
   printf("[SHT20] Test done. See above for reading.\r\n");
   //=== 测试结束 ===
   ```

## 5. 数据流向图
`SHT20 硬件` → `I2C1(PB6/PB7)` → `BSP_SHT20_Read()` → `原始值` → `BSP_SHT20_GetData()` → `物理量转换` → `printf` → `USART1` → `PC 串口终端`

## 6. 验证步骤
1. **确认硬件**：VCC→3.3V, GND→GND, SCL→PB6, SDA→PB7（确保总线上有 4.7kΩ 上拉）。
2. **确认串口**：串口线连接 PA9 (USART1 TX)，波特率 115200。
3. **编译烧录**：使用 Keil MDK 编译下载。
4. **观察输出**：启动后应立即在串口终端看到：
   ```text
   [SYS] Boot OK, USART1 ready.
   [SHT20] Starting sensor test...
   [SHT20] Raw: T=0x6780 RH=0x5A20 | Temp=25.3C Hum=48.2%
   [SHT20] Test done. See above for reading.
   ```
5. **动态验证**：每 300ms 观察一次输出，用手捂热或哈气观察数值变化。

## 7. 串口无反应排查要点
| 原因 | 说明 |
| :--- | :--- |
| **初始化阻塞** | 若看不到 `[SYS] Boot OK`，说明启动阶段被其他 I2C 设备（如 MPU6050 或 AP3216C）初始化卡住，通常是总线挂死。 |
| **任务未开启** | `UPDATE_APP_TASK_EN` 仅在 AppPage 界面才置 1。现在通过 `main.c` 增加启动自检已规避此限制。 |
| **I2C 总线问题** | SHT20 未接好或缺失上拉电阻，会导致 I2C 命令超时（约 400ms 延迟）并返回无效数据。 |
| **接线错误** | 检查串口线是否接对 PA9，以及波特率是否为 115200。 |

## 8. 测试约束
- **无需修改**：I2C 初始化、任务调度架构、TouchGFX GUI 逻辑。
- **无需新建**：任何额外源文件。
