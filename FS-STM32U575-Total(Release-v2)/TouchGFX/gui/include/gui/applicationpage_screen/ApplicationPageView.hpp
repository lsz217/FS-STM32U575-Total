#ifndef APPLICATIONPAGEVIEW_HPP
#define APPLICATIONPAGEVIEW_HPP

#include <gui_generated/applicationpage_screen/ApplicationPageViewBase.hpp>
#include <gui/applicationpage_screen/ApplicationPagePresenter.hpp>

class ApplicationPageView : public ApplicationPageViewBase
{
public:
    ApplicationPageView();
    virtual ~ApplicationPageView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
		virtual void Fanswitch();
		virtual void subNixieTubeVal();	//数值加
		virtual void addNixieTubeVal();	//数值减
		virtual void selectedNixieTubeUnit();	//选中个位
		virtual void selectedNixieTubeTen();	//选中十位
		virtual void selectedNixieTubeHundred();	//选中百位
		virtual void selectedNixieTubeThousand();	//选中千位	
		//进入数码管设置界面
		virtual void openModalNixieTube();
		//退出数码管设置界面
		virtual void closedModalNixieTube();
		//振动电机操作
		virtual void turnMotor();	//翻转振动电机状态
		//健康监测信息上传
		virtual void updateHeartRateInfo(uint32_t newHeartRate, uint32_t newSPO2);
		//更新火焰状态
		virtual void updateFireStatus(bool newStatus);
		//启动健康检测
		virtual void beginHeartRate();	//开始测量脉搏与血氧饱和度
		virtual void openModalHeartRate();	//打开健康监测页面
		virtual void closedModalHeartRate();	//关闭健康监测页面
		//更新电压、电流、温湿度、光照度数据上传	
		virtual void updateAppPageInfo(uint16_t CurrentVal, uint16_t VoltageVal, float newHum, float newTem, uint16_t newALS);
protected:
		uint16_t gshowValue;	//数码管显示的值0000(千位)0000(百位)0000(十位)0000(个位)
		uint8_t gshiftCount;	//移位值
		void NixieTubeDisplay(uint16_t pshowValue,uint8_t pshiftCount);	//显示最新图片
};

#endif // APPLICATIONPAGEVIEW_HPP
