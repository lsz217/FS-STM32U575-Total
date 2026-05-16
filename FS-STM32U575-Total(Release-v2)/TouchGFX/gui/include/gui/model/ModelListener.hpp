#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>
//#if defined LINK_HARDWARE 	//TuchGFX仿真与实际硬件操作隔离
//	#include "stdint.h"
//#endif
extern "C" {
#include "stdint.h"
}
class ModelListener
{
public:
    ModelListener() : model(0) {}

    virtual ~ModelListener() {}

    void bind(Model* m)
    {
        model = m;
    }
			 virtual void nfcTriggered() {};
			virtual void toggleShape(){};
			virtual void ChangeScreen(){};
			virtual void ChangeScreen1(){};
			virtual void notifySleep() {};

			virtual void onUnlockRequest() {} // 这是一个接口，意思是"请求解锁"
			//更新日期和时间
			virtual void updateDate(uint8_t Year, uint8_t Month, uint8_t Date, uint8_t WeekDay) {}
			virtual void updateTime(uint8_t Hours, uint8_t Minutes, uint8_t Seconds) {}
			virtual void updateFiveKey(uint16_t FiveKeyVal) {}
			virtual void updateSixAxis(float pitchVal,float rollVal,float yawVal) {}
			//获取WiFi模组的RSSI值
			virtual void updateWiFiRSSI(uint8_t (&pWiFiInfo)[40], uint16_t newRSSI) {}
			//更新芯片内部参数
			virtual void updateChipInfor(uint16_t ChipTempVal, uint16_t VrefVal, uint16_t VbatVal) {}
			//健康监测信息上传
			virtual void updateHeartRateInfo(uint32_t newHeartRate, uint32_t newSPO2) {}
			//更新火焰状态
			virtual void updateFireStatus(bool newStatus) {}
			//更新电压、电流、温湿度、光照度数据上传
			virtual void updateAppPageInfo(uint16_t CurrentVal, uint16_t VoltageVal, float newHum, float newTem, uint16_t newALS) {}
			//更新传感器数据到SensorPage
			virtual void updateSensorInfo(float temperature, float humidity, uint16_t co2, uint32_t heartRate, uint32_t spo2) {}
	protected:
	    Model* model;
};

#endif // MODELLISTENER_HPP
