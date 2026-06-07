# ESP8266 OneNet MQTT 测试记录

**日期**: 2026/05/29  
**项目名称**: FS-STM32U575-Total TouchGFX + ESP8266 OneNet MQTT  
**硬件**: STM32U575 + ESP-12F (ESP8266)

---

## 一、功能实现概述

成功实现 ESP8266 连接 OneNet 云平台的 MQTT 数据上报功能，主要特性：
- 非阻塞式 OneNet 连接流程（状态机实现）
- 每 10 秒上报传感器数据（温度、湿度、心率、血氧、酒精、烟雾、CO2、驾驶时长）
- 15 秒超时机制：心率和血氧超过 15 秒无新数据自动归零
- 设备掉线自动重连

---

## 二、遇到的问题及解决方法

### 问题 1: 开机卡在 "[Model] Constructor OK"

**现象**: 程序启动后串口打印 `[Model] Constructor OK` 后停止响应。

**根因**: TouchGFX Designer 生成代码时会覆盖部分自定义修改，包括：
- SwipeContainer 大小被重置
- WiFiModalLink 组件 Z-order 被修改
- 部分事件回调丢失

**解决方法**: 每次使用 TouchGFX Designer 生成代码后，必须运行修复脚本：
```bash
cd TouchGFX
python fix_after_designer.py
```

**fix_after_designer.py 关键操作**:
- 恢复 SwipeContainer 为 240x200 像素
- 调整 WiFiModalLink 到顶层 Z-order
- 修复其他 Designer 覆盖的配置

---

### 问题 2: 连接 WiFi 卡住（Stage 2）

**现象**: OneNet 初始化卡在 Stage 2（`[ON] S2: WiFi>`），无法连接 WiFi。

**根因**: `HAL_GetTick()` 返回值异常，出现跳变（如 3 → 134245188），导致时间判断失效。

**解决过程**:
1. 尝试方案：改用循环计数器代替 `HAL_GetTick()`
   ```c
   static uint32_t loop_count = 0;
   loop_count++;
   if(gOneNet_InitStage == 2 && loop_count >= 2500) { ... }
   ```
   
2. 最终方案：使用阻塞式 AT 命令（内部用 `HAL_Delay`），配合 `HAL_GetTick()` 仅用于上报间隔

**最终代码策略**:
- WiFi 连接使用阻塞模式（保证可靠性）
- 数据上报间隔使用 `HAL_GetTick()` 非阻塞检测
- 状态机设计保留非阻塞架构

---

### 问题 3: 设备在线但数据不更新

**现象**: OneNet 网页显示设备在线，但数据流没有更新。

**根因分析**:
1. **ID 重复**: JSON 中的 `id` 字段固定为 `"123"`，OneNet 认为是重复消息而丢弃
2. **物模型不匹配**: 标识符、数据类型与网页端定义不一致

**解决方法**:
1. **动态 ID**: 每次上报使用递增 ID
   ```c
   static uint16_t msg_id = 1;
   sprintf(payload, "{\"id\":\"%d\",...}", msg_id++);
   ```

2. **统一标识符**: 确保代码中的 JSON 键名与 OneNet 物模型标识符完全一致

3. **检查物模型定义**:
   | 标识符 | 数据类型 | 说明 |
   |--------|----------|------|
   | `car_temp` | float | 车内温度(℃) |
   | `car_humidity` | int | 湿度(%RH) |
   | `heart_rate` | int | 心率(BPM) |
   | `spo2` | int | 血氧饱和度(%) |
   | `fatigue` | bool | 疲劳状态 |
   | `alcohol` | int | 酒精浓度 |
   | `co2` | int | CO2(ppm) |
   | `smoke` | int | 烟雾浓度 |
   | `drive_time` | int | 驾驶时长(分钟) |

---

### 问题 4: 心率和血氧数值异常

**现象**: 
- 心率显示 149（应为 75）
- 血氧显示 undefined 或不更新

**根因**: 
1. **心率未除 4**: MAX30102 的 `n_heart_rate` 需要除以 4 才是真实 BPM
2. **未检查有效标志**: 必须使用 `ch_hr_valid` 和 `ch_spo2_valid` 标志位判断数据有效性
3. **标志位未清零**: 读取数据后需要清零标志等待下次测量

