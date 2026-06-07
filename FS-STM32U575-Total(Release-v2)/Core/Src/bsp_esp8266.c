/**
  ******************************************************************************
  * @file   bsp_esp8266.c
  * @brief  wifi模组ESP-12F的驱动程序
  *
  ******************************************************************************
  */
#include "bsp_esp8266.h"
#include <stdarg.h>
#include <stdlib.h>
#include "usart.h"
#include "bsp_sht20.h"
#include "bsp_scd41.h"
#include "bsp_mq2.h"
#include "bsp_mq3.h"
//
struct STRUCT_USART_Fram ESP8266_Fram_Record_Struct = { 0 };  //定义了一个数据帧结构体

// 添加清缓冲区函数
void ESP8266_Clear_Buffer(void)
{
    ESP8266_Fram_Record_Struct.InfBit.FramLength = 0;
    memset(ESP8266_Fram_Record_Struct.Data_RX_BUF, 0, RX_BUF_MAX_LEN);
}

//初始化波特率
void ESP8266_Init(UART_HandleTypeDef *huart, uint8_t *DataBuf,uint32_t bound)
{
	//设置波特率
  huart->Init.BaudRate = bound;
	//初始化配置
  if (HAL_UART_Init(huart) != HAL_OK)
  {
    Error_Handler();
  }
	//开启串口接收与空闲中断
	HAL_UART_Receive_IT(huart,(uint8_t *)DataBuf, 1);	//开启接收中断
	__HAL_UART_CLEAR_IDLEFLAG(huart);			//清除空闲中断标志
	__HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);	//开启空闲中断
}
//对ESP8266模块发送AT指令
// cmd 待发送的指令
// ack1,ack2;期待的响应，为NULL表不需响应，两者为或逻辑关系
// time 等待响应时间
//返回1发送成功， 0失败
bool ESP8266_Send_AT_Cmd(char *cmd,char *ack1,char *ack2,uint32_t time)
{
    ESP8266_Fram_Record_Struct .InfBit .FramLength = 0;		//重新接收新的数据包
    ESP8266_USART("%s\r\n", cmd);
    if(ack1==0&&ack2==0)		//不需要接收数据
    {
    return true;
    }
    HAL_Delay(time);	//延时
    ESP8266_Fram_Record_Struct.Data_RX_BUF[ESP8266_Fram_Record_Struct.InfBit.FramLength ] = '\0';
		//
    printf("%s",ESP8266_Fram_Record_Struct .Data_RX_BUF);
		//
    if((ack1!=0) && (ack2!=0))
    {
        return ( ( bool ) strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, ack1 ) ||
                         ( bool ) strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, ack2 ) );
    }
    else if( ack1 != 0 )
        return ( ( bool ) strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, ack1 ) );
    else
        return ( ( bool ) strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, ack2 ) );
}
//发送恢复出厂默认设置指令将模块恢复成出厂设置
void ESP8266_AT_Test(void)
{
    char count=0;
    HAL_Delay(100);
    while(count < 10)
    {
        if(ESP8266_Send_AT_Cmd("AT+RESTORE","OK",NULL,1000))
        {
            printf("OK\r\n");
            return;
        }
        ++ count;
    }
}
//选择ESP8266的工作模式
// enumMode 模式类型
//成功返回true，失败返回false
bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode)
{
    switch ( enumMode )
    {
        case STA:
            return ESP8266_Send_AT_Cmd ( "AT+CWMODE=1", "OK", "no change", 2500 );

        case AP:
            return ESP8266_Send_AT_Cmd ( "AT+CWMODE=2", "OK", "no change", 2500 );

        case STA_AP:
            return ESP8266_Send_AT_Cmd ( "AT+CWMODE=3", "OK", "no change", 2500 );

        default:
          return false;
    }
}
//ESP8266连接外部的WIFI
//pSSID WiFi帐号
//pPassWord WiFi密码
//设置成功返回true 反之false
bool ESP8266_JoinAP(char * pSSID, char * pPassWord)
{
    char cCmd [120];
		//
    sprintf ( cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );
    return ESP8266_Send_AT_Cmd( cCmd, "OK", NULL, 10000 );
}
//ESP8266 透传使能
//enumEnUnvarnishTx  是否多连接，bool类型
//设置成功返回true，反之false
bool ESP8266_Enable_MultipleId(FunctionalState enumEnUnvarnishTx )
{
    char cStr [20];

    sprintf ( cStr, "AT+CIPMUX=%d", ( enumEnUnvarnishTx ? 1 : 0 ) );

    return ESP8266_Send_AT_Cmd ( cStr, "OK", 0, 500 );
}
//ESP8266 连接服务器
//enumE  网络类型
//ip ，服务器IP
//ComNum  服务器端口
//id，连接号，确保通信不受外界干扰
//设置成功返回true，反之fasle
bool ESP8266_Link_Server(ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
{
    char cStr [100] = { 0 }, cCmd [120];

    switch (  enumE )
    {
        case enumTCP:
          sprintf ( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
          break;

        case enumUDP:
          sprintf ( cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum );
          break;

        default:
            break;
    }

    if ( id < 5 )
        sprintf ( cCmd, "AT+CIPSTART=%d,%s", id, cStr);

    else
        sprintf ( cCmd, "AT+CIPSTART=%s", cStr );

    return ESP8266_Send_AT_Cmd ( cCmd, "OK", "ALREAY CONNECT", 4000 );
}
//透传使能
//设置成功返回true， 反之false
bool ESP8266_UnvarnishSend (void)
{
    if (!ESP8266_Send_AT_Cmd ( "AT+CIPMODE=1", "OK", 0, 500 ))
        return false;
    return
        ESP8266_Send_AT_Cmd( "AT+CIPSEND", "OK", ">", 500 );
}
//ESP8266发送字符串
//enumEnUnvarnishTx是否使能透传模式
//pStr字符串
//ulStrLength字符串长度
//ucId 连接号
//设置成功返回true， 反之false
bool ESP8266_SendString(FunctionalState enumEnUnvarnishTx, char * pStr, uint32_t ulStrLength, ENUM_ID_NO_TypeDef ucId )
{
    char cStr [20];
    bool bRet = false;
		//
    if ( enumEnUnvarnishTx )
    {
        ESP8266_USART("%s", pStr );

        bRet = true;

    }
    else
    {
        if ( ucId < 5 )
            sprintf (cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2 );

        else
            sprintf (cStr, "AT+CIPSEND=%d", ulStrLength + 2 );

        ESP8266_Send_AT_Cmd (cStr, "> ", 0, 1000 );

        bRet = ESP8266_Send_AT_Cmd (pStr, "SEND OK", 0, 1000 );
  }
    return bRet;
}
//ESP8266退出透传模式
void ESP8266_ExitUnvarnishSend (void)
{
    HAL_Delay(1000);
    ESP8266_USART("+++");
    HAL_Delay( 500 );
}
//ESP8266 检测连接状态
//返回0：获取状态失败
//返回2：获得ip
//返回3：建立连接
//返回4：失去连接
uint8_t ESP8266_Get_LinkStatus (void)
{
    if (ESP8266_Send_AT_Cmd( "AT+CIPSTATUS", "OK", 0, 500 ) )
    {
        if ( strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, "STATUS:2\r\n" ) )
            return 2;

        else if ( strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, "STATUS:3\r\n" ) )
            return 3;

        else if ( strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, "STATUS:4\r\n" ) )
            return 4;

    }

    return 0;
}
//
static char *itoa(int value, char *string, int radix )
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    if (value < 0)
    {
        *ptr++ = '-';
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    *ptr = 0;

    return string;

}
//
void USART_printf(UART_HandleTypeDef * USARTx, char * Data, ... )
{
    const char *s;
    int d;
    char buf[16];
	  char singleBuff[1];
		//
    va_list ap;
    va_start(ap, Data);
		//
    while ( * Data != 0 )
    {
        if ( * Data == 0x5c )
        {
            switch ( *++Data )
            {
                case 'r':
								singleBuff[0] = 0x0d;
								__HAL_UART_CLEAR_FLAG(USARTx, UART_CLEAR_TCF);
								HAL_UART_Transmit(USARTx, (void*)singleBuff, 1, 1);
                Data ++;
                break;

                case 'n':
								singleBuff[0] = 0x0a;
								__HAL_UART_CLEAR_FLAG(USARTx, UART_CLEAR_TCF);
								HAL_UART_Transmit(USARTx, (void*)singleBuff, 1, 1);
                Data ++;
                break;

                default:
                Data ++;
                break;
            }
        }
        else if ( * Data == '%')
        {
            switch ( *++Data )
            {
                case 's':
                s = va_arg(ap, const char *);
                for ( ; *s; s++)
                {
								singleBuff[0] = *s;
								HAL_UART_Transmit(USARTx, (void*)singleBuff, 1, 1);
                  while(__HAL_UART_GET_FLAG(USARTx, UART_FLAG_TXE) == RESET);
                }
                Data++;
                break;

                case 'd':
                d = va_arg(ap, int);
                itoa(d, buf, 10);
                for (s = buf; *s; s++)
                {
								singleBuff[0] = *s;
								HAL_UART_Transmit(USARTx, (void*)singleBuff, 1, 1);
                  while(__HAL_UART_GET_FLAG(USARTx, UART_FLAG_TXE) == RESET);
                }
                     Data++;
                     break;
                default:
                     Data++;
                     break;
            }
        }
        else
				{
					singleBuff[0] = *Data++;
					__HAL_UART_CLEAR_FLAG(USARTx, UART_CLEAR_TCF);
					HAL_UART_Transmit(USARTx, (void*)singleBuff, 1, 1);
				}
        while (__HAL_UART_GET_FLAG(USARTx, UART_FLAG_TXE) == RESET);
    }
}


