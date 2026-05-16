/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "crc.h"
#include "gpdma.h"
#include "i2c.h"
#include "icache.h"
#include "octospi.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "app_touchgfx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "user_app.h"		//用户应用程序
#include "uart_receiver.h"  // <--- 插入这一行
#include "bsp_mq2.h"
#include "bsp_mq3.h"
#include "bsp_motor.h"
#include "bsp_max30102.h"
#include "bsp_scd41.h"
//
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define	ADC_CONVERTED_DATA_BUFFER_SIZE	6	
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t gChatCount = 10;  //全局按键按压计数
uint8_t KeyChangeScreen=0;
static uint8_t gTaskIndex = 0x00;  //系统任务索引变量
ADC_ValTypeDef gStruADC={0,0,0,0,0,0}; //A/D通道实时采集的数据
uint16_t ADC_KEY=0;
extern gTask_MarkEN gTaskEnMark;  //系统任务使能标识
extern volatile uint8_t gBacklightVal;	//背光值，默认50%
extern volatile uint8_t TcpClosedFlag;	
volatile uint8_t gRX3_BufF[1];	//串口3-wifi模组接收到的数据
extern volatile SHT20_TemRH_Val gTemRH_Val; // <--- 插入这一行

//--------------------------------------------------------------------------------------------------------------------------

/* USER CODE BEGIN PTD */
// --- NFC 模块变量 ---
NFC_Handler_t nfc;
uint8_t nfc_rx_pool[NFC_BUF_SIZE];

// NFC 内部业务状态（设密码 -> 读数据）
typedef enum {
    NFC_APP_IDLE,
    NFC_APP_WAIT_KEY,
    NFC_APP_WAIT_READ
} NFC_AppState_t;

NFC_AppState_t nfc_app_state = NFC_APP_IDLE;

// 给 UI 逻辑使用的最终结果标志
volatile uint8_t g_nfc_unlock_flag = 0; // 1 代表读卡成功并匹配
/* USER CODE END PTD */
//--------------------------------------------------------------------------------------------------------------------------------------------



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
/* USER CODE BEGIN PFP */
extern void touchgfx_signalVSynTimer(void);
extern gTask_BitDef gTaskStateBit;  //任务执行过程中使用到的标志位
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**************************关闭标准库下的半主机模式****************************/
__ASM (".global __use_no_semihosting");	//AC6编译器
//标准库需要的支持函数
struct FILE 
{
  int handle; 
};
FILE __stdout;
//定义_sys_exit()以避免使用半主机模式  
void _sys_exit(int x) 
{ 
  x = x; 
}
void _ttywrch(int ch)
{
  ch = ch;
}
//printf实现重定向
int fputc(int ch, FILE *f)	
{
	uint8_t temp[1] = {ch};
	HAL_UART_Transmit(&huart1, temp, 1, 2);
	return ch;
}
uint32_t flag=0;
/******************************************************************************/
//
/*******************************************************************************
*	函 数 名: OSPI_W25Qxx_mmap
*	入口参数: 无
*	返 回 值: 无
*	函数功能: 设置为内存映射模式
*	说    明: 无	
*******************************************************************************/
void OSPI_W25Qxx_mmap(void)		//Flash读写测试
{
	int32_t OSPI_Status ; 		 //检测标志位
	//
	OSPI_Status = OSPI_W25Qxx_MemoryMappedMode(); //配置OSPI为内存映射模式
	if( OSPI_Status == OSPI_W25Qxx_OK )
	{
		//printf ("\r\n内存映射模式设置成功>>>>\r\n");		
	}
	else
	{
		//printf ("\r\n内存映射模式设置失败>>>>\r\n");
		Error_Handler();
	}	
}
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the System Power */
  SystemPower_Config();

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_GPDMA1_Init();
  MX_OCTOSPI1_Init();
  MX_SPI1_Init();
  MX_CRC_Init();
  MX_I2C1_Init();
  MX_TIM16_Init();
  MX_TIM17_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_ADC1_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_UART5_Init();
  MX_ICACHE_Init();
  MX_UART4_Init();
  MX_TouchGFX_Init();
  /* USER CODE BEGIN 2 */
	for(gTaskIndex = 0;gTaskIndex < OS_TASKLISTCNT;gTaskIndex++)	g_OSTsakList[gTaskIndex]=NULL;   //清空任务列表
  //NOR Flash初始化
  OSPI_W25Qxx_Init();	//初始化W25Q128
  OSPI_W25Qxx_mmap();	//设置为内存映射模式
	
		//=== 传感器初始化 ===
		BSP_MQ2_Init();
		BSP_MQ3_Init();
		BSP_MOTOR_Init();
		//=== 初始化完成 ===
		// 测试调用（二选一，阻塞运行）：
		// BSP_MQ2_Test();  // MQ-2烟雾传感器测试
		// BSP_MQ3_Test();  // MQ-3酒精传感器测试


	// 插入 NFC 初始化
