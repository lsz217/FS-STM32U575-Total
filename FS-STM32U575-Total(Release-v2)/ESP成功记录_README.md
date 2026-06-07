# ESP8266 OneNet MQTT 接入成功记录

**日期**: 2026/05/30  
**项目名称**: FS-STM32U575-Total TouchGFX + ESP8266 OneNet MQTT  
**状态**: ✅ 已成功解决所有问题，数据正常上报

---

## 一、项目概述

成功实现 STM32U575 + ESP8266 连接 OneNet 云平台，完成传感器数据实时上报。
- 上报数据：车内温度/湿度、心率、血氧、酒精浓度、CO2、烟雾、驾驶时长
- 上报频率：每 10 秒一次
- 连接方式：MQTT over TCP (OneNet Studio)

---

## 二、问题现象

设备显示在线，MQTT 发布返回 `+MQTTPUB:OK`，但 OneNet 网页端数据始终不更新。

---

## 三、根本原因（层层递进）

### 3.1 时序混乱 - HAL_GetTick() 异常
- **现象**: `timediff=-37056`（时间倒跳，负数）
- **原因**: `HAL_GetTick()` 返回值不稳定，出现跳变（3→134245188）
- **解决**: 改用**循环计数器**方案，彻底避开时间戳问题

### 3.2 JSON 格式错误
- **现象**: 物模型日志 `301 json format error`
- **原因**: `sprintf` 拼接时 `"params":` 与 `"car_humidity"` 之间缺少 `{`
- **解决**: 修正为 `"params":{`

### 3.3 数据类型不匹配（致命）
- **现象**: 日志 `json format error` 但检查不出语法错误
- **原因**: 
  - 代码: `int car_humidity = 50;`
  - 物模型: `car_humidity` 定义为 **float** 类型
  - JSON: `"car_humidity":{"value":50}` （OneNet 解析 int 到 float 字段失败）
- **解决**: `int` → `float`，格式化 `%d` → `%.1f`

### 3.4 数据范围超限
- **现象**: 日志 `check_property:int32 over range:identifier:heart_rate`
- **原因**: `heart_rate=0` 小于物模型定义的最小值 `40`
- **解决**: 代码限幅: `if(hr < 40) hr = 40;` 或修改物模型 min=0

### 3.5 消息 ID 重复
- **现象**: 偶发数据不更新
- **原因**: 固定 `id="123"` 被 OneNet 视为重复消息丢弃
- **解决**: 递增 ID: `static uint16_t msg_id = 1; ... msg_id++`

---

## 四、最终解决方案

### 4.1 方案选择：循环计数器（替代 HAL_GetTick）

```c
// 简化的非阻塞任务 - 基于循环计数器（避免 HAL_GetTick 异常）
static uint32_t loop_counter = 0;

void OneNet_Report_Task(void)
{
    loop_counter++;
    char payload[1024];

    // S0: AT测试 (循环1)
    if(gOneNet_InitStage == 0 && loop_counter == 1)
    {
        ESP8266_Send_AT_Cmd("AT", "OK", NULL, 100);
        gOneNet_InitStage = 1;
        return;
    }

    // S1: 设置模式 (循环500 ≈ 1秒)
    if(gOneNet_InitStage == 1 && loop_counter >= 500)
    {
        ESP8266_Send_AT_Cmd("AT+CWMODE=1", "OK", "no change", 500);
        gOneNet_InitStage = 2;
        return;
    }

    // S2: 连接WiFi (循环1000)
    if(gOneNet_InitStage == 2 && loop_counter >= 1000)
    {
        ESP8266_JoinAP(User_ESP8266_SSID, User_ESP8266_PWD);
        gOneNet_InitStage = 3;
        return;
    }

    // S3: 清理MQTT (循环8000 ≈ WiFi稳定后)
    if(gOneNet_InitStage == 3 && loop_counter >= 8000)
    {
        OneNet_MQTT_Clear();
        gOneNet_InitStage = 4;
        return;
    }

    // S4: 配置MQTT (循环8500)
    if(gOneNet_InitStage == 4 && loop_counter >= 8500)
    {
        OneNet_MQTT_Config(ONENET_CLIENT_ID, ONENET_PRODUCT_ID, ONENET_TOKEN);
        gOneNet_InitStage = 5;
        return;
    }

    // S5: 连接OneNet (循环10000)
    if(gOneNet_InitStage == 5 && loop_counter >= 10000)
    {
        if(OneNet_MQTT_Connect(ONENET_SERVER_IP, ONENET_SERVER_PORT))
        {
            gOneNet_Connected = 1;
            gOneNet_InitStage = 100;
        }
        else
        {
            gOneNet_InitStage = 3;  // 失败重试
            loop_counter = 8000;
        }
        return;
    }

    // S100: 已连接，每5000循环上报一次(约10秒)
    if(gOneNet_InitStage == 100 && gOneNet_Connected)
    {
        if(loop_counter % 5000 == 0)
        {
            simulate_sensor_data();
            build_onenet_payload(payload, sizeof(payload));
            OneNet_Publish_Property(ONENET_TOPIC_PROP_POST, payload, strlen(payload));
        }
    }
}
```

