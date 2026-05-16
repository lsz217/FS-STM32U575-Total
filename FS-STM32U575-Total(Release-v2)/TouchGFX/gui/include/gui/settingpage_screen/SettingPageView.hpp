#ifndef SETTINGPAGEVIEW_HPP
#define SETTINGPAGEVIEW_HPP

#include <gui_generated/settingpage_screen/SettingPageViewBase.hpp>
#include <gui/settingpage_screen/SettingPagePresenter.hpp>

class SettingPageView : public SettingPageViewBase
{
public:
    SettingPageView();
    virtual ~SettingPageView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
		virtual void newBacklightValueHandler(int value);
		//更新芯片内部参数
		virtual void updateChipInfor(uint16_t newChipTempVal, uint16_t newVrefVal, uint16_t newVbatVal);
		uint8_t readBacklightVal;	//亮度值
		void updateBacklightText(int value);

protected:
};

#endif // SETTINGPAGEVIEW_HPP