NFC_Init(&nfc, &huart4, &huart1, nfc_rx_pool, NFC_BUF_SIZE);
	
	void Force_PC13_To_Input();
		
	
	
	  // 1. 启动 PWM 输出
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);

  // 2. 强制将背光变量设为 0 (原本默认是 50)
	// 开启定时器，但立刻强制输出 0
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
// 同步变量状态
  gBacklightVal = 90;
  Update_Backlight(90);

	
  ILI9341_Init();	//显示屏初始化	
  FT6336_init();	//触摸屏初始化	
	mpu_init_dmp();	//mpu6050 dmp初始化	
	ap3216c_init();	//环境光传感器初始化
		SCD41_Init();		//SCD41 CO2传感器初始化
		printf("[INIT] SCD41_Init done\r\n");
	//ESP8266初始化，HAL库使用USART3
	ESP8266_Init(&huart5,(uint8_t *)gRX3_BufF,115200);
		printf("[INIT] ESP8266_Init done\r\n");
	//ESP8266_STA_TCPClient_Test();//TCP测试，进入子函数While(1)
	//ESP8266_STA_MQTTClient_Test();//MQTT测试，进入子函数While(1)
	//系统时间初始化
	System_Time_init();
		printf("[INIT] System_Time_init done\r\n");
	//使能ADC电源
	HAL_PWREx_EnableVddA();
  HAL_PWREx_EnableVddIO2();
	//ADC校准
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
  {
    /* Calibration Error */
    Error_Handler();
  }
  printf("[INIT] ADC Calibration done\r\n");
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_TIM_Base_Start_IT(&htim16);//开启定时器16开启,系统任务调度开始
  HAL_TIM_Base_Start_IT(&htim17);//开启定时器17开启,设备控制任务开始 
	//PWM输出启动-背光亮度
	Update_Backlight(gBacklightVal);	//设置背光亮度	
	//启动ADC组与DMA的常规转换
  if (HAL_ADC_Start_DMA(&hadc1,(uint32_t *)&gStruADC,ADC_CONVERTED_DATA_BUFFER_SIZE) != HAL_OK)	{Error_Handler();}

	printf("[INIT] All init done, entering main loop\r\n");
									//主循环
  while (1)
  {
    /* USER CODE END WHILE */
  MX_TouchGFX_Process();
    /* USER CODE BEGIN 3 */
		for(gTaskIndex = 0;gTaskIndex < OS_TASKLISTCNT;gTaskIndex++)
		{
			if((*g_OSTsakList[gTaskIndex]) != NULL)
			{
				g_OSTsakList[gTaskIndex]();
				g_OSTsakList[gTaskIndex] = NULL;  
			}
		}
	
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV1;
  RCC_OscInitStruct.PLL.PLLM = 3;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 1;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_0;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Power Configuration
  * @retval None
  */
static void SystemPower_Config(void)
{

  /*
   * Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
   */
  HAL_PWREx_DisableUCPDDeadBattery();
/* USER CODE BEGIN PWR */
/* USER CODE END PWR */
}

/* USER CODE BEGIN 4 */


//定时器16/17的任务分配
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint8_t p_Time16Cnt = 0,p_Time17Cnt = 0;
	/***************************************************************************************/
	//定时器16进行5ms任务中断
	if (htim->Instance == htim16.Instance) 
	{
		p_Time16Cnt++;
		//
		if(!(p_Time16Cnt % 4))  //20ms(50Hz)进行触发刷新
		{
			touchgfx_signalVSynTimer();  //touchgfx用户接口
		}
		//
		//五项按键读取
		if(!(p_Time16Cnt % 20))  //100ms进行一次窗口更新
		{
			if(gTaskEnMark.UPDATE_FIVEKEY_EN) g_OSTsakList[eUPDATE_FIVEKEY] = Update_FiveKey_Value; 	//更新五向键数据
		}
		//蜂鸣器与数码管控制
		if(!(p_Time16Cnt % 30))  //150ms检查USER按键，蜂鸣器打开
		{
			if(!HAL_GPIO_ReadPin(USER_KEY_GPIO_Port,USER_KEY_Pin))	//按下低电平，打开蜂鸣器
			{
				HAL_GPIO_WritePin(RUN_BEEP_GPIO_Port, RUN_BEEP_Pin, GPIO_PIN_SET);
			}
			else	
			{
				HAL_GPIO_WritePin(RUN_BEEP_GPIO_Port, RUN_BEEP_Pin, GPIO_PIN_RESET);	//释放高电平，关闭蜂鸣器
			}
		}
		//数码管位切换,5ms调用一次
		if(gTaskEnMark.UPDATE_NIXIE_SHOW_EN) g_OSTsakList[eUPDATE_NIXIE_SHOW] = Update_NixieDisplay; 	//数码管显示
		//1000ms运行一次，系统运行指示灯
		if(!(p_Time16Cnt % 200))  
		{
			p_Time16Cnt = 0; 
			RUN_LED_TURN_STATE();
		}
	}
	/***************************************************************************************/
	//定时器17进行100ms任务中断
	if (htim->Instance == htim17.Instance) 
	{
		p_Time17Cnt++;
		if(!(p_Time17Cnt % 2))  //200ms进行一次下列代码
		{
			if(gTaskEnMark.UPDATE_SIX_AXIS_EN) g_OSTsakList[eUPDATE_SIX_AXIS] = Update_EulerAngle; 	//欧拉角更新
		}
		if(!(p_Time17Cnt % 4))  //300ms进行一次下列代码
		{
			if(gTaskEnMark.UPDATE_APP_TASK_EN) g_OSTsakList[eUPDATE_APP_INFO] = Update_AppPageInfo;	//更新电压、电流、温湿度、光照度
		}
		if(!(p_Time17Cnt % 5))  //500ms进行一次下列代码
		{
			if(gTaskEnMark.UPDATE_WIFI_RSSI_EN) g_OSTsakList[eUPDATE_WIFI_RSSI] = ESP8266_RSSI_Task; 	//获取wifi连接的RSSI值
		}
		if(!(p_Time17Cnt % 10))  //1s进行一次下列代码
		{
			if(gTaskEnMark.UPDATE_TIME_EN) g_OSTsakList[eUPDATE_TIME] = Update_System_Time; 	//系统时间更新
		}
		if(!(p_Time17Cnt % 20))  //2s进行一次下列代码
		{
			if(gTaskEnMark.UPDATE_CHIPINFO_EN) g_OSTsakList[eUPDATE_CHIPINFO] = Update_ChipInfo; 	//系统信息更新
		}
		if(!(p_Time17Cnt % 30))  //3s进行一次下列代码
		{
			if((gTaskEnMark.UPDATE_HEART_RATE_EN) && (!gTaskStateBit.Max30102)) g_OSTsakList[eUPDATE_HEART_RATE] = Update_HeartRateInfo; 	//获取健康信息
		}
		if(!(p_Time17Cnt % 100))  //10s进行一次下列代码 
		{
			p_Time17Cnt = 0; 
		}
	}
	/***************************************************************************************/
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);
}
/**
  * @brief  EXTI line rising detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
  


	
	/* Prevent unused argument(s) compilation warning */
	UNUSED(GPIO_Pin);
	//触摸按下事件
	if((!HAL_GPIO_ReadPin(TP_INT_GPIO_Port,TP_INT_Pin)) && (GPIO_Pin == TP_INT_Pin))
	{
		gTaskStateBit.TouchPress = 1;
	}
	if(!HAL_GPIO_ReadPin(USER_KEY_GPIO_Port,USER_KEY_Pin))
	{
		gChatCount = gChatCount + 10;
		if(gChatCount>=100) gChatCount=10;
	}
		//火焰状态读取，释放状态为低电平，上报TouchGFX显示
	if((!HAL_GPIO_ReadPin(EXT_FIRE_GPIO_Port,EXT_FIRE_Pin)) && (GPIO_Pin == EXT_FIRE_Pin))
	{
		gTaskStateBit.ExtFIRE = 0; 
	}
	//光电开关状态读取，释放状态为低电平，联动蜂鸣器-关
	if((!HAL_GPIO_ReadPin(EXT_ITR_GPIO_Port,EXT_ITR_Pin)) && (GPIO_Pin == EXT_ITR_Pin))
	{
		gTaskStateBit.ExtITR = 0; 
		HAL_GPIO_WritePin(RUN_BEEP_GPIO_Port,RUN_BEEP_Pin,GPIO_PIN_RESET);	//关蜂鸣器
	}
	//人体红外状态读取，释放状态为低电平，联动直流风扇-关
	if(!HAL_GPIO_ReadPin(EXT_PIR_GPIO_Port,EXT_PIR_Pin))
	{
		gTaskStateBit.ExtPIR = 0; 
		HAL_GPIO_WritePin(FAN_GPIO_Port,FAN_Pin,GPIO_PIN_RESET);	//关闭风扇
	}
}
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
	UNUSED(GPIO_Pin);
	//触摸释放事件
	if(HAL_GPIO_ReadPin(TP_INT_GPIO_Port,TP_INT_Pin) && gTaskStateBit.TouchPress)
	{
		FT6336_irq_fuc();	//触摸中断产生
		gTaskStateBit.TouchPress = 0;	//清除触摸标志
	}
		//光电开关状态读取，初始状态低电平，联动蜂鸣器-开
	if(HAL_GPIO_ReadPin(EXT_ITR_GPIO_Port,EXT_ITR_Pin) && (GPIO_Pin == EXT_ITR_Pin))
	{
		gTaskStateBit.ExtITR = 1; 
		HAL_GPIO_WritePin(RUN_BEEP_GPIO_Port,RUN_BEEP_Pin,GPIO_PIN_SET);	//开蜂鸣器
	}
