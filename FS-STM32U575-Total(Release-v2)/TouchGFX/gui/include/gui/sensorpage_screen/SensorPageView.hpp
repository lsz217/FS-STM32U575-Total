#ifndef SENSORPAGEVIEW_HPP
#define SENSORPAGEVIEW_HPP

#include <gui_generated/sensorpage_screen/SensorPageViewBase.hpp>
#include <gui/sensorpage_screen/SensorPagePresenter.hpp>
#include <touchgfx/widgets/Button.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>

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
    virtual void showBuzzer();
    virtual void showSmoke();
    virtual void showAlcohol();

    // Update sensor data on screen (keeps buffers fresh)
    virtual void updateSensorInfo(float temperature, float humidity,
                                   uint16_t co2, uint32_t heartRate, uint32_t spo2);
    virtual void updateBuzzerStatus(bool alarmOn);
    virtual void updateSmokeStatus(bool alarmOn);
    virtual void updateAlcoholStatus(bool alarmOn);

private:
    void hideActiveModal();
    void hideAllModals();
    void setIconsForModal(int modal); // -1=all, 0=Temp, 1=Hum, 2=CO2, 3=Heart, 4=Buzzer, 5=Alcohol, 6=Smoke
    int8_t activeModal; // -1=none, 0=temp, 1=hum, 2=co2, 3=heart, 4=buzzer, 5=alcohol, 6=smoke

    // Close/exit buttons for each modal window
    touchgfx::Button closeButton1;
    touchgfx::Button closeButton2;
    touchgfx::Button closeButton3;
    touchgfx::Button closeButton4;
    touchgfx::Button closeButton5;
    touchgfx::Button closeButton6;
    touchgfx::Button closeButton7;

    // Callback for close buttons
    touchgfx::Callback<SensorPageView, const touchgfx::AbstractButton&> closeButtonCallback;
    void closeButtonCallbackHandler(const touchgfx::AbstractButton& src);

    // Wildcard textAreas inside modals (base class textArea1-4 are plain TextArea now)
    touchgfx::TextAreaWithOneWildcard textModal1;
    touchgfx::TextAreaWithOneWildcard textModal2;
    touchgfx::TextAreaWithOneWildcard textModal3;
    touchgfx::TextAreaWithOneWildcard textModal4;
    touchgfx::TextAreaWithOneWildcard textModal5;
    touchgfx::TextAreaWithOneWildcard textModal6;
    touchgfx::TextAreaWithOneWildcard textModal7;

    static const uint16_t TEXTAREA1_SIZE = 30;
    touchgfx::Unicode::UnicodeChar textArea1Buffer[TEXTAREA1_SIZE];
    static const uint16_t TEXTAREA2_SIZE = 30;
    touchgfx::Unicode::UnicodeChar textArea2Buffer[TEXTAREA2_SIZE];
    static const uint16_t TEXTAREA3_SIZE = 30;
    touchgfx::Unicode::UnicodeChar textArea3Buffer[TEXTAREA3_SIZE];
    static const uint16_t TEXTAREA4_SIZE = 40;
    touchgfx::Unicode::UnicodeChar textArea4Buffer[TEXTAREA4_SIZE];
    static const uint16_t TEXTAREA5_SIZE = 30;
    touchgfx::Unicode::UnicodeChar textArea5Buffer[TEXTAREA5_SIZE];
    static const uint16_t TEXTAREA6_SIZE = 30;
    touchgfx::Unicode::UnicodeChar textArea6Buffer[TEXTAREA6_SIZE];
    static const uint16_t TEXTAREA7_SIZE = 30;
    touchgfx::Unicode::UnicodeChar textArea7Buffer[TEXTAREA7_SIZE];

    // Callback for flexButton icon tap zones
    touchgfx::Callback<SensorPageView, const touchgfx::AbstractButtonContainer&> flexButtonCallback;
    void flexButtonCallbackHandler(const touchgfx::AbstractButtonContainer& src);

    // Deadband tracking: only redraw when value changes meaningfully
    float    lastTemp;
    float    lastHum;
    uint16_t lastCO2;
    uint32_t lastHR;
    uint32_t lastSpO2;
    bool     sensorDataReady; // false until first measurement arrives

    // 15s hold-over for HR/SpO2: count consecutive invalid frames
    int32_t hrInvalidCount; // 0=receiving valid data, >0=consecutive invalid frames
};

#endif // SENSORPAGEVIEW_HPP
