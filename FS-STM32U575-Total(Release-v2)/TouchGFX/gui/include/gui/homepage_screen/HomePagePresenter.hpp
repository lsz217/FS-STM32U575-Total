#ifndef HOMEPAGEPRESENTER_HPP
#define HOMEPAGEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class HomePageView;

class HomePagePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    HomePagePresenter(HomePageView& v);

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

    virtual ~HomePagePresenter() {};
    // 更新日期和时间
    virtual void updateDate(uint8_t newYear, uint8_t newMonth, uint8_t newDate, uint8_t newWeekDay);
    virtual void updateTime(uint8_t newHours, uint8_t newMinutes, uint8_t newSeconds);
    // HomePagePresenter 的状态
    void HomePagePresenterState(bool enable);
    virtual void ChangeScreen();
    // 获取 WiFi 模组的 RSSI 值
    virtual void updateWiFiRSSI(uint8_t (&pWiFiInfo)[40], uint16_t newRSSI);
    // HomePageView 界面的 WiFiModalLink 的任务的状态
    void HomePageViewWiFiModalLinkTask(bool enable);
    // 背光控制
    void setBacklightValue(uint8_t value);

private:
    HomePagePresenter();

    HomePageView& view;
};

#endif // HOMEPAGEPRESENTER_HPP