### 4.2 数据类型统一

**头文件声明** (`bsp_esp8266.h`):
```c
extern float car_temp;       // ✅ float 匹配物模型
extern float car_humidity;   // ✅ float 匹配物模型
extern int heart_rate;       // ✅ int32 匹配物模型
extern int spo2;             // ✅ int32 匹配物模型
// ... 其他变量
```

**变量定义** (`bsp_esp8266.c`):
```c
float car_temp = 25.0f;      // float 类型
float car_humidity = 50.0f;  // float 类型
int heart_rate = 0;          // int 类型
int spo2 = 0;                // int 类型
```

**数据读取**:
```c
car_temp = gTemRH_Val.Tem;        // 直接赋值 float
car_humidity = gTemRH_Val.Hum;    // 直接赋值 float，不再 (int) 转换
```

### 4.3 JSON 构建（完整正确版）

```c
void build_onenet_payload(char *payload, size_t payload_size)
{
    static uint16_t msg_id = 1;
    
    // 数据范围限幅（防止超出物模型定义导致整包被拒）
    int hr = heart_rate;
    if(hr < 40) hr = 40;      // 物模型 min=40
    if(hr > 220) hr = 220;    // 物模型 max=220
    
    int sp = spo2;
    if(sp < 0) sp = 0;
    if(sp > 200) sp = 200;
    
    // 注意：car_humidity 是 float，必须用 %.1f
    snprintf(payload, payload_size,
        "{\"id\":\"%d\",\"version\":\"1.0\",\"params\":{"
        "\"alcohol\":{\"value\":%d},"
        "\"car_humidity\":{\"value\":%.1f},"  // ✅ %.1f 匹配 float
        "\"car_temp\":{\"value\":%.1f},"
        "\"co2\":{\"value\":%d},"
        "\"drive_time\":{\"value\":%d},"
        "\"fatigue\":{\"value\":%s},"
        "\"heart_rate\":{\"value\":%d},"
        "\"smoke\":{\"value\":%d},"
        "\"spo2\":{\"value\":%d}"
        "}}",
        msg_id++,
        alcohol,
        car_humidity,    // float 类型
        car_temp,        // float 类型
        co2,
        drive_time,
        fatigue ? "true" : "false",
        hr,              // 限幅后的值
        smoke,
        sp
    );
}
```

### 4.4 MQTT 发布（RAW 模式）

```c
bool OneNet_Publish_Property(char *Topic, char *JSON_Payload, uint16_t Len)
{
    char cCmd[512];
    
    HAL_Delay(200);
    sprintf(cCmd, "AT+MQTTPUBRAW=0,\"%s\",%d,1,0", Topic, Len);
    
    // Step 1: 发送命令，等待 ">"
    ESP8266_Clear_Buffer();
    ESP8266_USART("%s\r\n", cCmd);
    HAL_Delay(1000);
    
    if(!strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, ">"))
        return false;
    
    // Step 2: 发送 payload（不带 \r\n）
    ESP8266_Clear_Buffer();
    ESP8266_USART("%s", JSON_Payload);
    HAL_Delay(1000);
    
    // Step 3: 检查 SEND OK
    if(strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, "SEND OK") ||
       strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, "MQTTPUB:OK"))
        return true;
    
    return false;
}
```

