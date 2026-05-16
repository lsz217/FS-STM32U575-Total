#ifndef SENSORPAGEVIEW_HPP
#define SENSORPAGEVIEW_HPP

#include <gui_generated/sensorpage_screen/SensorPageViewBase.hpp>
#include <gui/sensorpage_screen/SensorPagePresenter.hpp>

class SensorPageView : public SensorPageViewBase
{
public:
    SensorPageView();
    virtual ~SensorPageView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    // Override back button handler
    virtual void backToHome();

    // Override icon click handlers
    virtual void showTemp();
    virtual void showHum();
    virtual void showCO2();
    virtual void showHeartRate();

    // Update sensor data on screen (keeps buffers fresh)
    virtual void updateSensorInfo(float temperature, float humidity,
                                   uint16_t co2, uint32_t heartRate, uint32_t spo2);

private:
    void hideActiveModal();
    void hideAllModals();
    int8_t activeModal; // -1=none, 0=temp, 1=hum, 2=co2, 3=heart

    // Deadband tracking: only redraw when value changes meaningfully
    float    lastTemp;
    float    lastHum;
    uint16_t lastCO2;
    uint32_t lastHR;
    uint32_t lastSpO2;
    bool     sensorDataReady; // false until first measurement arrives
};

#endif // SENSORPAGEVIEW_HPP