volatile uint8_t TcpClosedFlag = 0;

// ==================== 非阻塞 AT 命令 ====================
// 解决 ESP8266_Send_AT_Cmd 内部 HAL_Delay 阻塞主循环导致触摸卡死的问题
static uint8_t  nb_at_state = 0;       // 0=idle, 1=waiting
static uint32_t nb_at_deadline = 0;
static char     nb_at_ack1[32];
static char     nb_at_ack2[32];

void ESP8266_NB_AT_Start(const char *cmd, const char *ack1, const char *ack2, uint32_t timeout_ms)
{
    ESP8266_Fram_Record_Struct.InfBit.FramLength = 0;
    memset(ESP8266_Fram_Record_Struct.Data_RX_BUF, 0, RX_BUF_MAX_LEN);
    ESP8266_USART("%s\r\n", cmd);

    if (ack1) { strncpy(nb_at_ack1, ack1, sizeof(nb_at_ack1) - 1); nb_at_ack1[sizeof(nb_at_ack1)-1] = 0; }
    else nb_at_ack1[0] = 0;
    if (ack2) { strncpy(nb_at_ack2, ack2, sizeof(nb_at_ack2) - 1); nb_at_ack2[sizeof(nb_at_ack2)-1] = 0; }
    else nb_at_ack2[0] = 0;

    nb_at_deadline = HAL_GetTick() + timeout_ms;
    nb_at_state = 1;
}

