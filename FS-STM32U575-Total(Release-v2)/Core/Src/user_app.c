/**
  ******************************************************************************
  * @file   user_app.c
  * @brief  用户应用程序部分的代码
  * 
  ******************************************************************************
  */
#include "string.h"
#include "math.h"
//
#include "user_app.h"
#include "bsp_max30102.h"  // MAX30102 BSP header
/***************************心率血氧传感器***************************/
#define MAX30102_ENABLE
#define MAX30102_BUFF_LENGTH 200	//Defined in the "algorithm.h",MAX30102_BUFF_LENGTH = BUFFER_SIZE
//
#ifdef MAX30102_ENABLE
#define MAX_BRIGHTNESS 255
//
uint32_t aun_ir_buffer[MAX30102_BUFF_LENGTH]; //IR LED sensor data
int32_t n_ir_buffer_length;    //data length
uint32_t aun_red_buffer[MAX30102_BUFF_LENGTH];    //Red LED sensor data
int32_t n_sp02; //SPO2 value
int8_t ch_spo2_valid;   //indicator to show if the SP02 calculation is valid
int32_t n_heart_rate;   //heart rate value
float n_temperature;	//体表温度
int8_t  ch_hr_valid;    //indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;
//
extern void maxim_heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer,  int32_t n_ir_buffer_length, uint32_t *pun_red_buffer, int32_t *pn_spo2, int8_t *pch_spo2_valid, 
																									 int32_t *pn_heart_rate, int8_t  *pch_hr_valid);
#endif
/*********************************************************************/
extern ADC_ValTypeDef gStruADC;	//A/D通道采集结果，采用DMA循环连续采集模式
volatile uint16_t gFiveKeyVal = 4095;		//五向键原始值：0V/0.5V/1.0V/1.5V/2.0V	                  
volatile uint16_t gCurrentVal = 0;			//资源扩展板电流，通道IN8   
volatile uint16_t gVoltageVal = 0;			//资源扩展板电压，通道IN9        
volatile uint16_t gChipTempVal = 1037;	//内部参考电压，通道IN12  
volatile uint16_t gVrefVal = 1665;			//内部参考电压，通道IN13   	
volatile uint16_t gVbatVal = 4095;			//RTC电池电压，通道IN14   
//
volatile uint8_t gBacklightVal = 90;	//背光值，默认90%
//
extern volatile SHT20_TemRH_Val gTemRH_Val;
extern volatile StruAP3216C_Val gAP3216C_Val;	//AP3216数据结构
//系统时间
RTC_DateTypeDef gSystemDate;  //获取日期结构体
RTC_TimeTypeDef gSystemTime;   //获取时间结构体
//系统任务
gTask_BitDef gTaskStateBit;  //任务执行过程中使用到的标志位
gTask_MarkEN gTaskEnMark;  //系统任务使能标识
/**************************WiFi模组-获取RSSI**************************/
uint8_t gWiFiInfo[40];	//用于通知View界面的Text文本显示
wifiRSSI ao_wifiRSSI={0x0000,0x0000,0x0000};
#define AO_RSSI (efsm_t*)&ao_wifiRSSI
//状态机注册、触发与切换的宏定义
#define RSSI_EFSM_REG_STATE(STATE) (Efsm_RegState((AO_RSSI), (efsmState_t)(STATE)))//为当前对象注册state
#define RSSI_EFSM_EVT_TRIG(EVT) (Efsm_EvtTrig((AO_RSSI), (EVT)))//为当前对象发送事件
#define RSSI_EFSM_TRANS(STATE) (Efsm_Trans((AO_RSSI), (efsmState_t)(STATE)))//为当前对象切换状态
//获取RSSI的几个状态
void ESP8266_Send_AT(wifiRSSI *ao_wifi, uint16_t evt);
void ESP8266_Set_Mode(wifiRSSI *ao_wifi, uint16_t evt);
void ESP8266_Set_JoinAP(wifiRSSI *ao_wifi, uint16_t evt);
void ESP8266_Get_RSSI(wifiRSSI *ao_wifi, uint16_t evt);
/*********************************************************************/
/****************************姿态与运动检测***************************/
#define MPU6050_DMP_ENABLE
//
#ifdef MPU6050_DMP_ENABLE
//dmp相关参数设置
/* Starting sampling rate. */
#define DEFAULT_MPU_HZ  (100)

#define ACCEL_ON        (0x01)
#define GYRO_ON         (0x02)

#define MOTION          (0)
#define NO_MOTION       (1)
//
#define READ_MPU6050_DMP_QUAT
//
#ifdef READ_MPU6050_DMP_QUAT
//q30格式,long转float时的除数
#define q30  1073741824.0f
//需要读取的数据
volatile float pitch,roll,yaw; 		//欧拉角
short aacx,aacy,aacz;			//加速度传感器原始数据
short gyrox,gyroy,gyroz;	//陀螺仪原始数据
short temp;								//温度
#endif
unsigned int gSportStep = 0;	//运动步数
unsigned int gSportTime = 0;	//运动时间
//初始化过程中相关参数
struct hal_s {
    unsigned char sensors;
    unsigned char dmp_on;
    unsigned char wait_for_tap;
    volatile unsigned char new_gyro;
    unsigned short dmp_features;
    unsigned char motion_int_mode;
};
static struct hal_s hal = {0};
/* The sensors can be mounted onto the board in any orientation. The mounting
 * matrix seen below tells the MPL how to rotate the raw data from thei
 * driver(s).
 * TODO: The following matrices refer to the configuration on an internal test
 * board at Invensense. If needed, please modify the matrices to match the
 * chip-to-body matrix for your particular set up.
 */
