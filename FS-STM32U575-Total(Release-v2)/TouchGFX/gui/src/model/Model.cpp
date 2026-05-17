#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#if defined LINK_HARDWARE 	//TuchGFX仿真与实际硬件操作隔离
	//头文件包含
	extern "C"
	{
	#include "user_app.h"
	#include "bsp_motor.h"
		extern TIM_HandleTypeDef htim2;
		extern volatile uint8_t gBacklightVal;
	extern volatile uint8_t g_nfc_unlock_flag; // 声明来自 main.c 的全局标志位
	}
		extern RTC_DateTypeDef gSystemDate;  //获取日期结构体
		extern RTC_TimeTypeDef gSystemTime;   //获取时间结构体
		extern gTask_MarkEN gTaskEnMark;  //系统任务使能标识
		extern volatile uint8_t gLastTimeSeconds;	//上一次的时间
		extern volatile float pitch,roll,yaw; //欧拉角
		extern volatile SHT20_TemRH_Val gTemRH_Val;
		extern volatile StruAP3216C_Val gAP3216C_Val;	//AP3216数据结构
		extern gTask_BitDef gTaskStateBit;  //任务执行过程中使用到的标志位
		//
		extern wifiRSSI ao_wifiRSSI;
		extern volatile uint16_t gFiveKeyVal; //五向键原始值：0V/0.5V/1.0V/1.5V/2.0V
		extern volatile uint16_t gCurrentVal;	//资源扩展板电流，通道IN8
		extern volatile uint16_t gVoltageVal;	//资源扩展板电压，通道IN9
		extern volatile uint16_t gChipTempVal;//内部参考电压，通道IN12
		extern volatile uint16_t gVrefVal;		//内部参考电压，通道IN13
		extern volatile uint16_t gVbatVal;		//RTC电池电压，通道IN14
		extern volatile uint16_t gNixieShowData;	//数码管显示的数据
		//
		volatile uint8_t gLastTimeSeconds = 0;	//上一次的时间
		volatile uint16_t gLastFiveKey = 0;	//上一次的按键值
		volatile uint8_t gLastChipTemp = 0;	//上一次的温度值
		//
		extern uint8_t gWiFiInfo[40];	//用于通知View界面的Text文本显示
		extern volatile uint8_t gBacklightVal;	//背光值，默认50%
		extern int32_t n_heart_rate;   //heart rate value=n_heart_rate/4,采样率100sps,max30102设置4点求平均
		extern int32_t n_sp02; //SPO2 value
		extern int8_t ch_spo2_valid;   //indicator to show if the SP02 calculation is valid
		extern int8_t  ch_hr_valid;    //indicator to show if the heart rate calculation is valid
		extern int8_t KeyChangeScreen;
	#else //Designer仿真
		#include <ctime>
		#ifndef _MSC_VER

		#endif /* _MSC_VER*/
		//

	#endif