// 返回: 0=等待中, 1=成功, -1=失败/超时
int ESP8266_NB_AT_Poll(void)
{
    if (nb_at_state == 0) return 1;

    // 检查响应缓冲区（ESP8266 响应以 \r\n 结尾，由空闲中断标记帧完成）
    ESP8266_Fram_Record_Struct.Data_RX_BUF[ESP8266_Fram_Record_Struct.InfBit.FramLength] = '\0';

    bool ack1_needed = (nb_at_ack1[0] != 0);
    bool ack2_needed = (nb_at_ack2[0] != 0);

    // 两个 ack 都不需要 → 不等响应，直接成功
    if (!ack1_needed && !ack2_needed) {
        nb_at_state = 0;
        return 1;
    }

    // 检查需要的 ack：任一匹配即成功（OR 逻辑，与阻塞版 ESP8266_Send_AT_Cmd 一致）
    bool matched = false;
    if (ack1_needed && strstr((const char*)ESP8266_Fram_Record_Struct.Data_RX_BUF, nb_at_ack1) != NULL)
        matched = true;
    if (ack2_needed && strstr((const char*)ESP8266_Fram_Record_Struct.Data_RX_BUF, nb_at_ack2) != NULL)
        matched = true;

    if (matched) {
        nb_at_state = 0;
        return 1;
    }

    // 超时？
    if (HAL_GetTick() >= nb_at_deadline) {
        printf("[NB_AT] timeout: %s\r\n", (const char*)ESP8266_Fram_Record_Struct.Data_RX_BUF);
        nb_at_state = 0;
        return -1;
    }

    return 0; // 还在等
}

