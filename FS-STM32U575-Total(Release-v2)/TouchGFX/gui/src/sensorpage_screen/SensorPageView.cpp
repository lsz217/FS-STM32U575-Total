#include <gui/sensorpage_screen/SensorPageView.hpp>
#include <texts/TextKeysAndLanguages.hpp>

SensorPageView::SensorPageView()
    : activeModal(-1),
      lastTemp(0), lastHum(0), lastCO2(0), lastHR(0), lastSpO2(0),
      sensorDataReady(false)
{
}

void SensorPageView::setupScreen()
{
    SensorPageViewBase::setupScreen();

    // Show "measuring" placeholder until first data arrives
    Unicode::strncpy(textArea1Buffer, "Tem: -- C", TEXTAREA1_SIZE);
    Unicode::strncpy(textArea2Buffer, "Hum: -- %", TEXTAREA2_SIZE);
    Unicode::strncpy(textArea3Buffer, "CO2: -- ppm", TEXTAREA3_SIZE);
    Unicode::strncpy(textArea4Buffer, "HR: -- bpm", TEXTAREA4_SIZE);

    sensorDataReady = false;
}

void SensorPageView::tearDownScreen()
{
    SensorPageViewBase::tearDownScreen();
    hideAllModals();
}

void SensorPageView::backToHome()
{
    application().gotoHomePageScreenNoTransition();
}

void SensorPageView::hideActiveModal()
{
    if (activeModal == 0)
        modalWindow1.setVisible(false);
    else if (activeModal == 1)
        modalWindow2.setVisible(false);
    else if (activeModal == 2)
        modalWindow3.setVisible(false);
    else if (activeModal == 3)
        modalWindow4.setVisible(false);
}

void SensorPageView::hideAllModals()
{
    hideActiveModal();
    activeModal = -1;
    invalidate();
}

void SensorPageView::showTemp()
{
    if (activeModal == 0)
    {
        modalWindow1.setVisible(false);
        activeModal = -1;
    }
    else
    {
        if (activeModal >= 0)
            hideActiveModal();
        modalWindow1.setVisible(true);
        activeModal = 0;
    }
    invalidate();
}

void SensorPageView::showHum()
{
    if (activeModal == 1)
    {
        modalWindow2.setVisible(false);
        activeModal = -1;
    }
    else
    {
        if (activeModal >= 0)
            hideActiveModal();
        modalWindow2.setVisible(true);
        activeModal = 1;
    }
    invalidate();
}

void SensorPageView::showCO2()
{
    if (activeModal == 2)
    {
        modalWindow3.setVisible(false);
        activeModal = -1;
    }
    else
    {
        if (activeModal >= 0)
            hideActiveModal();
        modalWindow3.setVisible(true);
        activeModal = 2;
    }
    invalidate();
}

void SensorPageView::showHeartRate()
{
    if (activeModal == 3)
    {
        modalWindow4.setVisible(false);
        activeModal = -1;
    }
    else
    {
        if (activeModal >= 0)
            hideActiveModal();
        modalWindow4.setVisible(true);
        activeModal = 3;
    }
    invalidate();
}

void SensorPageView::updateSensorInfo(float temperature, float humidity,
                                       uint16_t co2, uint32_t heartRate, uint32_t spo2)
{
    // Deadband thresholds — only redraw when change exceeds these
    const float    TEMP_DELTA  = 0.5f;
    const float    HUM_DELTA   = 1.0f;
    const uint16_t CO2_DELTA   = 10;
    const uint32_t HR_DELTA    = 5;
    const uint32_t SPO2_DELTA  = 2;

    // Helper lambda: absolute difference for unsigned types
    auto absDiffU32 = [](uint32_t a, uint32_t b) -> uint32_t {
        return a > b ? a - b : b - a;
    };

    // Temperature
    if (!sensorDataReady || (temperature > lastTemp + TEMP_DELTA) || (temperature < lastTemp - TEMP_DELTA))
    {
        Unicode::snprintfFloat(textArea1Buffer, TEXTAREA1_SIZE, "Tem: %.1f C", temperature);
        if (textArea1.isVisible())
            textArea1.invalidate();
        lastTemp = temperature;
    }

    // Humidity
    if (!sensorDataReady || (humidity > lastHum + HUM_DELTA) || (humidity < lastHum - HUM_DELTA))
    {
        Unicode::snprintfFloat(textArea2Buffer, TEXTAREA2_SIZE, "Hum: %.1f %%", humidity);
        if (textArea2.isVisible())
            textArea2.invalidate();
        lastHum = humidity;
    }

    // CO2
    if (!sensorDataReady || (co2 > lastCO2 + CO2_DELTA) || (co2 < lastCO2 - CO2_DELTA))
    {
        Unicode::snprintf(textArea3Buffer, TEXTAREA3_SIZE, "CO2: %d ppm", co2);
        if (textArea3.isVisible())
            textArea3.invalidate();
        lastCO2 = co2;
    }

    // Heart rate / SpO2 — update if either value changes meaningfully
    if (!sensorDataReady ||
        (heartRate > lastHR + HR_DELTA) || (heartRate < lastHR - HR_DELTA) ||
        absDiffU32(spo2, lastSpO2) >= SPO2_DELTA)
    {
        Unicode::snprintf(textArea4Buffer, TEXTAREA4_SIZE, "HR: %d bpm  SpO2: %d %%",
                          heartRate, spo2);
        if (textArea4.isVisible())
            textArea4.invalidate();
        lastHR   = heartRate;
        lastSpO2 = spo2;
    }

    sensorDataReady = true;
}
