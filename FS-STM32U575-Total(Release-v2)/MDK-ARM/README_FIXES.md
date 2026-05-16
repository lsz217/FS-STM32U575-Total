# STM32U575 TouchGFX 项目问题修复记录 (2026-05-04)

本文档记录了项目开发过程中遇到的关键技术问题及其解决方案，供后续参考。

## 1. 编译错误：找不到头文件 ('max30102.h' file not found)
- **现象**：由于项目中集成了 MAX30102、MPU6050 等驱动，编译时提示头文件缺失。
- **原因**：Keil 项目配置中的 `Include Paths` 未包含这些驱动的文件夹。
- **解决方法**：在 `Options for Target -> C/C++ -> Include Paths` 中添加了以下路径：
    - `../Drivers/MAX30102_Maxim`
    - `../Drivers/Motion_Driver`
    - `../Drivers/efsm-master/efsm`
- **关联修改**：同步在项目分组中添加了对应的 `.c` 源文件。

## 2. 链接错误：变量重复定义 (Symbol gBacklightVal multiply defined)
- **现象**：链接阶段报错，提示 `gBacklightVal` 在 `Model.o` 和 `user_app.o` 中均有定义。
- **原因**：TouchGFX 的 `Model.cpp` 默认定义了该变量用于仿真，而硬件驱动 `user_app.c` 中也定义了它。
- **解决方法**：在全局宏定义（Define）中添加了 `LINK_HARDWARE`。
- **原理**：该宏会触发 `Model.cpp` 中的条件编译，将 `gBacklightVal` 声明为 `extern`，从而引用硬件层定义的变量。

## 3. 内存分配错误：执行区空间不足 (L6406E: No space in execution regions)
- **现象**：由于图片和字体资源过多，内部 Flash (2MB) 溢出。
- **原因**：TouchGFX 资产默认存放在内部 Flash。
- **解决方法**：使用自定义分散加载文件 `My_Fixed_Layout.sct`。
- **具体操作**：将 `ExtFlashSection`、`FontFlashSection` 和 `TextFlashSection` 映射到外部 OSPI Flash 地址 `0x90000000`。

## 4. 下载错误：无法加载 Flash 算法 (Cannot Load Flash Programming Algorithm!)
- **现象**：点击 Download 时弹出该错误，无法烧录外部 Flash。
- **原因**：`.uvoptx` 配置文件中缺失或错误配置了外部 Flash 的 `.flm` 算法路径。
- **解决方法**：
    1. 在 `Debug` -> `Settings` -> `Flash Download` 配置中，手动添加 D 盘下的算法文件。
    2. 修正 `.uvoptx` 文件中的 `-FP1` 参数为：`D:\Keil5\Core\ARM\flash\HQYJ-U575RI-W25Q128-QSPI.flm`。
    3. **注意细节**：修正了 `-FF1` 参数后的冗余 `.flm` 后缀，并将算法运行 RAM 地址修正为 `0x20000000`。

## 5. 硬件通信异常：屏幕显示卡顿或加载失败 (GPDMA1 通道优先级冲突)
- **现象**：屏幕硬件正常但显示异常，SPI 通信超时，数据传输失败。
- **原因**：STM32U5 的 GPDMA1 采用固定优先级（通道号越小优先级越高）。原配置中屏幕 SPI1_TX 位于 Channel 4，而串口 DMA 占用 Channel 0-3，导致高频串口数据持续抢占 DMA 资源。
- **解决方法**：调整 DMA 通道分配：
    - 将 **SPI1_TX** 分配至 **Channel 0**（最高优先级），确保屏幕数据优先传输。
    - 将 ADC1 采集分配至 Channel 1。
    - 禁用不必要的串口 DMA 通道，减少总线竞争。
- **结果**：SPI 屏幕恢复正常工作，图片与界面加载流畅，无卡顿或超时现象。

## 6. 功能逻辑错误：NFC 读卡后界面不跳转 (中断引脚与触发配置错误)
- **现象**：刷卡（NFC 感应）成功后，TouchGFX 界面不跳转。
- **原因**：
    - **引脚绑定错误**：误将 PA0（实际为五向按键）配置为 NFC 中断脚。
    - **触发电平不匹配**：NFC 模块 IRQ 为低电平有效，原配置误用上升沿触发。
    - **数据链路断裂**：UART4 DMA+IDLE 接收未正常联动，导致 NFC 数据解析异常。
- **解决方法**：
    - **修正引脚**：将代码中的中断引脚修改为 NFC 实际对应的硬件引脚。
    - **修正触发**：改为 **下降沿触发**（Falling Edge），并在 `HAL_GPIO_EXTI_Falling_Callback` 中处理感应逻辑。
    - **开启 DMA 接收**：初始化时调用 `HAL_UARTEx_ReceiveToIdle_DMA`，确保数据完整解析。
    - **逻辑联动**：在 TouchGFX 中增加对 `g_nfc_unlock_flag` 的判断，读卡成功后自动触发页面跳转。
- **结果**：NFC 刷卡识别后界面自动跳转，业务逻辑正常。

## 7. 环境同步建议
- **问题**：手动修改 `.uvprojx` 或 `.uvoptx` 后，Keil 界面可能不会立即刷新，甚至会被 Keil 自身的缓存覆盖。
- **建议操作**：在外部修改配置文件后，**必须先关闭 Keil，再重新打开项目**，以确保所有底层配置（尤其是 Flash 算法路径）生效。