// ==================== OneNet MQTT 功能 ====================

// 外部传感器数据变量声明
extern volatile SHT20_TemRH_Val gTemRH_Val;
extern int32_t n_heart_rate;
extern int32_t n_sp02;           // MAX30102 血氧值
extern volatile SCD41_Data_t gSCD41_Val;

// 传感器数据变量
float car_temp = 25.0f;      // 车内温度，float类型，实际值℃
float car_humidity = 50.0f;  // 车内湿度，改为float类型匹配物模型
int heart_rate = 0;          // 初始化0，测量后才更新
int spo2 = 0;                // 初始化0，测量后才更新
bool fatigue = false;
int alcohol = 5;
int co2 = 800;
int smoke = 5;
int drive_time = 0;

// 心率和血氧测量时间戳（用于10秒超时归零）
static uint32_t hr_last_valid_time = 0;
static uint32_t spo2_last_valid_time = 0;
#define HR_SPO2_TIMEOUT_MS  10000  // 10秒超时
// 死区过滤：仅当变化超过±5时才更新上传值，避免频繁上传稳定数据
	static int last_uploaded_hr = 0;
	static int last_uploaded_spo2 = 0;

// OneNet MQTT 配置
bool OneNet_MQTT_Config(char * Client_Id, char * User_Name, char * Password)
{
    char cCmd [512];
    sprintf ( cCmd, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"", Client_Id, User_Name, Password );
    return ESP8266_Send_AT_Cmd( cCmd, "OK", NULL, 2000 );
}

// 连接 OneNet MQTT 服务器
bool OneNet_MQTT_Connect(char * Server_Ip, int Port)
{
    char cCmd [128];
    sprintf ( cCmd,"AT+MQTTCONN=0,\"%s\",%d,1", Server_Ip, Port);

    for(int i = 0; i < 5; i++)
    {
        ESP8266_Send_AT_Cmd("AT", "OK", NULL, 500);

        if(ESP8266_Send_AT_Cmd(cCmd, "OK", "ALREADY CONNECTED", 8000))
        {
            return true;
        }

        if(strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, "busy"))
        {
            printf("Module busy, retrying... (%d/5)\r\n", i+1);
            HAL_Delay(3000);
        }
        else if(strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, "ERROR"))
        {
            printf("Connection Error. Check Token or Network. Retrying... (%d/5)\r\n", i+1);
            HAL_Delay(2000);
        }
    }
    return false;
}

// 订阅 Topic
bool OneNet_MQTT_Subscribe(char * Topic)
{
    char cCmd [128];
    sprintf ( cCmd, "AT+MQTTSUB=0,\"%s\",1", Topic );
    return ESP8266_Send_AT_Cmd( cCmd, "OK", NULL, 500 );
}

// 发布字符串数据
bool OneNet_MQTT_Publish(char * Topic, char * Data)
{
    char cCmd [256];
    sprintf (cCmd, "AT+MQTTPUB=0,\"%s\",\"%s\",1,0", Topic, Data);
    return ESP8266_Send_AT_Cmd( cCmd, "OK", NULL, 1000 );
}

