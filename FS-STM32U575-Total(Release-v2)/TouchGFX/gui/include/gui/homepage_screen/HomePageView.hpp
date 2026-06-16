#ifndef HOMEPAGEVIEW_HPP
#define HOMEPAGEVIEW_HPP

#include <gui_generated/homepage_screen/HomePageViewBase.hpp>
#include <gui/homepage_screen/HomePagePresenter.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>

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
    void updateSensorTexts(float temperature, float humidity, uint16_t co2,
                           uint32_t heartRate, uint32_t spo2);
    void updateALS(uint16_t als);
    void updatePowerInfo(uint16_t currentVal, uint16_t voltageVal, uint8_t sleepRatio);
    void switchToPowerPage();
    void switchToHomePage();
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

    // 功耗监控显示 (swipe page 2)
    touchgfx::Box powerBg;
    touchgfx::TextAreaWithOneWildcard textPowerCurrent;
    touchgfx::TextAreaWithOneWildcard textPowerVoltage;
    touchgfx::TextAreaWithOneWildcard textPowerCPU;

    static const uint16_t TEXT_POWER_CURRENT_SIZE = 30;
    touchgfx::Unicode::UnicodeChar textPowerCurrentBuffer[TEXT_POWER_CURRENT_SIZE];
    static const uint16_t TEXT_POWER_VOLTAGE_SIZE = 30;
    touchgfx::Unicode::UnicodeChar textPowerVoltageBuffer[TEXT_POWER_VOLTAGE_SIZE];
    static const uint16_t TEXT_POWER_CPU_SIZE = 30;
    touchgfx::Unicode::UnicodeChar textPowerCPUBuffer[TEXT_POWER_CPU_SIZE];
};

#endif // HOMEPAGEVIEW_HPP