---

## 五、OneNet 物模型配置（关键对应关系）

| 标识符 | 数据类型 | 取值范围 | 步长 | 代码变量类型 |
|-------|---------|---------|------|-------------|
| `car_temp` | **float** | -60.0 ~ 60.0 | - | `float` |
| `car_humidity` | **float** | 0.0 ~ 120.0 | - | `float` |
| `heart_rate` | **int32** | 40 ~ 220 | 1 | `int` ⚠️ 0需转40 |
| `spo2` | **int32** | 0 ~ 200 | - | `int` |
| `alcohol` | **int32** | 0 ~ 200 | 1 | `int` |
| `co2` | **int32** | 0 ~ 5000 | 1 | `int` |
| `smoke` | **int32** | 0 ~ 1000 | 1 | `int` |
| `drive_time` | **int32** | 0 ~ 1440 | 1 | `int` |
| `fatigue` | **bool** | true/false | - | `bool` |

**⚠️ 必须确保**：代码变量类型、JSON 格式化符、物模型数据类型三者完全一致！

---

## 六、调试方法与日志解读

### 6.1 查看 OneNet 设备日志
**路径**: OneNet Studio → 设备 → 设备调试 → 日志

| 日志内容 | 含义 | 解决方向 |
|---------|------|---------|
| `301 json format error` | JSON 语法错误或类型不匹配 | 检查括号匹配、引号转义、类型一致性 |
| `int32 over range:identifier:xxx` | 数值超出物模型范围 | 修改代码限幅或物模型范围 |
| `property not found:xxx` | 标识符拼写不一致 | 核对大小写、下划线位置 |
| `+MQTTPUB:OK` | MQTT 层成功（仅表示收到） | **不代表物模型解析成功**，继续查上层错误 |

### 6.2 串口调试输出
在关键节点添加打印：
```c
printf("[JSON] len=%d: %s\r\n", strlen(payload), payload);
```

**正确的 JSON 示例**:
```json
{"id":"5","version":"1.0","params":{
  "car_temp":{"value":25.5},
  "car_humidity":{"value":50.0},
  "heart_rate":{"value":75}
}}
```

**常见错误 JSON**:
```json
{"id":"123","params":"car_temp":{...}}  // params 后缺 {
{"car_humidity":{"value":50}}               // car_humidity 是 int 50，但物模型要 float 50.0
```

---

## 七、关键经验总结

### ✅ 成功经验
1. **不要用 HAL_GetTick() 做延时**：STM32U5 系列可能存在时钟问题，改用循环计数器最可靠
2. **数据类型必须严格匹配**：`50` ≠ `50.0`，OneNet 对 float/int 区分严格
3. **物模型范围是硬门槛**：超范围整包拒绝，不会跳过单个字段
4. **消息 ID 必须递增**：固定 ID 会被去重机制丢弃
5. **善用 OneNet 日志**：设备调试 → 日志，能看到具体报错（类型错误/范围错误/字段缺失）

### ⚠️ 常见陷阱
- `sprintf` 格式化符与变量类型不匹配（`%d` vs `%.1f`）
- 物模型修改后未点击**发布**
- 设备在线但数据不更新（通常是物模型校验失败）
- JSON 字符串拼接时缺少转义（`"` 必须写成 `\"`）

### 📋 检查清单
烧录前确认：
- [ ] 所有变量类型与物模型一致（float/int/bool）
- [ ] `snprintf` 格式化符与变量类型匹配
- [ ] JSON 中 `params` 后有 `{`，结尾有 `}}`
- [ ] 消息 ID 是递增变量，不是固定字符串
- [ ] OneNet 物模型已点击**发布**

---

## 八、相关文件

| 文件 | 关键内容 |
|------|---------|
| `Core/Src/bsp_esp8266.c` | OneNet 连接、数据上报、JSON 构建 |
| `Core/Inc/bsp_esp8266.h` | 外部变量声明（注意 float/int 类型）|
| `Core/Src/main.c` | `OneNet_Report_Task()` 循环调用 |

---

**文档结束**