static signed char gyro_orientation[9] = {-1, 0, 0,
                                           0,-1, 0,
                                           0, 0, 1};
#endif
/*********************************************************************/
void (* g_OSTsakList[OS_TASKLISTCNT])(void);
//系统时间初始化，编译时间																					 
void System_Time_init(void)
{
	int32_t lYear = 2026, lMonth = 5, lDate = 17;
	int32_t lweek = 0, weekBuff = 0;
	//
	uint32_t p_BKUPReadDRx = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
	//基姆拉尔森计算公式,根据输入的年月日输出星期几
	if(lMonth==1||lMonth==2) 
	{
		lMonth += 12;
		lYear--;
	}
	weekBuff=(lDate+2*lMonth+3*(lMonth+1)/5+lYear+lYear/4-lYear/100+lYear/400)%7;
	//
	switch(weekBuff)
	{
			case 0: lweek=RTC_WEEKDAY_MONDAY;			break;
			case 1: lweek=RTC_WEEKDAY_TUESDAY;		break;
			case 2: lweek=RTC_WEEKDAY_WEDNESDAY;	break;
			case 3: lweek=RTC_WEEKDAY_THURSDAY; 	break;
			case 4: lweek=RTC_WEEKDAY_FRIDAY; 		break;
			case 5: lweek=RTC_WEEKDAY_SATURDAY; 	break;
			case 6: lweek=RTC_WEEKDAY_SUNDAY; 		break;
	}
	//备份域数据读取，是否更新与保存编译时间
	if ((p_BKUPReadDRx & 0xFFFF) != RTC_BKP0RL_VAULE) 
	{
		/** Initialize RTC and set the Time and Date
		*/
		gSystemTime.Hours = 17;
		gSystemTime.Minutes = 0;
		gSystemTime.Seconds = 0;
		gSystemTime.SubSeconds = 0x0;
		gSystemTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		gSystemTime.StoreOperation = RTC_STOREOPERATION_RESET;
		if (HAL_RTC_SetTime(&hrtc, &gSystemTime, RTC_FORMAT_BIN) != HAL_OK)		{Error_Handler();}

		gSystemDate.WeekDay = lweek;
		gSystemDate.Month = 5;
		gSystemDate.Date = 17;
		gSystemDate.Year = 26;
		if (HAL_RTC_SetDate(&hrtc, &gSystemDate, RTC_FORMAT_BIN) != HAL_OK)		{Error_Handler();}
		//备份寄存器DR0用于存储时间是否设置
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, (p_BKUPReadDRx & 0xFFFF0000) | RTC_BKP0RL_VAULE);
		//
		BEEP_ENABLE();
		HAL_Delay(100);
		BEEP_DISABLE();
	}
}
/*
**********************************************************************
* @fun     :Update_System_Time 
* @brief   :系统时间更新,并通过核心板USB转串口打印
* @param   :None
* @return  :None 
* @remark  :周期性调用-1S运行一次
**********************************************************************
*/
void Update_System_Time(void)
{
	HAL_RTC_GetTime(&hrtc, &gSystemTime, RTC_FORMAT_BIN);//获取时间
	/* Get the RTC current Date */
	HAL_RTC_GetDate(&hrtc, &gSystemDate, RTC_FORMAT_BIN);//获取日期
	//输出系统时间
	#ifdef DEBUG_MODE	
		printf("sys_date:%04d%02d%02d\n\r", gSystemDate.Year+2000, gSystemDate.Month, gSystemDate.Date);
		printf("sys_time:%02d%02d%02d\n\r", gSystemTime.Hours, gSystemTime.Minutes, gSystemTime.Seconds);
	#endif // DEBUG
}
/**************************获取RSSI-相关函数**************************/
//M0对ESP8266发送AT指令,返回OK进入下一个状态
void ESP8266_Send_AT(wifiRSSI *ao_wifi, uint16_t evt)	
{
	//初始化RSSI值
	ao_wifiRSSI.gRSSI = 200;	//值越大，信号越弱
	switch (evt)
	{
		case EFSM_EVT_EXIT:
			sprintf((char*)gWiFiInfo,"%d:WiFi Send AT-Exit\n", ao_wifi->tickCount);
			//printf("%d:ESP8266_Send_AT, Exit\r\n",ao_wifi->tickCount);
			break;
		case EFSM_EVT_ENTRY:
			ao_wifi->tickCount = 0;	//计数清零
			sprintf((char*)gWiFiInfo,"%d:WiFi Send AT-Entry\n", ao_wifi->tickCount);
			//printf("%d:ESP8266_Send_AT, Entry\r\n",ao_wifi->tickCount);
			break;
		case EFSM_EVT_TICK:
			++ao_wifi->tickCount;
			//发送AT测试指令
			if(ESP8266_Send_AT_Cmd("AT","OK",NULL,100)) 
			{
				RSSI_EFSM_TRANS(ESP8266_Set_Mode);
      }
			else	//打印Tick用于调试
			{
				sprintf((char*)gWiFiInfo,"%d:WiFi SendAT-Tick\n", ao_wifi->tickCount);
				//printf("%d:ESP8266_Send_AT, Tick\r\n",ao_wifi->tickCount);
			}
			break;
	}
}	
//选择ESP8266的工作模式,返回OK进入下一个状态
void ESP8266_Set_Mode(wifiRSSI *ao_wifi, uint16_t evt)	
{
	switch (evt)
	{
		case EFSM_EVT_EXIT:
			sprintf((char*)gWiFiInfo,"%d:WiFi Set MODE STA-Exit\n", ao_wifi->tickCount);
			//printf("%d:ESP8266_Set_Mode, Exit\r\n",ao_wifi->tickCount);
			break;
		case EFSM_EVT_ENTRY:
			ao_wifi->tickCount = 0;	//计数清零
			sprintf((char*)gWiFiInfo,"%d:WiFi Set MODE STA-Entry\n", ao_wifi->tickCount);
			//printf("%d:ESP8266_Set_Mode, Entry\r\n",ao_wifi->tickCount);
			break;
		case EFSM_EVT_TICK:
			++ao_wifi->tickCount;
			//设置ESP8266进入STA模式
			if(ESP8266_Send_AT_Cmd("AT+CWMODE=1", "OK", "no change", 100)) 
			{
				RSSI_EFSM_TRANS(ESP8266_Set_JoinAP);
			}
			else	//打印Tick用于调试
			{
				sprintf((char*)gWiFiInfo,"%d:WiFi Set MODE STA-Tick\n", ao_wifi->tickCount);
				//printf("%d:ESP8266_Set_Mode, Tick\r\n",ao_wifi->tickCount);
			}
		break;
	}
}
//设置ESP8266加入AP,返回OK进入下一个状态
void ESP8266_Set_JoinAP(wifiRSSI *ao_wifi, uint16_t evt)	
{
  char cCmd [120];
	sprintf(cCmd, "AT+CWJAP=\"%s\",\"%s\"", User_ESP8266_SSID, User_ESP8266_PWD);
	switch (evt)
	{
		case EFSM_EVT_EXIT:
			sprintf((char*)gWiFiInfo,"%d:WiFi JoinAP-Exit\n", ao_wifi->tickCount);
			//printf("%d:ESP8266_Set_JoinAP, Exit\r\n",ao_wifi->tickCount);
			break;
		case EFSM_EVT_ENTRY:
			ao_wifi->tickCount = 0;	//计数清零
			sprintf((char*)gWiFiInfo,"%d:WiFi JoinAP-Entry\n", ao_wifi->tickCount);
			//printf("%d:ESP8266_Set_JoinAP, Entry\r\n",ao_wifi->tickCount);
			break;
		case EFSM_EVT_TICK:
			++ao_wifi->tickCount;
			//设置ESP8266加入AP，状态中存在ESP8266的数据接收处理的延时，可以优化发送与接收部分的逻辑
			if(ESP8266_Send_AT_Cmd(cCmd, "OK", NULL, 3000)) 
			{
				RSSI_EFSM_TRANS(ESP8266_Get_RSSI);
			}
			else	//打印Tick用于调试
			{
				sprintf((char*)gWiFiInfo,"%d:WiFi JoinAP-Tick\n", ao_wifi->tickCount);	
				//printf("%d:ESP8266_Set_JoinAP, Tick\r\n",ao_wifi->tickCount);
			}
		break;
	}
}
//获取ESP8266连接的RIIS强度，返回NO AP时,信号强度复位
void ESP8266_Get_RSSI(wifiRSSI *ao_wifi, uint16_t evt)	
{
	char *str_xx=",-";	//用来获取RSSI
	char *loc;
	char *token;
	//
	switch (evt)
	{
		case EFSM_EVT_EXIT:
			sprintf((char*)gWiFiInfo,"%d:WiFi Get RSSI-Exit\n", ao_wifi->tickCount);
			//printf("%d:ESP8266_Get_RSSI, Exit\r\n",ao_wifi->tickCount);
			break;
		case EFSM_EVT_ENTRY:
			ao_wifi->tickCount = 0;	//计数清零
			sprintf((char*)gWiFiInfo,"%d:WiFi Get RSSI-Entry\n", ao_wifi->tickCount);
			//printf("%d:ESP8266_Get_RSSI, Entry\r\n",ao_wifi->tickCount);
			break;
		case EFSM_EVT_TICK:
			++ao_wifi->tickCount;
			//发送RSSI查询指令
			if(ESP8266_Send_AT_Cmd("AT+CWJAP_CUR?", "+CWJAP_CUR:", NULL, 200)) 
			{
				loc=strstr((char*)ESP8266_Fram_Record_Struct.Data_RX_BUF,str_xx);
				token = strtok(loc,str_xx); 
				ao_wifiRSSI.gRSSI = atoi((char*)token);	//赋值信号强度
				//打印信号强度
				sprintf((char*)gWiFiInfo,"ESP8266 Get RSSI = %d\n", ao_wifiRSSI.gRSSI);
				//printf("ESP8266_Get_RSSI = %d\r\n",ao_wifiRSSI.gRSSI);
			}
			else	//打印Tick用于调试
			{
				sprintf((char*)gWiFiInfo,"%d:No AP(HQYJ-YF) found\n", ao_wifi->tickCount);
				//printf("%d:No AP(HQYJ-YF) found, Tick\r\n",ao_wifi->tickCount);
			}
		break;
	}
}
/*
**********************************************************************
* @fun     :ESP8266_RSSI_Task
* @brief   :连接AP获取RSSI信号强度
* @remark  :首先发送AT测试指令，设置模式，加入AP，获取RSSI
**********************************************************************
*/
void ESP8266_RSSI_Task(void)
{
	if(!ao_wifiRSSI.gWiFiTag.EFSMInit)	//进行状态机的初始化
	{
		Efsm_Ctor((efsm_t*)&ao_wifiRSSI, (efsmState_t)ESP8266_Send_AT);
		ao_wifiRSSI.gRSSI = 0;	//信号强度
		RSSI_EFSM_REG_STATE(ESP8266_Send_AT);	//提前注册state，用于Efsm_Trans()的参数检测,注册-WIFI测试AT指令
		RSSI_EFSM_REG_STATE(ESP8266_Set_Mode);	//注册-WIFI设置模式
		RSSI_EFSM_REG_STATE(ESP8266_Set_JoinAP);	//注册-WIFI加入AP
		RSSI_EFSM_REG_STATE(ESP8266_Get_RSSI);	//注册-获取WIFI的RSSI
		//
		ao_wifiRSSI.gWiFiTag.EFSMInit = 1;	//FSM初始化仅执行一次
	}
	//运行EFSM状态机
	Efsm_EvtTrig((efsm_t*)&ao_wifiRSSI, EFSM_EVT_TICK);	//500mS触发一次状态
	Efsm_Hand((efsm_t*)&ao_wifiRSSI);
}
/*
**********************************************************************
* @fun     :Update_FiveKey_Value 
* @brief   :更新五向键数据,并通过核心板USB转串口打印
* @param   :None
* @return  :None 
* @remark  :周期性调用-100mS运行一次
**********************************************************************
*/
void Update_FiveKey_Value(void)
{
	//恢复默认值
	gFiveKeyVal = 4095;	//通过五向键外部中断触发更新数据
	if((gTaskStateBit.ADCC) && (gTaskStateBit.FiveKeyPress))	//DMA采集完成与五向键按下
	{
		gFiveKeyVal = gStruADC.Key_Val;	//保存按键值
		//打印输出
		#ifdef DEBUG_MODE	
			printf("FiveKey Value(V): %.4f\n\r",(gStruADC.Key_Val)*3.3f/4095);	//获取五向键电压值		
		#endif // DEBUG
		//转换状态复位
		gTaskStateBit.ADCC = 0;
	}
}
#ifdef MPU6050_DMP_ENABLE
/* These next two functions converts the orientation matrix (see
 * gyro_orientation) to a scalar representation for use by the DMP.
 * NOTE: These functions are borrowed from Invensense's MPL.
 */
