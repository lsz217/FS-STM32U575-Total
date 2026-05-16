#include <gui/homepage_screen/HomePageView.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include "BitmapDatabase.hpp"
//标准库
#include <stdio.h>
#include <string.h>
//#include "stdlib.h"	//abs函数
//
#if defined LINK_HARDWARE
extern uint8_t KeyChangeScreen;
#endif

HomePageView::HomePageView()
{

}

void HomePageView::setupScreen()
{
    HomePageViewBase::setupScreen();
}

void HomePageView::tearDownScreen()
{
    HomePageViewBase::tearDownScreen();
}

void HomePageView::handleTickEvent()
{
	static int timCnt=0;
	timCnt++;
	if(timCnt>=2)
	{
		timCnt=0;
		tiledImage1.setOffset(0,tiledImage1.getYOffset()+1);
		tiledImage1.invalidate();
	}
}

void HomePageView::updateTime(uint8_t newHours, uint8_t newMinutes, uint8_t newSeconds)
{

		Unicode::snprintf(textSystemClockBuffer1, TEXTSYSTEMCLOCKBUFFER1_SIZE, "%02d", newHours);
		Unicode::snprintf(textSystemClockBuffer2, TEXTSYSTEMCLOCKBUFFER2_SIZE, "%02d", newMinutes);
	  textSystemClock.invalidate();
		
		Unicode::snprintf(textClockSecondBuffer, TEXTCLOCKSECOND_SIZE, "%02d", newSeconds);
    textClockSecond.invalidate();	
}
//更新日期
void HomePageView::updateDate(uint8_t newYear, uint8_t newMonth, uint8_t newDate, uint8_t newWeekDay)
{
		Unicode::snprintf(textSystemYearBuffer, TEXTSYSTEMYEAR_SIZE, "%04d", newYear + 2000);
    textSystemYear.invalidate();	
		//
		Unicode::snprintf(textSystemDateBuffer1, TEXTSYSTEMDATEBUFFER1_SIZE, "%02d", newMonth);
		Unicode::snprintf(textSystemDateBuffer2, TEXTSYSTEMDATEBUFFER2_SIZE, "%02d", newDate);
	  textSystemDate.invalidate();
		//
		Unicode::snprintf(textWeekDayBuffer, TEXTWEEKDAY_SIZE, "%d", newWeekDay);
    textWeekDay.invalidate();
		//
}
//
void HomePageView::connectWiFi()
{
		presenter->HomePageViewWiFiModalLinkTask(true);	//启用WiFi读取RSSI任务
}
//
void HomePageView::updateWiFiRSSI(uint8_t (&pWiFiInfo)[40], uint16_t newRSSI)
{
    this->TextAreaAddStr(pWiFiInfo, sizeof(pWiFiInfo), newRSSI);
}
//
void HomePageView::TextAreaAddStr(uint8_t* str, uint32_t len, uint16_t newRSSI)
{
    int16_t textHeight = 0, nowTextHeight = 0;
    nowTextHeight = textWiFiLinkInfo.getTextHeight();
    textHeight = textWiFiLinkInfo.getHeight();
		//文本显示满，清除
    if (nowTextHeight > textHeight)
    {
        memset(textBuf, 0, 10);
    }
    uint32_t lens = strlen((char*)textBuf);
    memcpy((char*)textBuf + lens, (char*)str, len);
    Unicode::fromUTF8(textBuf, textWiFiLinkInfoBuffer, lens + len);
    textWiFiLinkInfo.setWideTextAction(WIDE_TEXT_CHARWRAP);
    textWiFiLinkInfo.invalidate();
		//根据RSSI值更新图标
		if(newRSSI <= 40){
			WirelessConnection.setBitmaps(touchgfx::Bitmap(BITMAP_WIFI_RSSI_4_SIGNAL_ID),touchgfx::Bitmap(BITMAP_WIFILINK_PRESSED_54X54_ID));	
		} 
		else if(newRSSI > 40 && newRSSI <= 50){
			WirelessConnection.setBitmaps(touchgfx::Bitmap(BITMAP_WIFI_RSSI_3_SIGNAL_ID),touchgfx::Bitmap(BITMAP_WIFILINK_PRESSED_54X54_ID));	
		}
		else if(newRSSI > 50 && newRSSI <= 60){
			WirelessConnection.setBitmaps(touchgfx::Bitmap(BITMAP_WIFI_RSSI_2_SIGNAL_ID),touchgfx::Bitmap(BITMAP_WIFILINK_PRESSED_54X54_ID));	
		}
		else if(newRSSI > 70 && newRSSI <= 80){
			WirelessConnection.setBitmaps(touchgfx::Bitmap(BITMAP_WIFI_RSSI_1_SIGNAL_ID),touchgfx::Bitmap(BITMAP_WIFILINK_PRESSED_54X54_ID));	
		}
		else if(newRSSI > 80){
			WirelessConnection.setBitmaps(touchgfx::Bitmap(BITMAP_WIFI_RSSI_NO_SIGNAL_ID),touchgfx::Bitmap(BITMAP_WIFILINK_PRESSED_54X54_ID));	
		}
		WirelessConnection.invalidate();	//显示
}
void HomePageView::ChangeScreen()
{
		#if defined LINK_HARDWARE
		if(KeyChangeScreen==1){handleKeyEvent(1);}
		#endif
}