#include <gui/applicationpage_screen/ApplicationPageView.hpp>
#include <gui/applicationpage_screen/ApplicationPagePresenter.hpp>

ApplicationPagePresenter::ApplicationPagePresenter(ApplicationPageView& v)
    : view(v)
{

}

void ApplicationPagePresenter::activate()
{
		ApplicationPagePresenterState(true);	
}

void ApplicationPagePresenter::deactivate()
{
		ApplicationPagePresenterState(false);	
}

void ApplicationPagePresenter::setDCFanStatus(bool state)
{
	model->setDCFanStatus(state);
}
//更新火焰状态
void ApplicationPagePresenter::updateFireStatus(bool newStatus)
{
		view.updateFireStatus(newStatus);	
}
//振动电机操作
void ApplicationPagePresenter::turnMotorStatus()
{
		model->setMotorStatus();	//振动电机状态的翻转	
}
//数码管显示任务使能
void ApplicationPagePresenter::NixieTubeTaskState(bool newStatus)
{
		if(newStatus == true)
			model->NixieTubeTaskEnable(true);
		else
			model->NixieTubeTaskEnable(false);
}
//数码管显示值设置
void ApplicationPagePresenter::setNixieTube(uint16_t showValue)
{
		model->setNixieTubeValue(showValue);
}
//ApplicationPagePresenter状态
void ApplicationPagePresenter::ApplicationPagePresenterState(bool enable)
{
		if(enable == true)
			model->ApplicationPageViewFireTask(true);
		else
			model->ApplicationPageViewFireTask(false);
}
//更新火焰状态
void ApplicationPagePresenter::updateAppPageInfo(uint16_t CurrentVal, uint16_t VoltageVal, float newHum, float newTem, uint16_t newALS)
{
		view.updateAppPageInfo(CurrentVal, VoltageVal, newHum, newTem, newALS);	
}
//设置健康监测任务
void ApplicationPagePresenter::HeartRateTaskState(bool newStatus)
{
		if(newStatus == true)
			model->HeartRateTaskEnable(true);
		else
			model->HeartRateTaskEnable(false);
}
//健康监测信息上传
void ApplicationPagePresenter::updateHeartRateInfo(uint32_t newHeartRate, uint32_t newSPO2)
{
		view.updateHeartRateInfo(newHeartRate, newSPO2);
}