/**********************************TouchGFX与底层间的访问**********************************/
Model::Model() : modelListener(0), idleTimer(0), isSleeping(false), hapticCounter(0)
{
#if defined LINK_HARDWARE
	printf("[Model] Constructor OK\r\n");
#endif
}
//
void Model::tick()
{
	static uint8_t tickCount = 0;	//减少数据上传的次数，优化界面刷新
	tickCount++;
#if defined LINK_HARDWARE
		if (tickCount == 1)
			printf("[Model] First tick OK\r\n");
		// 马达震动计时：每 tick 减少计数，到 0 时关闭马达
		if (hapticCounter > 0)
		{
			hapticCounter--;
			if (hapticCounter == 0)
			{
				BSP_MOTOR_Off();
			}
		}
		//更新日期与时间
		if(gTaskEnMark.UPDATE_TIME_EN && (gSystemTime.Seconds != gLastTimeSeconds))	//每秒同步一次界面时间
		{
			modelListener->updateDate(gSystemDate.Year,gSystemDate.Month,gSystemDate.Date,gSystemDate.WeekDay);
			modelListener->updateTime(gSystemTime.Hours, gSystemTime.Minutes, gSystemTime.Seconds);
			//更新新值
			gLastTimeSeconds = gSystemTime.Seconds;
		}
		//更新芯片参数
		if(gTaskEnMark.UPDATE_CHIPINFO_EN && (abs(gChipTempVal-gLastChipTemp) >= 1))	//温度变化超过1度
		{
			modelListener->updateChipInfor(gChipTempVal, gVrefVal, gVbatVal);	//更新芯片温度、参考电压、Vbat
			//更新新值
			gLastChipTemp=gChipTempVal;
		}
		//更新欧拉角
		if(gTaskEnMark.UPDATE_SIX_AXIS_EN)	//六轴界面活动时上传
		{
			modelListener->updateSixAxis(pitch, roll, yaw);
		}
		//获取WiFi模组的RSSI值
		if((gTaskEnMark.UPDATE_WIFI_RSSI_EN) && (gTaskEnMark.UPDATE_TIME_EN))	//只有在系统主页时，才进行WiFi的RSSI数据读取
		{
			modelListener->updateWiFiRSSI(gWiFiInfo, ao_wifiRSSI.gRSSI);
		}
		//更新五向键数据
		if(gTaskEnMark.UPDATE_FIVEKEY_EN && (abs(gFiveKeyVal-gLastFiveKey) >= 12))	// 3.3*12/4095 = 11.7mV
		{
			modelListener->updateFiveKey(gFiveKeyVal);
			//更新新值
			gLastFiveKey = gFiveKeyVal;
		}
		//APP页面的数据更新使能
		if(gTaskEnMark.UPDATE_APP_TASK_EN)
		{
			if(gTaskStateBit.ExtFIRE)//检测到火焰与使能火焰状态更新
				modelListener->updateFireStatus(true);	//更新火焰状态
			else
				modelListener->updateFireStatus(false);	//更新火焰状态
			//电压、电流、温湿度、光照度数据上传
			if(!(tickCount % 5))	//降低界面刷新负担
				modelListener->updateAppPageInfo(gCurrentVal, gVoltageVal, gTemRH_Val.Hum, gTemRH_Val.Tem, gAP3216C_Val.ALS);
			//SensorPage传感器数据转发
			modelListener->updateSensorInfo(gTemRH_Val.Tem, gTemRH_Val.Hum, gSCD41_Val.CO2,
					ch_hr_valid   ? (uint32_t)(n_heart_rate / 4) : 0xFFFFFFFF,
					ch_spo2_valid ? (uint32_t)n_sp02             : 0xFFFFFFFF);
		}
		//健康监测信息上传
		if(gTaskEnMark.UPDATE_HEART_RATE_EN)
		{
			//send samples and calculation result to terminal program through UART
			if(ch_hr_valid || ch_spo2_valid)
			{
				modelListener->updateHeartRateInfo(n_heart_rate/4, n_sp02);
			}
			//
			if(gTaskStateBit.Max30102)	//单次测量完成，清除标志
			{
				ch_hr_valid =0;
				ch_spo2_valid=0;
				gTaskStateBit.Max30102 = 0;
				gTaskEnMark.UPDATE_HEART_RATE_EN = 0;
			}
		}

	#endif
//	    if (!isSleeping)
//    {
//        idleTimer++;
//        // 1800 帧大约是 30 秒 (60fps * 30s)
//        if (idleTimer >= 1800)
//        {
//            isSleeping = true;
//            if (modelListener != 0)
//            {
//                modelListener->notifySleep(); // 触发息屏通知
//            }
//        }
//    }



	    if (g_nfc_unlock_flag == 1)
	    {
	        g_nfc_unlock_flag = 0; // 记得清空标志位，防止重复触发

	        // 通知通知 Presenter（也就是对应的页面）
	        if (modelListener != 0)
	        {
	            modelListener->nfcTriggered(); // 这里的函数名要和 ModelListener.hpp 里对应
	        }
	   }
			}
	//直流风扇操作
	void Model::setDCFanStatus(bool state)
	{
		#if defined LINK_HARDWARE
		if(state == true)
			HAL_GPIO_WritePin(FAN_GPIO_Port,FAN_Pin,GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(FAN_GPIO_Port,FAN_Pin,GPIO_PIN_RESET);
	#endif
	}

	//ApplicationPageView界面的FIRE任务的状态
	void Model::ApplicationPageViewFireTask(bool enable)
	{
		#if defined LINK_HARDWARE
		if(enable == true)
			gTaskEnMark.UPDATE_APP_TASK_EN = 1;	//任务使能
		else
			gTaskEnMark.UPDATE_APP_TASK_EN = 0;	//任务清除
	#endif
	}
	//HomePageView的任务的状态
	void Model::HomePageViewTask(bool enable)
	{
		#if defined LINK_HARDWARE
		if(enable == true)
			gTaskEnMark.UPDATE_TIME_EN = 1;	//任务使能
		else
			gTaskEnMark.UPDATE_TIME_EN = 0;	//任务清除
	#endif
	}
	//FiveKeyPageView的任务的状态
	void Model::FiveKeyPageViewTask(bool enable)
	{
		#if defined LINK_HARDWARE
		if(enable == true)
			gTaskEnMark.UPDATE_FIVEKEY_EN = 1;	//任务使能
		else
			gTaskEnMark.UPDATE_FIVEKEY_EN = 0;	//任务清除
	#endif
	}
	//设置健康监测任务
	void Model::HeartRateTaskEnable(bool newStatus)
	{
		#if defined LINK_HARDWARE
		if(newStatus == true)
			gTaskEnMark.UPDATE_HEART_RATE_EN = 1;	//任务使能
		else
			gTaskEnMark.UPDATE_HEART_RATE_EN = 0;	//任务清除
	#endif
	}
	//SettingPageView的任务的状态
	void Model::SettingPageViewTask(bool enable)
	{
	#if defined LINK_HARDWARE
		if(enable == true)
			gTaskEnMark.UPDATE_CHIPINFO_EN = 1;	//任务使能
		else
			gTaskEnMark.UPDATE_CHIPINFO_EN = 0;	//任务清除
	#endif
	}
	//SixAxisPageView的任务的状态
	void Model::SixAxisPageViewTask(bool enable)
	{
	#if defined LINK_HARDWARE
		if(enable == true)
			gTaskEnMark.UPDATE_SIX_AXIS_EN = 1;	//任务使能
		else
			gTaskEnMark.UPDATE_SIX_AXIS_EN = 0;	//任务清除
	#endif
	}
	//数码管显示任务使能
	void Model::NixieTubeTaskEnable(bool newStatus)
	{
	#if defined LINK_HARDWARE
		if(newStatus == true)
			gTaskEnMark.UPDATE_NIXIE_SHOW_EN = 1;	//任务使能
		else
			gTaskEnMark.UPDATE_NIXIE_SHOW_EN = 0;	//任务清除
	#endif
	}
	//数码管显示任务使能
	void Model::setNixieTubeValue(uint16_t showValue)
	{
	#if defined LINK_HARDWARE
		gNixieShowData = showValue;	//更新数码管显示值
	#endif
	}
	//SensorPageView任务使能
	void Model::SensorPageViewTask(bool enable)
	{
	#if defined LINK_HARDWARE
		if(enable == true)
			gTaskEnMark.UPDATE_APP_TASK_EN = 1;	//任务使能
		else
			gTaskEnMark.UPDATE_APP_TASK_EN = 0;	//任务清除
	#endif
	}
	//HomePageView界面的WiFiModalLink的任务的状态
	void Model::HomePageViewWiFiModalLinkTask(bool enable)
	{
	#if defined LINK_HARDWARE
		if(enable == true)
			gTaskEnMark.UPDATE_WIFI_RSSI_EN = 1;	//任务使能
		else
			gTaskEnMark.UPDATE_WIFI_RSSI_EN = 0;	//任务清除
	#endif
	}
	//设置背光
	void Model::setBacklightValue(uint8_t value)
	{
	#if defined LINK_HARDWARE
	    if (value != gBacklightVal)
	    {
	        gBacklightVal = value;

	        // 如果值为 0，实现彻底息屏
	        if (gBacklightVal == 0)
	        {
	            // 强制将 TIM2 通道 3 的比较值设为 0，即 0% 占空比
	            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
	        }
	        else
	        {
	            // 将 0-100 的输入映射到 0-999 的定时器周期
	            // 公式：Pulse = (value * Period) / 100
	            uint32_t pulse = (uint32_t)gBacklightVal * 999 / 100;

	            // 确保定时器 PWM 处于开启状态
	            HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);

	            // 设置新的亮度
	            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulse);
	        }

	        // 如果你担心原有的 Update_Backlight 还有其他初始化操作（如开启电荷泵），
	        // 也可以保留这一行，但上面的 __HAL_TIM_SET_COMPARE 会覆盖它的亮度设置。
	        // Update_Backlight(gBacklightVal);
	    }
	#else
	    if (value != gBacklightVal)
	    {
	        gBacklightVal = value;
	    }
	#endif
	}
	//获取背光亮度
	uint8_t Model::getBacklightValue()
	{
	#if defined LINK_HARDWARE
	    return gBacklightVal;
	#else
			return gBacklightVal;	//仿真
	#endif
	}
	//振动电机操作
	void Model::setMotorStatus()
	{
	#if defined LINK_HARDWARE
			HAL_GPIO_TogglePin(EXT_MOTOR_GPIO_Port,EXT_MOTOR_Pin);	//振动电机状态的翻转
	#endif
	}

	//触摸震动：短暂马达脉冲（约50ms，非阻塞）
	void Model::triggerHapticPulse()
	{
	#if defined LINK_HARDWARE
		BSP_MOTOR_On();
		hapticCounter = 3; // 3 ticks ≈ 50ms
	#endif
	}

	//马达持续开/关控制
	void Model::setMotorOn(bool on)
	{
	#if defined LINK_HARDWARE
		if (on)
			BSP_MOTOR_On();
		else
			BSP_MOTOR_Off();
	#endif
	}

	void Model::resetTimer()
	{
	    idleTimer = 0;
	    isSleeping = false;
	}