static inline unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}
static inline unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx)
{
    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}
/*
**********************************************************************
* @fun     :mpu_init_dmp 
* @brief   :初始化dmp
* @param   :
* @return  :None 
**********************************************************************
*/
void mpu_init_dmp(void)
{
	int dmp_Init_Fail = 0;	//mpu6050初始化是否成功
	unsigned char accel_fsr;
	unsigned short gyro_rate, gyro_fsr;
  //unsigned long timestamp;
	//
	dmp_Init_Fail = mpu_init();  //失败返回1，成功返回O
	//
	while(dmp_Init_Fail)	
	{
		dmp_Init_Fail = mpu_init();	
		HAL_Delay(100);
	}
	
  /* Get/set hardware configuration. Start gyro. */
  /* Wake up all sensors. */
  mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);	//配置陀螺仪和加速计传感器的时钟和工作状态函数
  /* Push both gyro and accel data into the FIFO. */
  mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);	//配置陀螺仪和加速计开启FIFO通道函数
  mpu_set_sample_rate(DEFAULT_MPU_HZ);	//配置默认的采样率
  /* Read back configuration in case it was set improperly. */
  mpu_get_sample_rate(&gyro_rate);	//获得陀螺仪采样频率范围
  mpu_get_gyro_fsr(&gyro_fsr);	//得陀螺仪全尺寸范围
  mpu_get_accel_fsr(&accel_fsr);	//得加速计全尺寸范围

  /* Initialize HAL state variables. */
  memset(&hal, 0, sizeof(hal));	//初始化hal的变量
  hal.sensors = ACCEL_ON | GYRO_ON;	//标志位-"开启传感器"设置为加速计和陀螺仪

  /* To initialize the DMP:
   * 1. Call dmp_load_motion_driver_firmware(). This pushes the DMP image in
   *    inv_mpu_dmp_motion_driver.h into the MPU memory.
   * 2. Push the gyro and accel orientation matrix to the DMP.
   * 3. Register gesture callbacks. Don't worry, these callbacks won't be
   *    executed unless the corresponding feature is enabled.
   * 4. Call dmp_enable_feature(mask) to enable different features.
   * 5. Call dmp_set_fifo_rate(freq) to select a DMP output rate.
   * 6. Call any feature-specific control functions.
   *
   * To enable the DMP, just call mpu_set_dmp_state(1). This function can
   * be called repeatedly to enable and disable the DMP at runtime.
   *
   * The following is a short summary of the features supported in the DMP
   * image provided in inv_mpu_dmp_motion_driver.c:
   * DMP_FEATURE_LP_QUAT: Generate a gyro-only quaternion on the DMP at
   * 200Hz. Integrating the gyro data at higher rates reduces numerical
   * errors (compared to integration on the MCU at a lower sampling rate).
   * DMP_FEATURE_6X_LP_QUAT: Generate a gyro/accel quaternion on the DMP at
   * 200Hz. Cannot be used in combination with DMP_FEATURE_LP_QUAT.
   * DMP_FEATURE_TAP: Detect taps along the X, Y, and Z axes.
   * DMP_FEATURE_ANDROID_ORIENT: Google's screen rotation algorithm. Triggers
   * an event at the four orientations where the screen should rotate.
   * DMP_FEATURE_GYRO_CAL: Calibrates the gyro data after eight seconds of
   * no motion.
   * DMP_FEATURE_SEND_RAW_ACCEL: Add raw accelerometer data to the FIFO.
   * DMP_FEATURE_SEND_RAW_GYRO: Add raw gyro data to the FIFO.
   * DMP_FEATURE_SEND_CAL_GYRO: Add calibrated gyro data to the FIFO. Cannot
   * be used in combination with DMP_FEATURE_SEND_RAW_GYRO.
   */
	 dmp_load_motion_driver_firmware();	//加载DMP固件
   dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));	//推送陀螺仪和加速度计的方向矩阵到DMP
		//
   hal.dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
       DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
       DMP_FEATURE_GYRO_CAL;	//DMP的功能开启选项标志位设置
   dmp_enable_feature(hal.dmp_features);	//使能上述功能
   dmp_set_fifo_rate(DEFAULT_MPU_HZ);	//置DMP的FIFO速率
   mpu_set_dmp_state(1);	//开启DMP
   hal.dmp_on = 1;	//标志位"DMP状态"为开启
}
/*
**********************************************************************
* @fun     :mpu_get_dmp_data 
* @brief   :读取dmp的数据
* @param   :
* @return  :None 
**********************************************************************
*/
static void mpu_get_dmp_data(void)
{ 
	#ifdef READ_MPU6050_DMP_QUAT
	float q0=1.0f, q1=0.0f, q2=0.0f,q3=0.0f;
	unsigned long sensor_timestamp;
	short gyro[3], accel[3], sensors;
	unsigned char more;
	long quat[4];
	/* This function gets new data from the FIFO when the DMP is in
	 * use. The FIFO can contain any combination of gyro, accel,
	 * quaternion, and gesture data. The sensors parameter tells the
	 * caller which data fields were actually populated with new data.
	 * For example, if sensors == (INV_XYZ_GYRO | INV_WXYZ_QUAT), then
	 * the FIFO isn't being filled with accel data.
   * The driver parses the gesture data to determine if a gesture
 	 * event has occurred; on an event, the application will be notified
	 * via a callback (assuming that a callback function was properly
	 * registered). The more parameter is non-zero if there are
	 * leftover packets in the FIFO.
	 */
	dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
	//
	if (!more)	hal.new_gyro = 0;
	/* Gyro and accel data are written to the FIFO by the DMP in chip
	 * frame and hardware units. This behavior is convenient because it
	 * keeps the gyro and accel outputs of dmp_read_fifo and
	 * mpu_read_fifo consistent.
	 */
	if (sensors & INV_XYZ_GYRO)		{;}
	if (sensors & INV_XYZ_ACCEL)	{;}
	/* Unlike gyro and accel, quaternions are written to the FIFO in
	 * the body frame, q30. The orientation is set by the scalar passed
	 * to dmp_set_orientation during initialization.
   */
	if (sensors & INV_WXYZ_QUAT)
	{
		q0=quat[0] / q30;
		q1=quat[1] / q30;
		q2=quat[2] / q30;
		q3=quat[3] / q30;
		//
		pitch	= asin( (-2) * q1 * q3 + 2 * q0 * q2 ) * 57.3f;
		roll	= atan2( 2 * q2 * q3 + 2 * q0 * q1 , (-2) * q1 * q1 - 2 * q2 * q2 + 1) * 57.3f;
		yaw 	= atan2( 2 * (q1 * q2 + q0 * q3) , q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 57.3f;
	}
	#endif
	unsigned long pSportStep = 0;	//运动步数
	unsigned long pSportTime = 0;	//运动时间
	//获取步数
	dmp_get_pedometer_step_count(&pSportStep);
	dmp_get_pedometer_walk_time(&pSportTime);
	//
	gSportStep = pSportStep;
	gSportTime = pSportTime;
}
#endif

/*
**********************************************************************
* @fun     :Update_EulerAngle 
* @brief   :更新欧拉角,并通过核心板USB转串口打印
* @param   :None
* @return  :None 
* @remark  :周期性调用-200mS运行一次
**********************************************************************
*/
void Update_EulerAngle(void)
{
	mpu_get_dmp_data(); 
	//输出系统时间
	#ifdef DEBUG_MODE	
		printf("pitch:%.2f°, roll:%.2f°,yaw:%.2f°\n\r", pitch, roll, yaw); 
	#endif // DEBUG
}
/*
**********************************************************************
* @fun     :Update_NixieDisplay 
* @brief   :四位数码管显示，150ms周期性调用
* @param   :pData显示的数据，每4位一组，数字的范围0-9
* @remark  :
**********************************************************************
*/
const uint8_t DigitTable[]={0x3f,0x06,0x5b,0x4f, 0x66,0x6d,0x7d,0x07, 0x7f,0x6f};	//数字显示
volatile uint16_t gNixieShowData = 0x0000;	
//
void Update_NixieDisplay()
{
	uint8_t pDig4Data[2] = {0x00,0x00};	
	static uint8_t pDig4CS = 0;	
	
	switch(pDig4CS)
	{
		case 0:
			pDig4Data[0] = 0x10;
			pDig4Data[1] = DigitTable[(gNixieShowData & 0x000F)];
			HAL_SPI_Transmit(&hspi2,pDig4Data,2, 1);

			HAL_GPIO_WritePin(HC595_RCLK_GPIO_Port, HC595_RCLK_Pin, GPIO_PIN_SET);
			__NOP(); __NOP(); __NOP(); __NOP(); // minimal pulse width for 74HC595 latch (~25ns @160MHz)
			HAL_GPIO_WritePin(HC595_RCLK_GPIO_Port, HC595_RCLK_Pin, GPIO_PIN_RESET);
			pDig4CS++;
		break;
		case 1:
			pDig4Data[0] = 0x08;
			pDig4Data[1] = DigitTable[((gNixieShowData >> 4) & 0x000F)];
			HAL_SPI_Transmit(&hspi2,pDig4Data,2, 1);

			HAL_GPIO_WritePin(HC595_RCLK_GPIO_Port, HC595_RCLK_Pin, GPIO_PIN_SET);
			__NOP(); __NOP(); __NOP(); __NOP();
			HAL_GPIO_WritePin(HC595_RCLK_GPIO_Port, HC595_RCLK_Pin, GPIO_PIN_RESET);
			pDig4CS++;
		break;
		case 2:
			pDig4Data[0] = 0x04;
			pDig4Data[1] = DigitTable[((gNixieShowData >> 8) & 0x000F)];
			HAL_SPI_Transmit(&hspi2,pDig4Data,2, 1);

			HAL_GPIO_WritePin(HC595_RCLK_GPIO_Port, HC595_RCLK_Pin, GPIO_PIN_SET);
			__NOP(); __NOP(); __NOP(); __NOP();
			HAL_GPIO_WritePin(HC595_RCLK_GPIO_Port, HC595_RCLK_Pin, GPIO_PIN_RESET);
			pDig4CS++;
		break;
		case 3:
			pDig4Data[0] = 0x02;
			pDig4Data[1] = DigitTable[((gNixieShowData >> 12) & 0x000F)];
			HAL_SPI_Transmit(&hspi2,pDig4Data,2, 1);

			HAL_GPIO_WritePin(HC595_RCLK_GPIO_Port, HC595_RCLK_Pin, GPIO_PIN_SET);
			__NOP(); __NOP(); __NOP(); __NOP();
			HAL_GPIO_WritePin(HC595_RCLK_GPIO_Port, HC595_RCLK_Pin, GPIO_PIN_RESET);
			pDig4CS = 0;
		break;
	}
}
/*
**********************************************************************
* @fun     :Update_Backlight
* @brief   :更新背光亮度
* @param   :pDutyRatio占空比5-100
* @remark  :定时器16的通道1-PWM输出模式，时钟分频64-1为1MHz，自动重转载1000-1,比较值50-1000调节
**********************************************************************
*/
void Update_Backlight(uint8_t pDutyRatio)
{		
	//参数检查
	if((pDutyRatio < 5) || (pDutyRatio > 100)) return;
	//停止PWM信号
 	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
	//设置占空比值
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3,  pDutyRatio*10);
	//设置当前计数值
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	//启动PWM信号
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
}	
/*
**********************************************************************
* @fun     :Update_ChipInfo 
* @brief   :更新芯片温度、参考电压、VBAT数据,并通过核心板USB转串口打印
* @param   :None
* @return  :None 
* @remark  :周期性调用-2S运行一次
**********************************************************************
*/
void Update_ChipInfo(void)
{
	if((gTaskStateBit.ADCC))	//DMA采集完成
	{
		gChipTempVal =	((((gStruADC.Temp_Val)*3.3f/4095)*1000)-685)/2.5;	//更新温度值
		gVrefVal =	gStruADC.Vref_Val;	//更新参考电压值
		gVbatVal =	gStruADC.Vbat_Val;	//更新VBAT值
		//打印输出
		#ifdef DEBUG_MODE	
			/*
			1.温度与电压的斜率2.5mV/°C，TS_CAL1是在30度参考电压3V测试的值为TS_CAL1 = 1037(电压为760mv)，按照直线方程y=kx+b,计算得出b=685mV
			2.计算温度的公式为x=(y-685)/2.5,注意y需要转换为mV
			*/
			printf("Chip Temperature(°C): %.4f\n\r",((((gStruADC.Temp_Val)*3.3f/4095)*1000)-685)/2.5);
			/*
			1.用于纠正VREF+的公式VREF+ = VREF_Charac × VREFINT_CAL / VREFINT_DATA
			2.VREF+ = 3.0 x (*VREFINT_CAL) / gStruADC.Vref_Val    ->   例： 3 x 1645 / 1484 = 3.3  建议对gStruADC.Vref_Val进行多次取值滤波
			*/
			printf("Reference voltage correction value: %.4f \n\r",(*VREFINT_CAL)*3.0f/4095); 
			printf("ADC data register(ADC_DR): %d\n\r",gStruADC.Vref_Val);
			printf("Vref True Voltage(V): %.4f\n\r",(gStruADC.Vref_Val)*3.3f/4095);
			//RTC电池电压
			printf("RTC Battery(V): %.4f\n\r", (gStruADC.Vbat_Val)*4*3.3f/4095);
		#endif // DEBUG
		//转换状态复位
		gTaskStateBit.ADCC = 0;
	}
}
/*
**********************************************************************
* @fun     :Update_AppPageInfo
* @brief   :更新APP页面信息
* @param   :None
* @remark  :更新电压、电流、温湿度、光照度
**********************************************************************
*/
void Update_AppPageInfo(void)
{
	if((gTaskStateBit.ADCC))	//DMA采集完成
	{
		gCurrentVal =	gStruADC.AMP_Val;	//更新电流值
		gVoltageVal =	gStruADC.VDC_Val;	//更新电压值
		//打印输出
		#ifdef DEBUG_MODE	
			printf("PAX-E Current(mA): %.4f\n\r",(gStruADC.AMP_Val)*100*3.3f/4095);	//获取电流值，电压放大100倍，采样电阻0.1R，电流mA
			printf("PAX-E Voltage(V): %.4f\n\r", (gStruADC.VDC_Val)*3.3f/4095); 		//获取电压值，硬件电路无放大
		#endif // DEBUG
		//转换状态复位
		gTaskStateBit.ADCC = 0;
	}	
	//读取ALS值与PS值
	gAP3216C_Val.ALS = ap3216c_read_ambient_light();
	//gAP3216C_Val.PS = ap3216c_read_ps_data();
	//gAP3216C_Val.IR = ap3216c_read_ir_data();
	//读取温湿度数据
	BSP_SHT20_GetData();
	//读取CO2数据
	SCD41_GetData();
}
#ifdef MAX30102_ENABLE
// ── Heart rate state machine (non-blocking, single-sample yield per iteration) ──
//
//  Each call processes exactly one FIFO read and yields via g_hr_continue_flag,
//  so MX_TouchGFX_Process() runs every loop iteration — no more screen freeze.
//

