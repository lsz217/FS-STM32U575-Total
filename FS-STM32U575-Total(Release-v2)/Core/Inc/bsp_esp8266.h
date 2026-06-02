/**
  ******************************************************************************
  * @file   bsp_esp8266.h
  * @brief  wifi模组ESP-12F的驱动头文件
  * 
  ******************************************************************************
  */
#ifndef __BSP_ESP8266_H__
#define __BSP_ESP8266_H__
//
#include "main.h"
//
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
//
#if defined (__CC_ARM)
#pragma anon_unions
#endif
//TCP测试
#define User_ESP8266_SSID     "Xiaomi_D5D9"          //wifi名
#define User_ESP8266_PWD      "huat@iae"      	 					//wifi密码

#define User_ESP8266_TCPServer_IP     "192.168.31.79"   //服务器IP
#define User_ESP8266_TCPServer_PORT   "8080"      				//服务器端口号
//
// OneNet MQTT 鉴权参数
#define ONENET_CLIENT_ID    "d1"   										// 设备名称
#define ONENET_PRODUCT_ID   "NRL824h77z"									// 产品 ID (对应用户名)
#define ONENET_TOKEN        "version=2018-10-31&res=products%2FNRL824h77z%2Fdevices%2Fd1&et=1901881814&method=md5&sign=Ox25KvXCGh2d1WHaYWJHmA%3D%3D"

// OneNet MQTT 服务器配置
#define ONENET_SERVER_IP    "183.230.40.96"	    // OneNet Studio 官方 IP (对应 mqtts.heclouds.com)
#define ONENET_SERVER_PORT  1883     						// 端口号

// OneNet 物模型上报 Topic
#define ONENET_TOPIC_PROP_POST  "$sys/NRL824h77z/d1/thing/property/post"

// 全职传感器数据变量 (跨文件使用)
extern float car_temp;       // 车内温度，float类型
extern float car_humidity;   // 车内湿度，改为float类型匹配物模型
extern int heart_rate;       // 心率
extern int spo2;             // 血氧饱和度
extern bool fatigue;         // 疲劳状态
extern int alcohol;          // 酒精浓度
extern int co2;              // CO2浓度
extern int smoke;            // 烟雾浓度
extern int drive_time;       // 驾驶时长

//ESP8266模式选择
typedef enum{
		STA,
		AP,
		STA_AP  
}ENUM_Net_ModeTypeDef;
//网络传输层协议，枚举类型
typedef enum{
		enumTCP,
		enumUDP,
} ENUM_NetPro_TypeDef;
//连接号，指定为该连接号可以防止其他计算机访问同一端口而发生错误
typedef enum{
		Multiple_ID_0 = 0,
		Multiple_ID_1 = 1,
		Multiple_ID_2 = 2,
		Multiple_ID_3 = 3,
		Multiple_ID_4 = 4,
		Single_ID_0 = 5,
} ENUM_ID_NO_TypeDef;
//
#define ESP8266_USART(fmt, ...)  USART_printf(&huart5, fmt, ##__VA_ARGS__)    
//
#define RX_BUF_MAX_LEN 1024       //最大字节数
//
extern struct STRUCT_USART_Fram   //数据帧结构体
{
		char Data_RX_BUF[RX_BUF_MAX_LEN];
		union 
		{
			volatile uint16_t InfAll;
			struct 
			{
				volatile uint16_t FramLength       :15;	// 14:0 
				volatile uint16_t FramFinishFlag   :1;	// 15 
			}InfBit;
		}; 	
}ESP8266_Fram_Record_Struct;
//初始化和TCP功能函数
void ESP8266_Init(UART_HandleTypeDef *huart, uint8_t *DataBuf,uint32_t bound);
void ESP8266_AT_Test(void);
bool ESP8266_Send_AT_Cmd(char *cmd,char *ack1,char *ack2,uint32_t time);
bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode);
bool ESP8266_JoinAP(char * pSSID, char * pPassWord);
bool ESP8266_Enable_MultipleId (FunctionalState enumEnUnvarnishTx);
bool ESP8266_Link_Server(ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id);
bool ESP8266_SendString(FunctionalState enumEnUnvarnishTx, char * pStr, uint32_t ulStrLength, ENUM_ID_NO_TypeDef ucId);
bool ESP8266_UnvarnishSend(void);
void ESP8266_ExitUnvarnishSend(void);
uint8_t ESP8266_Get_LinkStatus(void);
void USART_printf(UART_HandleTypeDef * USARTx, char * Data, ...);
// OneNet核心功能函数
bool OneNet_MQTT_Config(char * Client_Id, char * User_Name, char * Password);
bool OneNet_MQTT_Connect(char * Server_Ip, int Port);
bool OneNet_MQTT_Subscribe(char * Topic);
bool OneNet_MQTT_Publish(char * Topic, char * Data);
bool OneNet_Publish_Property(char * Topic, char * JSON_Payload, uint16_t Len);
bool OneNet_MQTT_Clear(void);

// 阻塞模式（测试用）
void OneNet_Report_Process(void);

// 非阻塞 AT 命令（不阻塞主循环）
void ESP8266_NB_AT_Start(const char *cmd, const char *ack1, const char *ack2, uint32_t timeout_ms);
int  ESP8266_NB_AT_Poll(void);       // 0=等待中, 1=成功, -1=失败

// 非阻塞模式（正式用）
void OneNet_Init_Start(void);        // 初始化并启动连接（非阻塞）
void OneNet_Report_Task(void);       // 在主循环中周期性调用
void simulate_sensor_data(void);     // 读取传感器数据
void build_onenet_payload(char *payload, size_t payload_size);

// 状态标志（供外部查询）
extern uint8_t gOneNet_Connected;    // 0=未连接, 1=已连接
extern uint32_t gOneNet_LastReportTime;
#endif
