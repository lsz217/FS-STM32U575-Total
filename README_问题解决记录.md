# STM32U575 + ESP8266 传感器融合项目 —— 问题解决记录

## 项目概述

基于 STM32U575 的 TouchGFX 智能传感器融合系统，集成 MAX30102 心率血氧、ESP8266 WiFi（OneNet MQTT 上报）、SHT20 温湿度、MPU6050 六轴姿态等多种传感器。

---

## 问题 1：手指放在传感器上但检测不到

**现象**：手指紧贴 MAX30102，串口始终打印 `No finger detected`，系统无法进入测量流程。

**根因**：`max30102.c` 中 `REG_FIFO_CONFIG` 配置为 `0x4f`，bit4=0 表示 FIFO 满后停止（不翻转）。MAX30102 的 FIFO 只有 32 采样深度，4 采样平均 + 100Hz 采样率 = 25Hz 有效输出，FIFO 在 1.28 秒内填满并停止。3 秒检测间隔内新数据全部丢失，`max30102_check_finger()` 读到的一直是 FIFO 中的旧数据或空数据。

**解决**：将 `REG_FIFO_CONFIG` 的 bit4 置 1（翻转使能）。

**文件**：[Drivers/MAX30102_Maxim/max30102.c:143](FS-STM32U575-Total(Release-v2)/Drivers/MAX30102_Maxim/max30102.c#L143)

```c
// 修改前
if(!maxim_max30102_write_reg(REG_FIFO_CONFIG,0x4f))

// 修改后
if(!maxim_max30102_write_reg(REG_FIFO_CONFIG,0x5f))  //sample avg=4, fifo rollover=TRUE
```

**效果**：FIFO 满后自动覆盖旧数据，手指检测立即生效。

---

## 问题 2：系统运行到第 10 次左右卡死

**现象**：心率测量进行到 id ≈ 10~16 时系统完全卡死，再无任何输出。

**根因**：`user_app.c` 中 FIFO 等待超时使用**计数方式**（5000 次循环），而非时间方式。MAX30102 与 FT6336 触摸控制器共享 I2C1 总线，当触摸控制器占用总线时，MAX30102 的 I2C 读取会持续失败，但循环计数照增。5000 次 × 每次约 10ms = 50 秒/采样，100 个采样 ≈ 5000 秒，表现为系统卡死。

**解决**：将 `FIFO_WAIT_TIMEOUT` 替换为 `FIFO_WAIT_TIMEOUT_MS 100`（100ms 时间基准，使用 `HAL_GetTick()`）。

**涉及 3 个 while 循环**（[user_app.c:724](FS-STM32U575-Total(Release-v2)/Core/Src/user_app.c#L724)）：

| 位置 | 函数 | 修改 |
|------|------|------|
| [user_app.c:753](FS-STM32U575-Total(Release-v2)/Core/Src/user_app.c#L753) | `max30102_check_finger()` | ✅ 已改为时间基准 |
| [user_app.c:796](FS-STM32U575-Total(Release-v2)/Core/Src/user_app.c#L796) | `mpu_get_max30102_data()` 滚动读取 | ✅ 已改为时间基准 |
| [user_app.c:881](FS-STM32U575-Total(Release-v2)/Core/Src/user_app.c#L881) | `Update_HeartRateInfo()` 初始 200 采样 | ✅ 已改为时间基准 |

**修改前**（计数方式）：
```c
#define FIFO_WAIT_TIMEOUT 5000

timeout = 0;
while (((uch_dummy & 0xC0) == 0x00) && timeout < FIFO_WAIT_TIMEOUT)
{
    maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_dummy);
    timeout++;
}
```

**修改后**（时间基准）：
```c
#define FIFO_WAIT_TIMEOUT_MS 100

uint32_t t_start = HAL_GetTick();
while (((uch_dummy & 0xC0) == 0x00) && (HAL_GetTick() - t_start < FIFO_WAIT_TIMEOUT_MS))
{
    maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_dummy);
}
if (HAL_GetTick() - t_start >= FIFO_WAIT_TIMEOUT_MS)
{
    printf("[HR] FIFO timeout at sample %d\r\n", i);
    break;
}
```

**效果**：I2C 故障时最多等待 100ms 即超时跳出，系统不再卡死。

---

## 问题 3：手指离开 10 秒后屏幕显示 "0" 而非 "--"

**现象**：手指离开传感器 10 秒后，屏幕心率/血氧显示 "0"，与初始占位符不一致。

**根因**：`user_app.c` 中超时后 `n_heart_rate=0, n_sp02=0`，Model.cpp 将 `0` 直接传给 View，View 将其当数字显示。

**解决**：Model.cpp 检测到值为 0 时，传递 `0xFFFFFFFF` 作为"重置信号"；View 收到此信号后恢复 wildcard 文本（Designer 中设置的初始占位符）。

**文件**：
- [Model.cpp:183-185](FS-STM32U575-Total(Release-v2)/TouchGFX/gui/src/model/Model.cpp#L183-L185)
- [ApplicationPageView.cpp:169-189](FS-STM32U575-Total(Release-v2)/TouchGFX/gui/src/applicationpage_screen/ApplicationPageView.cpp#L169-L189)

```cpp
// Model.cpp
modelListener->updateHeartRateInfo(
    (n_heart_rate > 0) ? (uint32_t)(n_heart_rate / 4) : 0xFFFFFFFF,
    (n_sp02 > 0)     ? (uint32_t)n_sp02             : 0xFFFFFFFF);

// ApplicationPageView.cpp
if (newHeartRate == 0xFFFFFFFF)
{
    Unicode::snprintf(textPulseBuffer, TEXTPULSE_SIZE, "%s",
        touchgfx::TypedText(T___SINGLEUSE_XI1E).getText());
}
else
{
    Unicode::snprintf(textPulseBuffer, TEXTPULSE_SIZE, "%d", newHeartRate);
}
```

---

## 问题 4：OneNet 上传值始终 ≥ 40，超时后上传 40 而非 0

**现象**：手指离开 10 秒后，OneNet 平台显示心率 = 40，而非期望的 0。

**根因**：`bsp_esp8266.c` 中 `build_onenet_payload()` 对心率做了下限钳位：`(heart_rate < 40) ? 40 : heart_rate`。

**解决**：移除钳位逻辑，直接上传原始值。

**文件**：[bsp_esp8266.c:550-554](FS-STM32U575-Total(Release-v2)/Core/Src/bsp_esp8266.c#L550-L554)

```c
// 修改前
"\"heart_rate\":{\"value\":%d},"
// 实际传入: (heart_rate < 40) ? 40 : heart_rate

// 修改后
"\"heart_rate\":{\"value\":%d},"
// 实际传入: heart_rate（原始值，含 0）
```

---

## 问题 5：温度变量遮蔽（Shadowing）导致全局变量未更新

**现象**：温度读数始终不正确，芯片温度未能更新。

**根因**：`user_app.c` 中第 819 行使用了 `float n_temperature = ...`，局部变量遮蔽了同名的全局变量 `n_temperature`。

**解决**：移除 `float` 类型声明。

**文件**：[user_app.c:819](FS-STM32U575-Total(Release-v2)/Core/Src/user_app.c#L819)

```c
// 修改前
float n_temperature = integer_temperature + ((float)fractional_temperature) / 16.0f;

// 修改后
n_temperature = integer_temperature + ((float)fractional_temperature) / 16.0f;
```

---

## 问题 6：首次测量后卡死（计算完成但无后续输出）

**现象**：串口打印 `[HR] Calculated: HR=35, SpO2=-999, valid=1/0` 后系统无响应。

**根因**：计算完成后 `mpu_get_max30102_data()` 尝试读取 100 个采样做滚动窗口，但 FIFO 刚被初始 200 采样读空，补满 100 个需等待 4~10 秒，期间无任何输出，表现为卡死。**首次测量不存在"旧数据"，滚动读取无意义**。

**解决**：首次测量周期跳过 `mpu_get_max30102_data()`，直接重置 FIFO。后续测量正常执行滚动读取。

**文件**：[user_app.c:911-925](FS-STM32U575-Total(Release-v2)/Core/Src/user_app.c#L911-L925)

```c
// 首次测量跳过滚动采集：FIFO刚被读空，再读100点耗时数秒无意义
{
    static uint8_t first_measurement = 1;
    if (!first_measurement)
    {
        printf("[HR] Reading 100 rolling samples...\r\n");
        mpu_get_max30102_data();
        printf("[HR] Rolling read done\r\n");
    }
    else
    {
        first_measurement = 0;
        printf("[HR] First measurement, skip rolling read\r\n");
    }
}
```

---

## 问题 7：算法返回无效 SpO2（-999）和不可信 HR（35）

**现象**：首次计算后 `n_sp02=-999, n_heart_rate=140(÷4=35BPM)`，SpO2 无效但 HR 被判定为有效，屏幕显示半可靠数据。

**根因**：MAX30102 算法在信号质量不足时对 SpO2 返回 -999（无效标记），对 HR 可能基于部分峰值给出低可信度结果。单一有效不应视为可靠测量。

**解决**：任一无效则两者均清零，等待下次完整测量。Model 检测到 0 值传递 `0xFFFFFFFF`，View 显示 "--"。

**文件**：[user_app.c:898-903](FS-STM32U575-Total(Release-v2)/Core/Src/user_app.c#L898-L903)

```c
// 任一无效则两者都清零：部分结果不可靠，等下次完整测量
if (!ch_hr_valid || !ch_spo2_valid)
{
    n_heart_rate = 0;
    n_sp02 = 0;
}
```

---

## 问题 8：OneNet 频繁重复上报相同数据

**现象**：手指持续放置时，每约 10 秒上报一次相同的心率/血氧值，浪费流量和服务器资源。

**根因**：无变化过滤，每次 `simulate_sensor_data()` 都直接用新值覆盖上传缓存。

**解决**：添加死区过滤器（Dead-band Filter），仅当新值与上次上传值相差超过 ±5 时才更新，超时归零时强制更新。

**文件**：[bsp_esp8266.c:493-530](FS-STM32U575-Total(Release-v2)/Core/Src/bsp_esp8266.c#L493-L530)

```c
static int last_uploaded_hr = 0;
static int last_uploaded_spo2 = 0;

// 心率：每次有效读数刷新时间戳，变化超过±5才更新上传值
if(n_heart_rate > 0 && n_heart_rate < 1200)
{
    int new_hr = n_heart_rate / 4;
    hr_last_valid_time = now;  // 时间戳每次刷新，与上传无关
    if(abs(new_hr - last_uploaded_hr) > 5)
    {
        heart_rate = new_hr;
        last_uploaded_hr = new_hr;
    }
}
else if((now - hr_last_valid_time) > HR_SPO2_TIMEOUT_MS)
{
    if(heart_rate != 0)
    {
        heart_rate = 0;
        last_uploaded_hr = 0;  // 归零时强制更新
    }
}
// SpO2 同理
```

**效果**：稳定状态下不重复上报，数值变化超过 5 或超时归零时才触发上传。

---

## 问题 9：ESP8266 SNTP 时间解析失败（多次迭代）

**现象**：ESP8266 通过 `AT+CIPSNTPTIME?` 成功获取 NTP 时间（串口可见 `Sun Jun 14 21:08:28 2026`），但解析器持续输出 `mon= day=0 00:00:00 year=0`，时间未能写入 RTC，HomePage 时间显示不更新。

**根因（多个）**：

1. **`p += 13` 正确但被误改为 `p += 14`**：`"+CIPSNTPTIME:"` 是 13 个字符（+ C I P S N T P T I M E :），不是 14。
2. **Keil MicroLib 不支持 `sscanf`**：MicroLib 仅提供 printf/sprintf 等输出函数，`sscanf` 不可用，调用后变量保持为 0。
3. **`strncmp` 在 MicroLib 中行为不可靠**：逐字符直接比较 `p[0]=='J' && p[1]=='u' && p[2]=='n'` 更稳定。
4. **星期几跳过逻辑错误**：代码假设第一个词总是星期几（如 `Sun`），但 ESP8266 响应有时不含星期几（`Jun 14 20:49:51 2026`），导致月份被当作星期几跳过。

**原始响应格式**（两种都有）：

```
+CIPSNTPTIME:Sun Jun 14 21:08:28 2026    ← 含星期几
+CIPSNTPTIME:Jun 14 20:49:51 2026        ← 不含星期几
```

AT 命令回显 `AT+CIPSNTPTIME?` 也在接收缓冲区中，但 `strstr` 不会误匹配（回显以 `?` 结尾，搜索串以 `:` 结尾）。

**解决**：

1. 使用 `p += 13` 正确跳过前缀
2. 用 `*p >= '0' && *p <= '9'` 判断第一个词是否是数字，不是数字则跳过（星期几）
3. 用直接字符比较识别月份（`p[0]=='J' && p[1]=='a' && p[2]=='n'` ...）
4. 用 `strtol` 手动解析数字（MicroLib 支持）
5. 添加调试打印 `printf("[ON] SNTP raw: [%s]\r\n", buf)` 查看原始缓冲区

**文件**：[bsp_esp8266.c:856-910](FS-STM32U575-Total(Release-v2)/Core/Src/bsp_esp8266.c#L856-L910)

**最终解析代码**：

```c
case OS_WAIT_SNTP_QUERY: {
    r = ESP8266_NB_AT_Poll();
    if (r == 0) return;
    if (r == 1) {
        char *buf = (char*)ESP8266_Fram_Record_Struct.Data_RX_BUF;
        printf("[ON] SNTP raw: [%s]\r\n", buf);
        char *p = strstr(buf, "+CIPSNTPTIME:");
        if (p) {
            p += 13; // skip "+CIPSNTPTIME:" (13 chars)
            while (*p == ' ') p++;
            // 如果第一个词不是数字，跳过（星期几）
            if (!(*p >= '0' && *p <= '9')) {
                while (*p && *p != ' ') p++;
                while (*p == ' ') p++;
            }
            // 直接字符比较识别月份
            int m = 0;
            if      (p[0]=='J' && p[1]=='a' && p[2]=='n') m=1;
            else if (p[0]=='F' && p[1]=='e' && p[2]=='b') m=2;
            else if (p[0]=='M' && p[1]=='a' && p[2]=='r') m=3;
            else if (p[0]=='A' && p[1]=='p' && p[2]=='r') m=4;
            else if (p[0]=='M' && p[1]=='a' && p[2]=='y') m=5;
            else if (p[0]=='J' && p[1]=='u' && p[2]=='n') m=6;
            else if (p[0]=='J' && p[1]=='u' && p[2]=='l') m=7;
            else if (p[0]=='A' && p[1]=='u' && p[2]=='g') m=8;
            else if (p[0]=='S' && p[1]=='e' && p[2]=='p') m=9;
            else if (p[0]=='O' && p[1]=='c' && p[2]=='t') m=10;
            else if (p[0]=='N' && p[1]=='o' && p[2]=='v') m=11;
            else if (p[0]=='D' && p[1]=='e' && p[2]=='c') m=12;
            if (m > 0) {
                p += 3; while (*p == ' ') p++;
                char *e;
                int day  = (int)strtol(p, &e, 10); p = e; while (*p == ' ') p++;
                int hour = (int)strtol(p, &e, 10); p = (*e == ':') ? e + 1 : e;
                int min  = (int)strtol(p, &e, 10); p = (*e == ':') ? e + 1 : e;
                int sec  = (int)strtol(p, &e, 10); p = e; while (*p == ' ') p++;
                int year = (int)strtol(p, NULL, 10);
                if (year >= 1970 && day >= 1 && day <= 31) {
                    RTC_TimeTypeDef sTime = {0};
                    RTC_DateTypeDef sDate = {0};
                    sTime.Hours = hour; sTime.Minutes = min; sTime.Seconds = sec;
                    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
                    sTime.StoreOperation = RTC_STOREOPERATION_SET;
                    sDate.WeekDay = 1; sDate.Month = m;
                    sDate.Date = day; sDate.Year = (uint8_t)(year - 2000);
                    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                    printf("[ON] RTC set OK: %02d/%02d/%04d %02d:%02d:%02d\r\n",
                           m, day, year, hour, min, sec);
                }
            }
        }
    }
    os = OS_CONNECTED;
    os_last_report = HAL_GetTick();
    return;
}
```

---

## 时间显示流程（ESP8266 → RTC → HomePage）

```
ESP8266 SNTP 时间串
  → 解析器提取 month/day/hour/min/sec/year
  → HAL_RTC_SetTime / HAL_RTC_SetDate 写入硬件 RTC
  → Update_System_Time() (user_app.c, 每1秒) 读 RTC → gSystemTime / gSystemDate
  → Model::tick() (Model.cpp) 检测秒变化 → modelListener->updateTime/updateDate
  → HomePagePresenter → HomePageView
  → textSystemClock (时:分) / textClockSecond (秒) / textSystemYear (年) / textSystemDate (月/日)
```

**关键点**：SNTP 在 init 流程中（MQTT CONNECT 成功之后），只执行一次。RTC 设置后由硬件自动走时，无需反复查询 ESP8266。

---

## 修改文件清单

| 文件 | 修改内容 |
|------|---------|
| `Drivers/MAX30102_Maxim/max30102.c` | FIFO 翻转使能 (`0x4f` → `0x5f`) |
| `Core/Src/user_app.c` | 时间基准超时、温度遮蔽修复、无效值清零、首次测量跳过滚动读取 |
| `Core/Src/bsp_esp8266.c` | 移除 40 下限钳位、添加 HR/SpO2 死区过滤、**SNTP 时间解析器重写** |
| `TouchGFX/gui/src/model/Model.cpp` | 无效值传递 `0xFFFFFFFF` 信号 |
| `TouchGFX/gui/src/applicationpage_screen/ApplicationPageView.cpp` | 接收 `0xFFFFFFFF` 后恢复 wildcard 文本 |

---

## 关键设计原则

1. **I2C 共享总线**：MAX30102 与 FT6336 共享 I2C1，所有 I2C 操作必须有时间上限，禁止无限等待。
2. **FIFO 翻转必须开启**：连续测量场景下 FIFO 满后必须覆盖旧数据，否则传感器"静默"。
3. **无效数据不展示**：传感器算法可能返回部分有效的结果，应全部拒绝，避免误导。
4. **上传做死区过滤**：IoT 场景下频繁重复上报浪费资源，变化阈值过滤是标准做法。
5. **超时归零与正常归零区分**：使用 `0xFFFFFFFF` 作为"重置为默认显示"的信号，与数值 0 区分开。