// 发布物模型属性数据 (RAW 模式)
bool OneNet_Publish_Property(char * Topic, char * JSON_Payload, uint16_t Len)
{
    char cCmd [512];

    HAL_Delay(200);
    sprintf(cCmd, "AT+MQTTPUBRAW=0,\"%s\",%d,1,0", Topic, Len);

    printf("[MQTT] Stage1 cmd: %s\r\n", cCmd);

    // Step 1: 发送命令，等待 ">"
    ESP8266_Clear_Buffer();
    ESP8266_USART("%s\r\n", cCmd);
    HAL_Delay(1000);  // 等待响应

    if(!strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, ">"))
    {
        printf("[MQTT] FAIL: No '>' prompt! buf=%s\r\n", ESP8266_Fram_Record_Struct.Data_RX_BUF);
        return false;
    }

    printf("[MQTT] Got '>', sending payload...\r\n");

    // Step 2: 发送 payload 数据（不带\r\n）
    ESP8266_Clear_Buffer();
    ESP8266_USART("%s", JSON_Payload);
    HAL_Delay(1000);  // 等待 SEND OK

    printf("[MQTT] Stage2 response: %s\r\n", ESP8266_Fram_Record_Struct.Data_RX_BUF);

    // Step 3: 检查 SEND OK 或 OK
    if(strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, "SEND OK") ||
       strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, "MQTTPUB:OK"))
    {
        printf("[MQTT] Publish SUCCESS!\r\n");
        return true;
    }

    printf("[MQTT] Publish FAILED!\r\n");
    return false;
}

// 清除 MQTT 连接状态
bool OneNet_MQTT_Clear(void)
{
    return ESP8266_Send_AT_Cmd("AT+MQTTCLEAN=0", "OK", NULL, 500);
}

// 读取传感器数据
void simulate_sensor_data(void)
{
    uint32_t now = HAL_GetTick();  // 获取当前时间

    // 读取 SHT20
    BSP_SHT20_GetData();
    HAL_Delay(20);

    if(gTemRH_Val.Tem > 0.1f && gTemRH_Val.Tem < 100.0f)
    {
        car_temp = gTemRH_Val.Tem;        // 直接存储float温度值
        car_humidity = gTemRH_Val.Hum;  // 湿度改为float类型
    }

	// 读取心率（死区过滤：变化超过±5才更新上传值，超时时归零）
	if(n_heart_rate > 0 && n_heart_rate < 1200)  // 最大300BPM*4=1200
	{
		int new_hr = n_heart_rate / 4;
		hr_last_valid_time = now;  // 每次有效读数都刷新时间戳
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
			last_uploaded_hr = 0;
		}
	}

	// 读取血氧（死区过滤：变化超过±5才更新上传值，超时时归零）
	if(n_sp02 > 70 && n_sp02 <= 100)  // 正常血氧70-100%
	{
		spo2_last_valid_time = now;  // 每次有效读数都刷新时间戳
		if(abs(n_sp02 - last_uploaded_spo2) > 5)
		{
			spo2 = n_sp02;
			last_uploaded_spo2 = n_sp02;
		}
	}
	else if((now - spo2_last_valid_time) > HR_SPO2_TIMEOUT_MS)
	{
		if(spo2 != 0)
		{
			spo2 = 0;
			last_uploaded_spo2 = 0;
		}
	}

    // MQ-3 酒精
    if(BSP_MQ3_Read() == 0)
    {
        alcohol = 15;
        fatigue = true;
    }
    else
    {
        alcohol = 0;
    }

    // MQ-2 烟雾
    if(BSP_MQ2_Read() == 0)
    {
        smoke = 50;
    }
    else
    {
        smoke = 5;
    }

    // SCD41 CO2
    co2 = (int)gSCD41_Val.CO2;
    if(co2 < 400) co2 = 400;

    // 驾驶时长
    drive_time++;
    if(drive_time > 1440) drive_time = 0;
}

// 构建OneNet物模型JSON（id每次变化，避免被当重复消息）
void build_onenet_payload(char *payload, size_t payload_size)
{
    static uint16_t msg_id = 1;

    // 简化测试 - 只发几个基本字段，按物模型定义的顺序
    snprintf(payload, payload_size,
        "{\"id\":\"%d\",\"version\":\"1.0\",\"params\":{"
        "\"car_temp\":{\"value\":%.1f},"
        "\"car_humidity\":{\"value\":%.1f},"  // 改为%%.1f，匹配float类型
        "\"heart_rate\":{\"value\":%d},"
        "\"spo2\":{\"value\":%d}"
        "}}",
        msg_id++,
        car_temp,
        car_humidity,  // 现在是float类型
        heart_rate,
        spo2
    );

    // 调试用 - 打印JSON长度和内容
    printf("[JSON] len=%d: %s\r\n", strlen(payload), payload);
}