**关键代码**:
```c
// 读取心率（带 15 秒超时，使用有效标志位，除以 4）
if(ch_hr_valid && n_heart_rate > 0 && n_heart_rate < 1200) {
    heart_rate = n_heart_rate / 4;  // 原始值/4 = 真实 BPM
    hr_last_valid_time = now;
    ch_hr_valid = 0;  // 读取后清零标志
} else if((now - hr_last_valid_time) > 15000) {
    heart_rate = 0;   // 超时归零
}

// 读取血氧（带 15 秒超时，使用有效标志位）
if(ch_spo2_valid && n_sp02 > 70 && n_sp02 <= 100) {
    spo2 = n_sp02;
    spo2_last_valid_time = now;
    ch_spo2_valid = 0;  // 读取后清零标志
} else if((now - spo2_last_valid_time) > 15000) {
    spo2 = 0;   // 超时归零
}
```

---

### 问题 5: 编译错误 - 类型冲突

**现象**: 
```
error: redefinition of 'car_temp' with a different type: 'float' vs 'int'
```

**根因**: `bsp_esp8266.h` 和 `bsp_esp8266.c` 中 `car_temp` 类型不一致。

**解决方法**: 统一头文件和源文件中的变量声明：
```c
// bsp_esp8266.h
extern float car_temp;
extern int spo2;  // 新增声明

// bsp_esp8266.c
float car_temp = 25.0f;
int spo2 = 0;
```

---

### 问题 6: 语法错误 - 大括号不匹配

**现象**: 
```
error: expected identifier or '('
```

**根因**: `simulate_sensor_data()` 函数中多了一个 `}`。

**解决方法**: 删除多余的大括号，确保函数结构正确。

---

## 三、关键配置参数

### OneNet 连接配置 (bsp_esp8266.h)
```c
#define ONENET_CLIENT_ID    "d1"
#define ONENET_PRODUCT_ID   "NRL824h77z"
#define ONENET_TOKEN        "version=2018-10-31&res=products%2FNRL824h77z%2Fdevices%2Fd1&et=1901881814&method=md5&sign=Ox25KvXCGh2d1WHaYWJHmA%3D%3D"
#define ONENET_SERVER_IP    "183.230.40.96"
#define ONENET_SERVER_PORT  1883
#define ONENET_TOPIC_PROP_POST  "$sys/NRL824h77z/d1/thing/property/post"
```

### WiFi 配置
```c
#define User_ESP8266_SSID   "Xiaomi_D5D9"
#define User_ESP8266_PWD    "huat@iae"
```

---

## 四、核心代码片段

### 4.1 结构体与外部声明
```c
// 外部传感器数据变量声明
extern volatile SHT20_TemRH_Val gTemRH_Val;
extern int32_t n_heart_rate;
extern int32_t n_sp02;
extern int8_t ch_hr_valid;       // 心率有效标志
extern int8_t ch_spo2_valid;     // 血氧有效标志
extern volatile SCD41_Data_t gSCD41_Val;

// 传感器数据变量
float car_temp = 25.0f;      // 车内温度
int car_humidity = 50;       // 车内湿度
int heart_rate = 0;          // 初始化0，测量后才更新
int spo2 = 0;                // 初始化0，测量后才更新
bool fatigue = false;
int alcohol = 5;
int co2 = 800;
int smoke = 5;
int drive_time = 0;

// 时间戳用于 15 秒超时归零
static uint32_t hr_last_valid_time = 0;
static uint32_t spo2_last_valid_time = 0;
#define HR_SPO2_TIMEOUT_MS  15000
```

### 4.2 非阻塞 OneNet 任务
```c
void OneNet_Report_Task(void)
{
    uint32_t now = HAL_GetTick();
    int32_t timediff;
    char payload[512];
    
    // 阶段0-5: 连接流程（AT测试、设置模式、连接WiFi、MQTT配置、连接OneNet）
    // ...
    
    // 阶段100: 定时上报
    if(gOneNet_InitStage == 100 && gOneNet_Connected) {
        timediff = (int32_t)(now - gOneNet_LastReportTime);
        if(timediff >= 10000) {  // 10秒间隔
            gOneNet_LastReportTime = now;
            simulate_sensor_data();
            build_onenet_payload(payload, sizeof(payload));
            OneNet_Publish_Property(ONENET_TOPIC_PROP_POST, payload, strlen(payload));
        }
    }
}
```

