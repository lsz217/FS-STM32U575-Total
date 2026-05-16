#ifndef APPLICATIONPAGEPRESENTER_HPP
#define APPLICATIONPAGEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class ApplicationPageView;

class ApplicationPagePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    ApplicationPagePresenter(ApplicationPageView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();
		void setDCFanStatus(bool state);
		//控制设备翻转状态
    virtual void turnMotorStatus();
    virtual ~ApplicationPagePresenter() {};
		//ApplicationPagePresenter状态
		void ApplicationPagePresenterState(bool enable);
		//数码管显示任务使能
		virtual void NixieTubeTaskState(bool newStatus);
		//数码管显示值设置
		virtual void setNixieTube(uint16_t showValue);
		//健康监测任务使能
		virtual void HeartRateTaskState(bool newStatus);
		//更新火焰状态
		virtual void updateFireStatus(bool newStatus);
		//健康监测信息上传
		virtual void updateHeartRateInfo(uint32_t newHeartRate, uint32_t newSPO2);
		//更新电压、电流、温湿度、光照度数据上传	
		virtual void updateAppPageInfo(uint16_t CurrentVal, uint16_t VoltageVal, float newHum, float newTem, uint16_t newALS);
private:
    ApplicationPagePresenter();

    ApplicationPageView& view;
};

#endif // APPLICATIONPAGEPRESENTER_HPP