//	//五向按键按下
//	if(HAL_GPIO_ReadPin(EXT_FIVEKEY_GPIO_Port,EXT_FIVEKEY_Pin) && (GPIO_Pin == EXT_FIVEKEY_Pin))
//	{
//		gTaskStateBit.FiveKeyPress = 1; 
//	}
	//火焰状态读取，初始状态低电平，上报TouchGFX显示
	if(HAL_GPIO_ReadPin(EXT_FIRE_GPIO_Port,EXT_FIRE_Pin) && (GPIO_Pin == EXT_FIRE_Pin))
	{
		gTaskStateBit.ExtFIRE = 1; 
	}
	//人体红外状态读取，初始状态低电平，联动直流风扇-开
	if(HAL_GPIO_ReadPin(EXT_PIR_GPIO_Port,EXT_PIR_Pin) && (GPIO_Pin == EXT_PIR_Pin))
	{
		gTaskStateBit.ExtPIR = 1; 
		HAL_GPIO_WritePin(FAN_GPIO_Port,FAN_Pin,GPIO_PIN_SET);	//打开风扇
	}
	//物理按键的页面切换
	if(HAL_GPIO_ReadPin(SCREENKEY_GPIO_Port,SCREENKEY_Pin) && (GPIO_Pin == SCREENKEY_Pin))
	{
		KeyChangeScreen = 1;
	}
	
