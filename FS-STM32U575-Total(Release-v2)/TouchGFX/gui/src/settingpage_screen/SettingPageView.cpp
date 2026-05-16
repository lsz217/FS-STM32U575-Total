#include <gui/settingpage_screen/SettingPageView.hpp>

SettingPageView::SettingPageView()
{

}

void SettingPageView::setupScreen()
{
		readBacklightVal =	presenter->getBacklightValue();	//读取亮度
		sliderScreenBacklight.setValue(readBacklightVal);	//设置滑块位置
		updateBacklightText(readBacklightVal);	//显示亮度值
		//
		SettingPageViewBase::setupScreen();
}

void SettingPageView::tearDownScreen()
{
    SettingPageViewBase::tearDownScreen();
}
//更新Chip信息
void SettingPageView::updateChipInfor(uint16_t newChipTempVal, uint16_t newVrefVal, uint16_t newVbatVal)
{
		//更新芯片内部温度
		Unicode::snprintf(textChipTempBuffer, TEXTCHIPTEMP_SIZE, "%02d",newChipTempVal);
	  textChipTemp.invalidate();
		//更新芯片参考电压
		Unicode::snprintf(textChipVrefBuffer1, TEXTCHIPVREFBUFFER1_SIZE, "%d",((newVrefVal)*3300/4095)/1000);
		Unicode::snprintf(textChipVrefBuffer2, TEXTCHIPVREFBUFFER2_SIZE, "%02d", ((newVrefVal)*3300/4095)%1000);
	  textChipVref.invalidate();
		//更新VBAT电压
		Unicode::snprintf(textChipVbatBuffer1, TEXTCHIPVBATBUFFER1_SIZE, "%d",((newVbatVal)*4*3300/4095)/1000);
		Unicode::snprintf(textChipVbatBuffer2, TEXTCHIPVBATBUFFER2_SIZE, "%02d", ((newVbatVal)*4*3300/4095)%1000);
	  textChipVbat.invalidate();
}
//背光亮度滑动条
void SettingPageView::updateBacklightText(int value)
{
    Unicode::snprintf(textBackLightBuffer, TEXTBACKLIGHT_SIZE, "%d", value);
    textBackLight.resizeToCurrentText();
    container.invalidate();
}

void SettingPageView::newBacklightValueHandler(int value)
{
    presenter->setBacklightValue(value);
    updateBacklightText(value);
}