#include <gui/sensorpage_screen/SensorPageView.hpp>
#include <gui/sensorpage_screen/SensorPagePresenter.hpp>

SensorPagePresenter::SensorPagePresenter(SensorPageView& v)
    : view(v)
{
}

void SensorPagePresenter::activate()
{
    model->SensorPageViewTask(true);
    model->HeartRateTaskEnable(true);
}

void SensorPagePresenter::deactivate()
{
    model->SensorPageViewTask(false);
    model->HeartRateTaskEnable(false);
}

void SensorPagePresenter::updateSensorInfo(float temperature, float humidity,
                                            uint16_t co2, uint32_t heartRate, uint32_t spo2)
{
    view.updateSensorInfo(temperature, humidity, co2, heartRate, spo2);
}

void SensorPagePresenter::updateBuzzerStatus(bool alarmOn)
{
    view.updateBuzzerStatus(alarmOn);
}

void SensorPagePresenter::updateSmokeStatus(bool alarmOn)
{
    view.updateSmokeStatus(alarmOn);
}

void SensorPagePresenter::updateAlcoholStatus(bool alarmOn)
{
    view.updateAlcoholStatus(alarmOn);
}

void SensorPagePresenter::setMotorState(bool on)
{
    model->setMotorOn(on);
}

void SensorPagePresenter::setBuzzerState(bool on)
{
    model->setBuzzerOn(on);
}