//--------------------------------------------------------------------------------------------------------------------
	  // --- 新增：NFC 感应触发 ---
  if (GPIO_Pin == GPIO_PIN_0) // 确认引脚是你配给 NFC 的那个（比如 PC12）
  {
      if (nfc_app_state == NFC_APP_IDLE) 
      {
          nfc_app_state = NFC_APP_WAIT_KEY;
          g_nfc_unlock_flag = 0;
          // 发送第一步：校验密钥
          NFC_Send_SetKey(&nfc, "1234567890AB"); 
          //printf("[NFC] Card Detected, Verifying...\r\n");
      }
  }
//----------------------------------------------------------------------------------------------------------------------------
}
/**
  * @brief  Conversion complete callback in non blocking mode 
  * @param  hadc: ADC handle
  * @note   This example shows a simple way to report end of conversion
  *         and get conversion result. You can add your own implementation.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  //更新DMA传输状态标志
  gTaskStateBit.ADCC = 1;  
}
/**
  * @brief  Rx Transfer completed callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//USART3接收数据
	if (huart->Instance == UART5) 
	{
		if(ESP8266_Fram_Record_Struct .InfBit .FramLength < ( RX_BUF_MAX_LEN - 1 ) ) 	//接收到的数据存储至buffer
		{
			//留最后一位做结束位
			ESP8266_Fram_Record_Struct .Data_RX_BUF[ ESP8266_Fram_Record_Struct .InfBit .FramLength ++ ]  = gRX3_BufF[0];  
			//UART3开启下一次接收
			HAL_UART_Receive_IT(&huart5,(uint8_t *)&gRX3_BufF, 1);//接收一个字节
		}  
	}

}
/**
  * @brief  UART Abort Receive Complete callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart)
{
	//USART3帧传输完成，产生空闲
	if (huart->Instance == UART5)
	{
		ESP8266_Fram_Record_Struct .InfBit .FramFinishFlag = 1;	//帧传输完成标志
		TcpClosedFlag = strstr ( ESP8266_Fram_Record_Struct .Data_RX_BUF, "CLOSED\r\n" ) ? 1 : 0;	//判断连接
		//UART3开启下一次接收
		HAL_UART_Receive_IT(&huart5,(uint8_t *)&gRX3_BufF, 1);//接收一个字节
	} 	
}
//----------------------------------------------------------------------------------------------------------------------
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == UART4) 
    {
        NFC_Result_t res = NFC_Analyze_And_Forward(&nfc, Size);
        if (res == NFC_RES_NONE) return;

        switch (nfc_app_state) {
            case NFC_APP_WAIT_KEY:
                if (res == NFC_RES_OK) {
                    nfc_app_state = NFC_APP_WAIT_READ;
                    NFC_Send_ReadBlock(&nfc, 4); // 密码对了，去读第4块
                } else {
                    nfc_app_state = NFC_APP_IDLE;
                }
                break;

            case NFC_APP_WAIT_READ:
                if (res == NFC_RES_READ_SUCCESS) {
                    g_nfc_unlock_flag = 1; // 【大功告成】设置成功标志
                    //printf("[NFC] Auth Success! Unlock Triggered.\r\n");
                }
                nfc_app_state = NFC_APP_IDLE;
                break;
            
            default: break;
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