#define FINGER_PRESENT_THRESHOLD  50000
#define FINGER_CHECK_SAMPLES      3
#define FIFO_WAIT_TIMEOUT_MS      100

static uint32_t g_last_valid_hr_time = 0;
static uint8_t  g_sensor_inited = 0;

#define HR_STATE_FINGER_CHECK   0
#define HR_STATE_INIT_SENSOR    1
#define HR_STATE_COLLECT_200    2
#define HR_STATE_CALCULATE      3
#define HR_STATE_ROLLING_SHIFT  4
#define HR_STATE_ROLLING_READ   5
#define HR_STATE_RESET_FIFO     6
#define HR_STATE_DONE           7

static uint8_t  hr_state = HR_STATE_FINGER_CHECK;
static uint16_t hr_sample_idx = 0;
static uint8_t  hr_first_measurement = 1;
uint8_t g_hr_continue_flag = 0;

// Non-blocking FIFO wait helpers
static uint32_t hr_fifo_wait_start = 0;   // tick at start of current FIFO wait
static uint32_t hr_finger_ir_sum  = 0;    // accumulated IR sum during finger check
static uint8_t  hr_finger_samples  = 0;   // samples collected in finger check
static uint32_t hr_finger_red = 0;        // placeholder for read_fifo in finger check

