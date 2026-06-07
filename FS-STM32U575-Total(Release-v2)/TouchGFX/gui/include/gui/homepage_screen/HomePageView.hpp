#ifndef HOMEPAGEVIEW_HPP
#define HOMEPAGEVIEW_HPP

#include <gui_generated/homepage_screen/HomePageViewBase.hpp>
#include <gui/homepage_screen/HomePagePresenter.hpp>
#include <touchgfx/widgets/Box.hpp>

class HomePageView : public HomePageViewBase
{
public:
    HomePageView();
    virtual ~HomePageView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    void handleTickEvent();
    virtual void updateDate(uint8_t newYear, uint8_t newMonth, uint8_t newDate, uint8_t newWeekDay);
    virtual void updateTime(uint8_t newHours, uint8_t newMinutes, uint8_t newSeconds);
    virtual void updateWiFiRSSI(uint8_t (&pWiFiInfo)[40], uint16_t newRSSI);
    void TextAreaAddStr(uint8_t* str, uint32_t len, uint16_t newRSSI);
    void ChangeScreen();
    virtual void connectWiFi();
    void expandButtonCallbackHandler(const touchgfx::AbstractButton& src);
    void addTempPoint(float temperature);
    void addHRPoint(int heartRate);
protected:
    uint8_t textBuf[200];
    int lastPage;
    int16_t tickCounter;
    float pendingTemp;
    int   pendingHR;
    bool  tempUpdated;
    bool  hrUpdated;
private:
    touchgfx::Callback<HomePageView, const touchgfx::AbstractButton&> expandButtonCallback;

    // 折线图背景 + 坐标轴边框（纯色 Box，不涉及文本渲染）
    touchgfx::Box graphBg1;
    touchgfx::Box graphBg2;
    touchgfx::Box graphBorderBtm1;
    touchgfx::Box graphBorderBtm2;
    touchgfx::Box graphBorderLeft1;
    touchgfx::Box graphBorderLeft2;
    touchgfx::Box graphBorderTop1;
    touchgfx::Box graphBorderTop2;
};

#endif // HOMEPAGEVIEW_HPP