### 4.3 传感器数据读取
```c
void simulate_sensor_data(void)
{
    uint32_t now = HAL_GetTick();
    
    // 读取温湿度
    BSP_SHT20_GetData();
    HAL_Delay(20);
    if(gTemRH_Val.Tem > 0.1f && gTemRH_Val.Tem < 100.0f) {
        car_temp = gTemRH_Val.Tem;
        car_humidity = (int)gTemRH_Val.Hum;
    }
    
    // 心率（带超时和除4）
    if(ch_hr_valid && n_heart_rate > 0 && n_heart_rate < 1200) {
        heart_rate = n_heart_rate / 4;
        hr_last_valid_time = now;
        ch_hr_valid = 0;
    } else if((now - hr_last_valid_time) > HR_SPO2_TIMEOUT_MS) {
        heart_rate = 0;
    }
    
    // 血氧（带超时）
    if(ch_spo2_valid && n_sp02 > 70 && n_sp02 <= 100) {
        spo2 = n_sp02;
        spo2_last_valid_time = now;
        ch_spo2_valid = 0;
    } else if((now - spo2_last_valid_time) > HR_SPO2_TIMEOUT_MS) {
        spo2 = 0;
    }
    
    // MQ-3/MQ-2/SCD41...（略）
}
```

### 4.4 JSON 构建（动态 ID）
```c
void build_onenet_payload(char *payload, size_t payload_size)
{
    static uint16_t msg_id = 1;  // 递增消息ID避免重复
    sprintf(payload,
        "{\"id\":\"%d\",\"version\":\"1.0\",\"params\":"
        "\"car_humidity\":{\"value\":%d},"
        "\"car_temp\":{\"value\":%.1f},"   // float类型
        "\"fatigue\":{\"value\":%s},"
        "\"heart_rate\":{\"value\":%d},"
        "\"spo2\":{\"value\":%d},"
        "\"alcohol\":{\"value\":%d},"
        "\"co2\":{\"value\":%d},"
        "\"smoke\":{\"value\":%d},"
        "\"drive_time\":{\"value\":%d}"
        "}}",
        msg_id++,
        car_humidity,
        car_temp,
        fatigue ? "true" : "false",
        heart_rate,
        spo2,
        alcohol,
        co2,
        smoke,
        drive_time
    );
}
```

### 4.5 MQTT 发布（RAW 模式）
```c
bool OneNet_Publish_Property(char * Topic, char * JSON_Payload, uint16_t Len)
{
    char cCmd [256];
    HAL_Delay(500);
    sprintf(cCmd, "AT+MQTTPUBRAW=0,\"%s\",%d,0,0", Topic, Len);
    
    if(ESP8266_Send_AT_Cmd(cCmd, ">", NULL, 2000)) {
        ESP8266_USART("%s", JSON_Payload);
        HAL_Delay(500);
        return true;
    }
    return false;
}
```

---

## 五、调试建议

1. **查看串口输出**: 开启 `[ON] REP> ` 开头的日志，确认 JSON 格式正确
2. **Web 端日志**: OneNet Studio → 设备调试 → 查看上报日志和错误提示
3. **验证物模型**: 确保所有上报字段已在 OneNet 产品物模型中定义
4. **测试命令**: 用串口工具手动发送 AT 命令验证 ESP8266 响应

---

## 六、已知限制

- 心率/血氧 15 秒后归零，需重新测量
- ESP8266 连接失败后需从 Stage 3（MQTTCLEAN）重试，约 20 秒周期
- 数据上报间隔固定 10 秒，不可动态调整

---

## 附录：文件修改清单

| 文件 | 修改内容 |
|------|----------|
| `Core/Inc/bsp_esp8266.h` | 添加 OneNet 配置宏、外部变量声明（包括 spo2） |
| `Core/Src/bsp_esp8266.c` | OneNet MQTT 功能实现、非阻塞任务、数据读取、JSON 构建 |
| `Core/Src/main.c` | 初始化调用 `OneNet_Init_Start()`、主循环调用 `OneNet_Report_Task()` |
| `TouchGFX/fix_after_designer.py` | Designer 代码生成后修复脚本 |

---

**文档结束**