// ==================== 非阻塞版本 ====================

uint8_t gOneNet_Connected = 0;
uint32_t gOneNet_LastReportTime = 0;
static uint8_t gOneNet_InitStage = 0;
static uint32_t gOneNet_NextActionTime = 0;

// ==================== 非阻塞 OneNet 状态机 ====================

// OneNet 内部状态（偶数=发送命令, 奇数=等待响应）
typedef enum {
    OS_SEND_AT = 0,
    OS_WAIT_AT,
    OS_SEND_MODE,
    OS_WAIT_MODE,
    OS_SEND_WIFI,
    OS_WAIT_WIFI,
    OS_WAIT_IP,
    OS_SEND_CLEAN,
    OS_WAIT_CLEAN,
    OS_SEND_CFG,
    OS_WAIT_CFG,
    OS_SEND_CONN,
    OS_WAIT_CONN,
    OS_CONNECTED,
    // publish sub-states
    OS_PUB_SEND_CMD,
    OS_PUB_WAIT_PROMPT,
    OS_PUB_SEND_DATA,
    OS_PUB_WAIT_OK,
} OneNet_NB_State;

static OneNet_NB_State os = OS_SEND_AT;
static uint32_t os_last_report = 0;
static uint8_t  os_retry = 0;
static char     os_payload[1024];
static uint16_t os_payload_len = 0;
static char     os_topic[128];

void OneNet_Init_Start(void)
{
    printf("[OneNet] Starting non-blocking init...\r\n");
    gOneNet_InitStage = 0;
    gOneNet_Connected = 0;
    gOneNet_LastReportTime = 0;
    gOneNet_NextActionTime = HAL_GetTick() + 100;
    os = OS_SEND_AT;       // 复位非阻塞状态机
    os_retry = 0;
    nb_at_state = 0;       // 复位 AT 命令状态
}

