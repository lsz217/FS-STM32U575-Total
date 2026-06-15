#include <gui/homepage_screen/HomePageView.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include "BitmapDatabase.hpp"
#include <touchgfx/Color.hpp>
#include <stdio.h>
#include <string.h>

#if defined LINK_HARDWARE
extern uint8_t KeyChangeScreen;
#endif

HomePageView::HomePageView()
    : lastPage(0), tickCounter(0), pendingTemp(0), pendingHR(0),
      tempUpdated(false), hrUpdated(false)
{
}

void HomePageView::setupScreen()
{
    HomePageViewBase::setupScreen();

    expandButtonCallback = touchgfx::Callback<HomePageView, const touchgfx::AbstractButton&>(
        this, &HomePageView::expandButtonCallbackHandler);
    toggleButton4.setAction(expandButtonCallback);

    // 重排 Z 序：先移除 graph，再加背景/边框，最后加回 graph
    swipeContainer1Pe3.remove(dynamicGraph1);
    swipeContainer1Pe3.remove(dynamicGraph2);

    // ========== 温度折线图背景 + 边框 ==========
    graphBg1.setPosition(69, 0, 251, 90);
    graphBg1.setColor(touchgfx::Color::getColorFromRGB(15, 25, 80));
    graphBg1.setAlpha(102);
    swipeContainer1Pe3.add(graphBg1);

    graphBorderTop1.setPosition(69, 0, 251, 1);
    graphBorderTop1.setColor(touchgfx::Color::getColorFromRGB(60, 65, 75));
    swipeContainer1Pe3.add(graphBorderTop1);

    graphBorderLeft1.setPosition(69, 0, 2, 90);
    graphBorderLeft1.setColor(touchgfx::Color::getColorFromRGB(60, 65, 75));
    swipeContainer1Pe3.add(graphBorderLeft1);

    graphBorderBtm1.setPosition(69, 89, 251, 1);
    graphBorderBtm1.setColor(touchgfx::Color::getColorFromRGB(60, 65, 75));
    swipeContainer1Pe3.add(graphBorderBtm1);

    dynamicGraph1.clear();
    dynamicGraph1Line1Painter.setColor(touchgfx::Color::getColorFromRGB(220, 40, 40));
    dynamicGraph1.invalidate();
    swipeContainer1Pe3.add(dynamicGraph1);

    // ========== 心率折线图背景 + 边框 ==========
    graphBg2.setPosition(69, 147, 251, 91);
    graphBg2.setColor(touchgfx::Color::getColorFromRGB(15, 25, 80));
    graphBg2.setAlpha(102);
    swipeContainer1Pe3.add(graphBg2);

    graphBorderTop2.setPosition(69, 147, 251, 1);
    graphBorderTop2.setColor(touchgfx::Color::getColorFromRGB(60, 65, 75));
    swipeContainer1Pe3.add(graphBorderTop2);

    graphBorderLeft2.setPosition(69, 147, 2, 91);
    graphBorderLeft2.setColor(touchgfx::Color::getColorFromRGB(60, 65, 75));
    swipeContainer1Pe3.add(graphBorderLeft2);

    graphBorderBtm2.setPosition(69, 237, 251, 1);
    graphBorderBtm2.setColor(touchgfx::Color::getColorFromRGB(60, 65, 75));
    swipeContainer1Pe3.add(graphBorderBtm2);

    dynamicGraph2.clear();
    dynamicGraph2Line1Painter.setColor(touchgfx::Color::getColorFromRGB(40, 120, 220));
    dynamicGraph2.invalidate();
    swipeContainer1Pe3.add(dynamicGraph2);
}

void HomePageView::tearDownScreen()
{
    HomePageViewBase::tearDownScreen();
}

void HomePageView::handleTickEvent()
{
    int page = swipeContainer1.getSelectedPage();
    if (page != lastPage)
    {
        lastPage = page;
        bool show = (page == 0);
        toggleButton4.setVisible(show);
        toggleButton4.invalidate();
    }

    tickCounter++;
    if (tickCounter >= 60)
    {
        tickCounter = 0;
        if (tempUpdated) {
            dynamicGraph1.addDataPoint(pendingTemp);
            tempUpdated = false;
        }
        if (hrUpdated) {
            dynamicGraph2.addDataPoint(pendingHR);
            hrUpdated = false;
        }
    }
}

