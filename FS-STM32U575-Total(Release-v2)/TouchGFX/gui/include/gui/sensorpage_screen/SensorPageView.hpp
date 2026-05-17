#ifndef SENSORPAGEVIEW_HPP
#define SENSORPAGEVIEW_HPP

#include <gui_generated/sensorpage_screen/SensorPageViewBase.hpp>
#include <gui/sensorpage_screen/SensorPagePresenter.hpp>
#include <touchgfx/widgets/Button.hpp>

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

    // Close/exit buttons for each modal window
    touchgfx::Button closeButton1;
    touchgfx::Button closeButton2;
    touchgfx::Button closeButton3;
    touchgfx::Button closeButton4;

    // Callback for close buttons
    touchgfx::Callback<SensorPageView, const touchgfx::AbstractButton&> closeButtonCallback;
    void closeButtonCallbackHandler(const touchgfx::AbstractButton& src);

    // Deadband tracking: only redraw when value changes meaningfully
    float    lastTemp;
    float    lastHum;
    uint16_t lastCO2;
    uint32_t lastHR;
    uint32_t lastSpO2;
    bool     sensorDataReady; // false until first measurement arrives

    // 30s hold-over for HR/SpO2: count consecutive invalid frames
    int32_t hrInvalidCount; // 0=receiving valid data, >0=consecutive invalid frames
};

#endif // SENSORPAGEVIEW_HPP