void OneNet_Report_Task(void)
{
    int r;

    switch (os) {
    // ===== INIT: AT test =====
    case OS_SEND_AT:
        printf("[ON] S0: AT>\r\n");
        ESP8266_NB_AT_Start("AT", "OK", NULL, 200);
        os = OS_WAIT_AT;
        return;
    case OS_WAIT_AT:
        r = ESP8266_NB_AT_Poll();
        if (r == 0) return;
        os = (r == 1) ? OS_SEND_MODE : OS_SEND_AT;
        return;

    // ===== INIT: CWMODE =====
    case OS_SEND_MODE:
        printf("[ON] S1: MODE>\r\n");
        ESP8266_NB_AT_Start("AT+CWMODE=1", "OK", "no change", 500);
        os = OS_WAIT_MODE;
        return;
    case OS_WAIT_MODE:
        r = ESP8266_NB_AT_Poll();
        if (r == 0) return;
        os = (r == 1) ? OS_SEND_WIFI : OS_SEND_MODE;
        return;

    // ===== INIT: Join WiFi =====
    case OS_SEND_WIFI: {
        char cCmd[120];
        sprintf(cCmd, "AT+CWJAP=\"%s\",\"%s\"", User_ESP8266_SSID, User_ESP8266_PWD);
        printf("[ON] S2: WiFi> [%s]\r\n", User_ESP8266_SSID);
        ESP8266_NB_AT_Start(cCmd, "OK", NULL, 10000);
        os = OS_WAIT_WIFI;
        return;
    }
    case OS_WAIT_WIFI:
        r = ESP8266_NB_AT_Poll();
        if (r == 0) return;
        if (r == 1) {
            printf("[ON] WiFi OK, waiting for IP...\r\n");
            os = OS_WAIT_IP;
            ESP8266_Fram_Record_Struct.InfBit.FramLength = 0;
            memset(ESP8266_Fram_Record_Struct.Data_RX_BUF, 0, RX_BUF_MAX_LEN);
            nb_at_deadline = HAL_GetTick() + 15000;
            strncpy(nb_at_ack1, "WIFI GOT IP", sizeof(nb_at_ack1) - 1);
            nb_at_ack2[0] = 0;
            nb_at_state = 1;
        } else {
            printf("[ON] WiFi FAIL\r\n");
            os = OS_SEND_WIFI;
        }
        return;

    // ===== INIT: Wait for DHCP IP =====
    case OS_WAIT_IP:
        r = ESP8266_NB_AT_Poll();
        if (r == 0) return;
        if (r == 1) {
            printf("[ON] IP OK\r\n");
            os = OS_SEND_CLEAN;
            os_retry = 0;
        } else {
            printf("[ON] IP timeout, proceed anyway\r\n");
            os = OS_SEND_CLEAN;
        }
        return;

    // ===== INIT: MQTT CLEAN =====
    case OS_SEND_CLEAN:
        printf("[ON] S3: CLEAN>\r\n");
        ESP8266_NB_AT_Start("AT+MQTTCLEAN=0", "OK", NULL, 500);
        os = OS_WAIT_CLEAN;
        return;
    case OS_WAIT_CLEAN:
        r = ESP8266_NB_AT_Poll();
        if (r == 0) return;
        if (r == 1) {
            os = OS_SEND_CFG;
        } else {
            printf("[ON] MQTTCLEAN failed, skip\r\n");
            os = OS_SEND_CFG;
            os_retry = 0;
        }
        return;

    // ===== INIT: MQTT CONFIG =====
    case OS_SEND_CFG: {
        char cCmd[512];
        sprintf(cCmd, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"",
                ONENET_CLIENT_ID, ONENET_PRODUCT_ID, ONENET_TOKEN);
        printf("[ON] S4: CFG>\r\n");
        ESP8266_NB_AT_Start(cCmd, "OK", NULL, 2000);
        os = OS_WAIT_CFG;
        return;
    }
    case OS_WAIT_CFG:
        r = ESP8266_NB_AT_Poll();
        if (r == 0) return;
        os = (r == 1) ? OS_SEND_CONN : OS_SEND_CFG;
        return;

    // ===== INIT: MQTT CONNECT =====
    case OS_SEND_CONN: {
        char cCmd[128];
        sprintf(cCmd, "AT+MQTTCONN=0,\"%s\",%d,1", ONENET_SERVER_IP, ONENET_SERVER_PORT);
        printf("[ON] S5: CONN>\r\n");
        // 先发个 AT 确认模块正常
        ESP8266_NB_AT_Start("AT", "OK", NULL, 200);
        os = OS_WAIT_CONN;
        os_retry = 0; // 用 os_retry 记子状态: 0=等AT OK, 1=发CONN, 2=等CONN OK
        return;
    }
    case OS_WAIT_CONN: {
        r = ESP8266_NB_AT_Poll();
        if (r == 0) return;

        if (os_retry == 0) {
            // AT OK, 现在发 CONN
            char cCmd[128];
            sprintf(cCmd, "AT+MQTTCONN=0,\"%s\",%d,1", ONENET_SERVER_IP, ONENET_SERVER_PORT);
            ESP8266_NB_AT_Start(cCmd, "OK", "ALREADY CONNECTED", 8000);
            os_retry = 1;
            return;
        }

        // CONN 完成
        if (r == 1) {
            printf("[ON] === CONNECTED! ===\r\n");
            gOneNet_Connected = 1;
            gOneNet_InitStage = 100;
            os = OS_CONNECTED;
            os_last_report = 0; // 立即上报第一次
        } else {
            printf("[ON] Connect failed, retry...\r\n");
            os = OS_SEND_CLEAN; // 回退重试
        }
        return;
    }

    // ===== CONNECTED: 定时上报 =====
    case OS_CONNECTED:
        if (HAL_GetTick() - os_last_report < 10000)
            return; // 不到10秒，跳过

        // 准备上报
        simulate_sensor_data();
        build_onenet_payload(os_payload, sizeof(os_payload));
        os_payload_len = strlen(os_payload);
        strncpy(os_topic, ONENET_TOPIC_PROP_POST, sizeof(os_topic) - 1);
        printf("[ON] REP> %s\r\n", os_payload);
        os = OS_PUB_SEND_CMD;
        return;

    // ===== PUB: 发送 MQTTPUBRAW 命令，等 '>' =====
    case OS_PUB_SEND_CMD: {
        char cCmd[512];
        sprintf(cCmd, "AT+MQTTPUBRAW=0,\"%s\",%d,1,0", os_topic, os_payload_len);
        printf("[ON] PUB CMD: %s\r\n", cCmd);
        ESP8266_NB_AT_Start(cCmd, ">", NULL, 2000);
        os = OS_PUB_WAIT_PROMPT;
        return;
    }
    case OS_PUB_WAIT_PROMPT:
        r = ESP8266_NB_AT_Poll();
        if (r == 0) return;
        if (r == 1) {
            os = OS_PUB_SEND_DATA;
        } else {
            printf("[ON] PUB FAIL: no '>'\r\n");
            os = OS_CONNECTED;
            os_last_report = HAL_GetTick();
        }
        return;

    // ===== PUB: 发送 payload，等 OK（payload 不带 \r\n）=====
    case OS_PUB_SEND_DATA:
        printf("[ON] SEND payload...\r\n");
        // 清缓冲，发 payload（不加 \r\n，和原版 OneNet_Publish_Property 一致）
        ESP8266_Fram_Record_Struct.InfBit.FramLength = 0;
        memset(ESP8266_Fram_Record_Struct.Data_RX_BUF, 0, RX_BUF_MAX_LEN);
        ESP8266_USART("%s", os_payload);
        // 手动设 nb 等待状态，不调 NB_AT_Start（避免多发 \r\n）
        nb_at_deadline = HAL_GetTick() + 2000;
        strncpy(nb_at_ack1, "SEND OK", sizeof(nb_at_ack1) - 1);
        strncpy(nb_at_ack2, "MQTTPUB:OK", sizeof(nb_at_ack2) - 1);
        nb_at_ack1[sizeof(nb_at_ack1)-1] = 0;
        nb_at_ack2[sizeof(nb_at_ack2)-1] = 0;
        nb_at_state = 1;
        os = OS_PUB_WAIT_OK;
        return;
    case OS_PUB_WAIT_OK:
        r = ESP8266_NB_AT_Poll();
        if (r == 0) return;
        if (r == 1) {
            printf("[ON] === PUB OK! ===\r\n");
        } else {
            printf("[ON] PUB FAIL at data stage\r\n");
        }
        os = OS_CONNECTED;
        os_last_report = HAL_GetTick();
        return;
    }
}

