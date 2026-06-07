/**
  ******************************************************************************
  * @file   bsp_esp8266.c
  * @brief  wifi模组ESP-12F的驱动程序
  * 
  ******************************************************************************
  */
#include "bsp_esp8266.h"
#include <stdarg.h>
#include "usart.h"
//
struct STRUCT_USART_Fram ESP8266_Fram_Record_Struct = { 0 };  //定义了一个数据帧结构体
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
    else if( ack1 != 0 )  //strstr(s1,s2);检测s2是否为s1的一部分，是返回该位置，否则返回false，它强制转换为bool类型了
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
    return ESP8266_Send_AT_Cmd( cCmd, "OK", NULL, 5000 );
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
static char *itoa(int value, char *string, int radix )    //把一整数转换为字符串。
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
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

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
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

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */
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
    while ( * Data != 0 )     // 判断数据是否到达结束符
    {                                         
        if ( * Data == 0x5c )  //'\'
        {                                     
            switch ( *++Data )
            {
                case 'r':                                     //回车符
								singleBuff[0] = 0x0d;    
								__HAL_UART_CLEAR_FLAG(USARTx, UART_CLEAR_TCF);/* Clear the TC flag in the ICR register */
								HAL_UART_Transmit(USARTx, (void*)singleBuff, 1, 1);//阻塞式发送数据
                Data ++;
                break;

                case 'n':                                     //换行符
								singleBuff[0] = 0x0a;    
								__HAL_UART_CLEAR_FLAG(USARTx, UART_CLEAR_TCF);/* Clear the TC flag in the ICR register */
								HAL_UART_Transmit(USARTx, (void*)singleBuff, 1, 1);//阻塞式发送数据
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
                case 's':                                         //字符串
                s = va_arg(ap, const char *);
                for ( ; *s; s++) 
                {
									singleBuff[0] = *s;    
									HAL_UART_Transmit(USARTx, (void*)singleBuff, 1, 1);//阻塞式发送数据
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
									HAL_UART_Transmit(USARTx, (void*)singleBuff, 1, 1);//阻塞式发送数据
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
					__HAL_UART_CLEAR_FLAG(USARTx, UART_CLEAR_TCF);/* Clear the TC flag in the ICR register */
					HAL_UART_Transmit(USARTx, (void*)singleBuff, 1, 1);//阻塞式发送数据
				}		
        while (__HAL_UART_GET_FLAG(USARTx, UART_FLAG_TXE) == RESET);
    }
}


volatile uint8_t TcpClosedFlag = 0;

// 定义全局传感器数据变量
int car_temp = 250;      // 车内温度，真实值25.0℃，×10放大，int32
int car_humidity = 50;   // 车内湿度，真实值50%RH，直接上传
bool fatigue = false;    // 疲劳检测，布尔值
int heart_rate = 75;     // 心率，真实值75bpm，直接上传
int alcohol = 5;         // 酒精浓度，真实值0.05mg/L，×100放大
int co2 = 800;           // CO2浓度，真实值800ppm，直接上传
int smoke = 5;          // 烟雾浓度，真实值0.5%LEL，×10放大
int drive_time = 0;      // 驾驶时长，单位分钟，直接上传


/*
* OneNet MQTT 配置用户属性
*LinkID 连接ID,目前只支持0
*scheme 连接方式，这里选择MQTT over TCP,这里设置为1
*client_id MQTTclientID 用于标志client身份
*username 用于登录 MQTT 服务器 的 username
*password 用于登录 MQTT 服务器 的 password
*cert_key_ID 证书 ID, 目前支持一套 cert 证书, 参数为 0
*CA_ID 目前支持一套 CA 证书, 参数为 0
*path 资源路径，这里设置为""
*设置成功返回true 反之false
*/
/*
* OneNet MQTT 配置用户属性
* Client_Id: 设备名称
* User_Name: 产品 ID
* Password: Token
*/
bool OneNet_MQTT_Config(char * Client_Id, char * User_Name, char * Password)
{
    char cCmd [512]; 
    sprintf ( cCmd, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"", Client_Id, User_Name, Password );
    return ESP8266_Send_AT_Cmd( cCmd, "OK", NULL, 1000 );
}

/*
* 连接 OneNet MQTT 服务器
*/
bool OneNet_MQTT_Connect(char * Server_Ip, int Port)
{
    char cCmd [128];
    sprintf ( cCmd,"AT+MQTTCONN=0,\"%s\",%d,1", Server_Ip, Port);
    
    for(int i = 0; i < 5; i++)
    {
        ESP8266_Send_AT_Cmd("AT", "OK", NULL, 500);
        
        if(ESP8266_Send_AT_Cmd(cCmd, "OK", "ALREADY CONNECTED", 5000))
        {
            return true;
        }
        
        if(strstr(ESP8266_Fram_Record_Struct.Data_RX_BUF, "busy p..."))
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

/*
* 订阅 Topic
*/
bool OneNet_MQTT_Subscribe(char * Topic)
{
    char cCmd [128];
    sprintf ( cCmd, "AT+MQTTSUB=0,\"%s\",1", Topic );
    return ESP8266_Send_AT_Cmd( cCmd, "OK", NULL, 500 );
}

/*
* 发布字符串数据
*/
bool OneNet_MQTT_Publish(char * Topic, char * Data)
{
    char cCmd [256];
    sprintf (cCmd, "AT+MQTTPUB=0,\"%s\",\"%s\",1,0", Topic, Data);
    return ESP8266_Send_AT_Cmd( cCmd, "OK", NULL, 1000 );
}

/*
* 发布物模型属性数据 (RAW 模式)
*/
bool OneNet_Publish_Property(char * Topic, char * JSON_Payload, uint16_t Len)
{
    char cCmd [256];
    
    HAL_Delay(800);
    sprintf (cCmd, "AT+MQTTPUBRAW=0,\"%s\",%d,0,0", Topic, Len);
    
    if(ESP8266_Send_AT_Cmd(cCmd, ">", NULL, 2000))
    {
        ESP8266_USART("%s", JSON_Payload);
        HAL_Delay(800);
        return true;
    }
    return false;
}

/*
* 清除 MQTT 连接状态
*/
bool OneNet_MQTT_Clear(void)
{
    return ESP8266_Send_AT_Cmd("AT+MQTTCLEAN=0", "OK", NULL, 500);
}

/*
* OneNet MQTT 数据上报主流程
*/


void simulate_sensor_data(void)
{
    // 温度模拟：在20.0~40.0℃之间变化（对应int值200~400）
    car_temp += 2; 
    if(car_temp > 40) car_temp = 20;

    // 湿度模拟：在30~80%RH之间变化
    car_humidity--; 
    if(car_humidity < 30) car_humidity = 80;

    // 心率模拟：在60~150bpm之间变化
    heart_rate += 2; 
    if(heart_rate > 150) heart_rate = 60;

    // 疲劳状态翻转
    fatigue = !fatigue;

    // 酒精浓度模拟：在0~0.2mg/L之间变化（对应int值0~20）
    alcohol += 1;
    if(alcohol > 20) alcohol = 0;

    // CO2浓度模拟：在400~1500ppm之间变化
    co2 += 50;
    if(co2 > 1500) co2 = 400;

    // 烟雾浓度模拟：在0~5%LEL之间变化（对应int值0~50）
    smoke += 1;
    if(smoke > 50) smoke = 0;

    // 驾驶时长模拟：每分钟+1，到1440分钟（24小时）重置
    drive_time++;
    if(drive_time > 1440) drive_time = 0;
}

// 3. 构建OneNet物模型JSON
void build_onenet_payload(char *payload, size_t payload_size)
{
    sprintf(payload, 
        "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{"
        "\"car_humidity\":{\"value\":%d},"
        "\"car_temp\":{\"value\":%d},"
        "\"fatigue\":{\"value\":%s},"
        "\"heart_rate\":{\"value\":%d},"
        "\"alcohol\":{\"value\":%d},"
        "\"co2\":{\"value\":%d},"
        "\"smoke\":{\"value\":%d},"
        "\"drive_time\":{\"value\":%d}"
        "}}",
        car_humidity,
        car_temp,
        fatigue ? "true" : "false",
        heart_rate,
        alcohol,
        co2,
        smoke,
        drive_time
    );
}

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
        // 模拟传感器数据变化
        simulate_sensor_data();

        // 构建 OneNet 物模型 JSON
        build_onenet_payload(payload, sizeof(payload));
        
        printf("Report Payload: %s\r\n", payload);

        HAL_Delay(1000);  
        OneNet_Publish_Property(ONENET_TOPIC_PROP_POST, payload, strlen(payload));
        
        HAL_Delay(5000); 
    }
}