// Returns 0 = no finger, 1 = finger detected, 2 = still working (call again)
static uint8_t max30102_check_finger(void)
{
    if (hr_finger_samples == 0)
    {
        // First call — clear status registers
        uint8_t dummy = 0;
        maxim_max30102_read_reg(REG_INTR_STATUS_1, &dummy);
        maxim_max30102_read_reg(REG_INTR_STATUS_2, &dummy);
        hr_finger_ir_sum   = 0;
        hr_finger_samples  = 0;
        hr_fifo_wait_start = 0;
    }

    if (hr_finger_samples >= FINGER_CHECK_SAMPLES)
    {
        // All samples collected — evaluate
        uint32_t ir_avg = hr_finger_ir_sum / FINGER_CHECK_SAMPLES;
        printf("[FINGER] samples=%d, avg_IR=%u, threshold=%u\r\n",
            FINGER_CHECK_SAMPLES, ir_avg, (uint32_t)FINGER_PRESENT_THRESHOLD);
        hr_finger_samples = 0;
        return (ir_avg > FINGER_PRESENT_THRESHOLD) ? 1 : 0;
    }

    // Try to read one sample from FIFO
    uint8_t dummy = 0;
    maxim_max30102_read_reg(REG_INTR_STATUS_1, &dummy);
    if ((dummy & 0xC0) == 0x00)
    {
        if (hr_fifo_wait_start == 0)
            hr_fifo_wait_start = HAL_GetTick();
        if (HAL_GetTick() - hr_fifo_wait_start < FIFO_WAIT_TIMEOUT_MS)
            return 2;           // still waiting — yield to main loop
        // Timeout — finger not present
        printf("[FINGER] timeout, samples=%d\r\n", hr_finger_samples);
        hr_finger_samples = 0;
        return 0;
    }

    hr_fifo_wait_start = 0;
    uint32_t red, ir;
    maxim_max30102_read_fifo(&red, &ir);
    hr_finger_ir_sum += ir;
    hr_finger_samples++;
    (void)red;
    return 2;   // got a sample, need more — yield to main loop
}