// 原来的阻塞式函数（保留兼容）
void OneNet_Report_Process(void)
{
    char payload[512];

    printf("Step 1: System Restore...\r\n");
    ESP8266_AT_Test();
    HAL_Delay(3000);

    printf("Step 2: Connecting WiFi...\r\n");
    ESP8266_Net_Mode_Choose(STA);
    while(!ESP8266_JoinAP(User_ESP8266_SSID, User_ESP8266_PWD))
    {
        HAL_Delay(2000);
    }
    HAL_Delay(2000);

    printf("Step 3: Configuring OneNet MQTT...\r\n");
    OneNet_MQTT_Clear();
    HAL_Delay(500);
    OneNet_MQTT_Config(ONENET_CLIENT_ID, ONENET_PRODUCT_ID, ONENET_TOKEN);
    HAL_Delay(1000);

    printf("Step 4: Connecting to OneNet...\r\n");
    while(!OneNet_MQTT_Connect(ONENET_SERVER_IP, ONENET_SERVER_PORT))
    {
        printf("Retrying Connection...\r\n");
        HAL_Delay(3000);
        OneNet_MQTT_Clear();
        OneNet_MQTT_Config(ONENET_CLIENT_ID, ONENET_PRODUCT_ID, ONENET_TOKEN);
    }

    printf("Step 5: Starting Data Report Loop...\r\n");

    while(1)
    {
        simulate_sensor_data();
        build_onenet_payload(payload, sizeof(payload));

        printf("Report Payload: %s\r\n", payload);

        HAL_Delay(1000);
        OneNet_Publish_Property(ONENET_TOPIC_PROP_POST, payload, strlen(payload));

        HAL_Delay(5000);
    }
}
