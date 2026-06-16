#include <gui/homepage_screen/HomePageView.hpp>
#include <gui/homepage_screen/HomePagePresenter.hpp>

HomePagePresenter::HomePagePresenter(HomePageView& v)
    : view(v)
{

}

void HomePagePresenter::activate()
{
	HomePagePresenterState(true);
}

void HomePagePresenter::deactivate()
{
	HomePagePresenterState(false);
}



void HomePagePresenter::updateTime(uint8_t newHours, uint8_t newMinutes, uint8_t newSeconds)
{
    view.updateTime(newHours, newMinutes, newSeconds);
}

//HomePagePresenter状态
void HomePagePresenter::HomePagePresenterState(bool enable)
{
	if(enable == true)
		model->HomePageViewTask(true);
	else
		model->HomePageViewTask(false);
}
//HomePagePresenter状态
void HomePagePresenter::HomePageViewWiFiModalLinkTask(bool enable)
{
	if(enable == true)
		model->HomePageViewWiFiModalLinkTask(true);
	else
		model->HomePageViewWiFiModalLinkTask(false);
}

//更新日期
void HomePagePresenter::updateDate(uint8_t newYear, uint8_t newMonth, uint8_t newDate, uint8_t newWeekDay)
{
    view.updateDate(newYear, newMonth, newDate, newWeekDay);
}
//获取WiFi模组的RSSI值
void HomePagePresenter::updateWiFiRSSI(uint8_t (&pWiFiInfo)[40], uint16_t newRSSI)
{
    view.updateWiFiRSSI(pWiFiInfo, newRSSI);
}

void HomePagePresenter::updateSensorInfo(float temperature, float humidity, uint16_t co2, uint32_t heartRate, uint32_t spo2)
{
    view.addTempPoint(temperature);
    if (heartRate != 0xFFFFFFFF)
        view.addHRPoint((int)heartRate);
    view.updateSensorTexts(temperature, humidity, co2, heartRate, spo2);
}

void HomePagePresenter::updateAppPageInfo(uint16_t, uint16_t, float, float, uint16_t newALS)
{
    view.updateALS(newALS);
}

void HomePagePresenter::updatePowerInfo(uint16_t CurrentVal, uint16_t VoltageVal, uint8_t SleepRatio)
{
    view.updatePowerInfo(CurrentVal, VoltageVal, SleepRatio);
}

void HomePagePresenter::ChangeScreen()
{
    view.ChangeScreen();
}

void HomePagePresenter::setBacklightValue(uint8_t value)
{
    model->setBacklightValue(value);
}

void HomePagePresenter::switchToPowerPage()
{
    view.switchToPowerPage();
}

void HomePagePresenter::switchToHomePage()
{
    view.switchToHomePage();
}