void Update_HeartRateInfo(void)
{
    uint8_t dummy;

    switch (hr_state) {

    case HR_STATE_FINGER_CHECK: {
        uint8_t result = max30102_check_finger();
        if (result == 2)
        {
            // Still working — yield to main loop
            g_hr_continue_flag = 1;
            break;
        }

        if (result == 0)
        {
            ch_hr_valid   = 0;
            ch_spo2_valid = 0;
            n_heart_rate  = 0;
            n_sp02        = 0;

            if (g_last_valid_hr_time != 0
                && (HAL_GetTick() - g_last_valid_hr_time) > 10000)
            {
                ch_hr_valid   = 1;
                ch_spo2_valid = 1;
                g_last_valid_hr_time = HAL_GetTick();
                printf("[HR] 10s timeout, force UI update\r\n");
            }
            hr_state = HR_STATE_DONE;
            g_hr_continue_flag = 1;
        }
        else  // result == 1, finger detected
        {
            hr_state = HR_STATE_INIT_SENSOR;
            g_hr_continue_flag = 1;
        }
        break;
    }

    case HR_STATE_INIT_SENSOR:
        if (!g_sensor_inited)
        {
            maxim_max30102_init();
            g_sensor_inited = 1;
        }
        n_ir_buffer_length = MAX30102_BUFF_LENGTH;
        hr_sample_idx = 0;
        hr_fifo_wait_start = 0;
        hr_state = HR_STATE_COLLECT_200;
        g_hr_continue_flag = 1;
        break;

    case HR_STATE_COLLECT_200:
        if (hr_sample_idx >= MAX30102_BUFF_LENGTH)
        {
            hr_state = HR_STATE_CALCULATE;
            g_hr_continue_flag = 1;
            break;
        }

        // Single-sample non-blocking read
        dummy = 0;
        maxim_max30102_read_reg(REG_INTR_STATUS_1, &dummy);
        if ((dummy & 0xC0) == 0x00)
        {
            if (hr_fifo_wait_start == 0)
                hr_fifo_wait_start = HAL_GetTick();
            if (HAL_GetTick() - hr_fifo_wait_start < FIFO_WAIT_TIMEOUT_MS)
            {
                g_hr_continue_flag = 1;   // yield to main loop
                break;
            }
            printf("[HR] FIFO timeout at init sample %d\r\n", hr_sample_idx);
            hr_state = HR_STATE_CALCULATE; // proceed with what we have
            g_hr_continue_flag = 1;
            break;
        }

        hr_fifo_wait_start = 0;
        maxim_max30102_read_fifo((aun_red_buffer + hr_sample_idx),
                                 (aun_ir_buffer + hr_sample_idx));
#ifdef DEBUG_MODE
        printf("%i,%i\n\r", aun_red_buffer[hr_sample_idx], aun_ir_buffer[hr_sample_idx]);
#endif
        hr_sample_idx++;
        g_hr_continue_flag = 1;
        break;

    case HR_STATE_CALCULATE:
        // Use actual collected count, not MAX30102_BUFF_LENGTH
        // (hr_sample_idx may be less than 200 if FIFO timed out)
        n_ir_buffer_length = hr_sample_idx;
        maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length,
            aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);

        {
            int8_t  integer_temperature;
            uint8_t fractional_temperature;
            maxim_max30102_read_temperature(&integer_temperature, &fractional_temperature);
            n_temperature = integer_temperature + ((float)fractional_temperature) / 16.0f;
        }

        if (!ch_hr_valid || !ch_spo2_valid)
        {
            n_heart_rate = 0;
            n_sp02 = 0;
        }

        printf("[HR] Calculated: HR=%d, SpO2=%d, valid=%d/%d (samples=%d)\r\n",
            n_heart_rate / 4, n_sp02, ch_hr_valid, ch_spo2_valid, n_ir_buffer_length);

        if (ch_hr_valid || ch_spo2_valid)
            g_last_valid_hr_time = HAL_GetTick();

        if (hr_first_measurement)
        {
            hr_first_measurement = 0;
            printf("[HR] First measurement, skip rolling read\r\n");
            hr_state = HR_STATE_RESET_FIFO;
        }
        else
        {
            printf("[HR] Starting rolling read...\r\n");
            hr_state = HR_STATE_ROLLING_SHIFT;
        }
        g_hr_continue_flag = 1;
        break;

    case HR_STATE_ROLLING_SHIFT: {
        uint16_t i;
        for (i = 100; i < MAX30102_BUFF_LENGTH; i++)
        {
            aun_red_buffer[i - 100] = aun_red_buffer[i];
            aun_ir_buffer[i - 100] = aun_ir_buffer[i];
        }
        hr_sample_idx = 100;
        hr_fifo_wait_start = 0;
        hr_state = HR_STATE_ROLLING_READ;
        g_hr_continue_flag = 1;
        break;
    }

    case HR_STATE_ROLLING_READ:
        if (hr_sample_idx >= MAX30102_BUFF_LENGTH)
        {
            printf("[HR] Rolling read done\r\n");
            hr_state = HR_STATE_RESET_FIFO;
            g_hr_continue_flag = 1;
            break;
        }

        // Single-sample non-blocking read
        dummy = 0;
        maxim_max30102_read_reg(REG_INTR_STATUS_1, &dummy);
        if ((dummy & 0xC0) == 0x00)
        {
            if (hr_fifo_wait_start == 0)
                hr_fifo_wait_start = HAL_GetTick();
            if (HAL_GetTick() - hr_fifo_wait_start < FIFO_WAIT_TIMEOUT_MS)
            {
                g_hr_continue_flag = 1;   // yield to main loop
                break;
            }
            printf("[HR] Roll FIFO timeout at sample %d\r\n", hr_sample_idx);
            hr_state = HR_STATE_RESET_FIFO; // proceed
            g_hr_continue_flag = 1;
            break;
        }

        hr_fifo_wait_start = 0;
        maxim_max30102_read_fifo((aun_red_buffer + hr_sample_idx),
                                 (aun_ir_buffer + hr_sample_idx));
#ifdef DEBUG_MODE
        printf("%i,%i\n\r", aun_red_buffer[hr_sample_idx], aun_ir_buffer[hr_sample_idx]);
#endif
        hr_sample_idx++;
        g_hr_continue_flag = 1;
        break;

    case HR_STATE_RESET_FIFO:
        maxim_max30102_write_reg(REG_FIFO_WR_PTR, 0x00);
        maxim_max30102_write_reg(REG_FIFO_RD_PTR, 0x00);
        maxim_max30102_write_reg(REG_OVF_COUNTER, 0x00);
        hr_state = HR_STATE_DONE;
        // fall through to DONE

    case HR_STATE_DONE:
        gTaskStateBit.Max30102 = 1;
        hr_state = HR_STATE_FINGER_CHECK;
        g_hr_continue_flag = 0;
        break;
    }
}
#endif