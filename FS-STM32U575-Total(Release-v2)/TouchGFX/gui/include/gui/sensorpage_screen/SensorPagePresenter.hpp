#ifndef SENSORPAGEPRESENTER_HPP
#define SENSORPAGEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class SensorPageView;

class SensorPagePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    SensorPagePresenter(SensorPageView& v);

    virtual void activate();
    virtual void deactivate();

    virtual ~SensorPagePresenter() {};

    // Called by Model when sensor data is updated
    virtual void updateSensorInfo(float temperature, float humidity,
                                   uint16_t co2, uint32_t heartRate, uint32_t spo2);

    // Motor control from View
    void setMotorState(bool on);

private:
    SensorPagePresenter();
    SensorPageView& view;
};

#endif // SENSORPAGEPRESENTER_HPP
