#ifndef SETTINGPAGEPRESENTER_HPP
#define SETTINGPAGEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class SettingPageView;

class SettingPagePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    SettingPagePresenter(SettingPageView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate()override;

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate()override;

    virtual ~SettingPagePresenter() {};
		//背光亮度
    void setBacklightValue(int value);
    uint8_t	getBacklightValue();	
		//更新芯片内部参数
		virtual void updateChipInfor(uint16_t newChipTempVal, uint16_t newVrefVal, uint16_t newVbatVal) override;
		//SettingPagePresenter的状态
		void SettingPagePresenterState(bool enable);

private:
    SettingPagePresenter();

    SettingPageView& view;
};

#endif // SETTINGPAGEPRESENTER_HPP