void HomePageView::addTempPoint(float temperature)
{
    pendingTemp = temperature;
    tempUpdated = true;
}

void HomePageView::addHRPoint(int heartRate)
{
    pendingHR = heartRate;
    hrUpdated = true;
}

void HomePageView::updateSensorTexts(float temperature, float humidity, uint16_t co2,
                                      uint32_t heartRate, uint32_t spo2)
{
    // TODO: Re-add TextArea widgets in TouchGFX Designer for sensor data display
    // textArea1-6 were removed during Designer cleanup.
    // Use the swipe container page 1 (swipeContainer1Page1) or page 2 (swipeContainer1Pe3)
    // to hold temperature/humidity/CO2/heart rate/SpO2 text areas.
    (void)temperature; (void)humidity; (void)co2; (void)heartRate; (void)spo2;
}

void HomePageView::updateALS(uint16_t als)
{
    // TODO: Re-add ALS text display widget
    (void)als;
}

void HomePageView::updateTime(uint8_t newHours, uint8_t newMinutes, uint8_t newSeconds)
{
    Unicode::snprintf(textSystemClockBuffer1, TEXTSYSTEMCLOCKBUFFER1_SIZE, "%02d", newHours);
    Unicode::snprintf(textSystemClockBuffer2, TEXTSYSTEMCLOCKBUFFER2_SIZE, "%02d", newMinutes);
    textSystemClock.invalidate();

    Unicode::snprintf(textClockSecondBuffer, TEXTCLOCKSECOND_SIZE, "%02d", newSeconds);
    textClockSecond.invalidate();
}

void HomePageView::updateDate(uint8_t newYear, uint8_t newMonth, uint8_t newDate, uint8_t newWeekDay)
{
    Unicode::snprintf(textSystemYearBuffer, TEXTSYSTEMYEAR_SIZE, "%04d", newYear + 2000);
    textSystemYear.invalidate();

    Unicode::snprintf(textSystemDateBuffer1, TEXTSYSTEMDATEBUFFER1_SIZE, "%02d", newMonth);
    Unicode::snprintf(textSystemDateBuffer2, TEXTSYSTEMDATEBUFFER2_SIZE, "%02d", newDate);
    textSystemDate.invalidate();

    Unicode::snprintf(textWeekDay_1Buffer, TEXTWEEKDAY_1_SIZE, "%d", newWeekDay);
    textWeekDay_1.invalidate();
}

void HomePageView::connectWiFi()
{
    presenter->HomePageViewWiFiModalLinkTask(true);
}

void HomePageView::updateWiFiRSSI(uint8_t (&pWiFiInfo)[40], uint16_t newRSSI)
{
    this->TextAreaAddStr(pWiFiInfo, sizeof(pWiFiInfo), newRSSI);
}

void HomePageView::TextAreaAddStr(uint8_t* str, uint32_t len, uint16_t newRSSI)
{
    int16_t textHeight = 0, nowTextHeight = 0;
    nowTextHeight = textWiFiLinkInfo.getTextHeight();
    textHeight = textWiFiLinkInfo.getHeight();

    if (nowTextHeight > textHeight)
    {
        memset(textBuf, 0, 10);
    }
    uint32_t lens = strlen((char*)textBuf);
    memcpy((char*)textBuf + lens, (char*)str, len);
    Unicode::fromUTF8(textBuf, textWiFiLinkInfoBuffer, lens + len);
    textWiFiLinkInfo.setWideTextAction(WIDE_TEXT_CHARWRAP);
    textWiFiLinkInfo.invalidate();

    // TODO: WiFi RSSI icon widget removed by Designer. Re-add a widget for signal display.
    (void)newRSSI;
}

void HomePageView::expandButtonCallbackHandler(const touchgfx::AbstractButton& src)
{
    application().gotoSensorPageScreenNoTransition();
}

void HomePageView::ChangeScreen()
{
#if defined LINK_HARDWARE
    if (KeyChangeScreen == 1) { handleKeyEvent(1); }
#endif
}
