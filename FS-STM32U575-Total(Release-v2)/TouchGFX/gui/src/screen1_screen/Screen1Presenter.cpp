#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

Screen1Presenter::Screen1Presenter(Screen1View& v)
    : view(v)
{

}

void Screen1Presenter::activate()
{
    model->SensorPageViewTask(true);
}

void Screen1Presenter::deactivate()
{
    model->SensorPageViewTask(false);
}

void Screen1Presenter::updateSensorInfo(float temperature, float humidity, uint16_t co2, uint32_t heartRate, uint32_t spo2)
{
    view.addTempPoint(temperature);
    if (heartRate != 0xFFFFFFFF)
        view.addHRPoint((int)heartRate);
}
