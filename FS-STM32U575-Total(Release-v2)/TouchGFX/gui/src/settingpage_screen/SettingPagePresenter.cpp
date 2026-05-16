#include <gui/settingpage_screen/SettingPageView.hpp>
#include <gui/settingpage_screen/SettingPagePresenter.hpp>

SettingPagePresenter::SettingPagePresenter(SettingPageView& v)
    : view(v)
{

}

void SettingPagePresenter::activate()
{
		SettingPagePresenterState(true);	
}

void SettingPagePresenter::deactivate()
{
		SettingPagePresenterState(false);	
}

//更新背光亮度
void SettingPagePresenter::setBacklightValue(int value)
{
    model->setBacklightValue(value);
}
//更新芯片信息
void SettingPagePresenter::updateChipInfor(uint16_t newChipTempVal, uint16_t newVrefVal, uint16_t newVbatVal)
{
    view.updateChipInfor(newChipTempVal, newVrefVal, newVbatVal);
}
//获取背光亮度
uint8_t SettingPagePresenter::getBacklightValue()
{
    return(model->getBacklightValue());
}
//SettingPagePresenter状态
void SettingPagePresenter::SettingPagePresenterState(bool enable)
{
	if(enable == true)
		model->SettingPageViewTask(true);
	else
		model->SettingPageViewTask(false);
}
