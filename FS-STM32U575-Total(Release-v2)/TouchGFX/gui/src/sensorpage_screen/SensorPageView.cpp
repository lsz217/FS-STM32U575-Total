#include <gui/sensorpage_screen/SensorPageView.hpp>
#include <images/BitmapDatabase.hpp>
#include <texts/TextKeysAndLanguages.hpp>

SensorPageView::SensorPageView()
    : activeModal(-1),
      lastTemp(0), lastHum(0), lastCO2(0), lastHR(0), lastSpO2(0),
      sensorDataReady(false), hrInvalidCount(0),
      closeButtonCallback(this, &SensorPageView::closeButtonCallbackHandler)
{
}

void SensorPageView::setupScreen()
{
    SensorPageViewBase::setupScreen();

    // Set explicit sizes — TypedText template is just "<>" so width auto-calcs to ~0
    textArea1.setPosition(105, 136, 200, 30);
    textArea2.setPosition(93, 109, 200, 30);
    textArea3.setPosition(15, 175, 200, 30);
    textArea4.setPosition(5, 186, 310, 30);

    // Show "measuring" placeholder until first data arrives
    Unicode::strncpy(textArea1Buffer, "Tem: -- C", TEXTAREA1_SIZE);
    Unicode::strncpy(textArea2Buffer, "Hum: -- %", TEXTAREA2_SIZE);
    Unicode::strncpy(textArea3Buffer, "CO2: -- ppm", TEXTAREA3_SIZE);
    Unicode::strncpy(textArea4Buffer, "HR: -- bpm  SpO2: --", TEXTAREA4_SIZE);

    sensorDataReady = false;

    // Add close/exit buttons to each modal window
    closeButton1.setXY(261, 186);
    closeButton1.setBitmaps(touchgfx::Bitmap(BITMAP_IMAGE_MENU_ICON_ID), touchgfx::Bitmap(BITMAP_IMAGE_MENU_ICON_PRESSED_ID));
    closeButton1.setAction(closeButtonCallback);
    modalWindow1.add(closeButton1);

    closeButton2.setXY(261, 186);
    closeButton2.setBitmaps(touchgfx::Bitmap(BITMAP_IMAGE_MENU_ICON_ID), touchgfx::Bitmap(BITMAP_IMAGE_MENU_ICON_PRESSED_ID));
    closeButton2.setAction(closeButtonCallback);
    modalWindow2.add(closeButton2);

    closeButton3.setXY(261, 186);
    closeButton3.setBitmaps(touchgfx::Bitmap(BITMAP_IMAGE_MENU_ICON_ID), touchgfx::Bitmap(BITMAP_IMAGE_MENU_ICON_PRESSED_ID));
    closeButton3.setAction(closeButtonCallback);
    modalWindow3.add(closeButton3);

    closeButton4.setXY(261, 186);
    closeButton4.setBitmaps(touchgfx::Bitmap(BITMAP_IMAGE_MENU_ICON_ID), touchgfx::Bitmap(BITMAP_IMAGE_MENU_ICON_PRESSED_ID));
    closeButton4.setAction(closeButtonCallback);
    modalWindow4.add(closeButton4);
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
        modalWindow1.hide();
    else if (activeModal == 1)
        modalWindow2.hide();
    else if (activeModal == 2)
        modalWindow3.hide();
    else if (activeModal == 3)
        modalWindow4.hide();
}

void SensorPageView::hideAllModals()
{
    hideActiveModal();
    activeModal = -1;
    invalidate();
}

void SensorPageView::closeButtonCallbackHandler(const touchgfx::AbstractButton& src)
{
    hideActiveModal();
    activeModal = -1;
    invalidate();
}

void SensorPageView::showTemp()
{
    if (activeModal == 0)
    {
        modalWindow1.hide();
        activeModal = -1;
    }
    else
    {
        if (activeModal >= 0)
            hideActiveModal();
        modalWindow1.show();
        activeModal = 0;
        textArea1.invalidate();
    }
    invalidate();
}

void SensorPageView::showHum()
{
    if (activeModal == 1)
    {
        modalWindow2.hide();
        activeModal = -1;
    }
    else
    {
        if (activeModal >= 0)
            hideActiveModal();
        modalWindow2.show();
        activeModal = 1;
        textArea2.invalidate();
    }
    invalidate();
}

void SensorPageView::showCO2()
{
    if (activeModal == 2)
    {
        modalWindow3.hide();
        activeModal = -1;
    }
    else
    {
        if (activeModal >= 0)
            hideActiveModal();
        modalWindow3.show();
        activeModal = 2;
        textArea3.invalidate();
    }
    invalidate();
}

void SensorPageView::showHeartRate()
{
    if (activeModal == 3)
    {
        modalWindow4.hide();
        activeModal = -1;
    }
    else
    {
        if (activeModal >= 0)
            hideActiveModal();
        modalWindow4.show();
        activeModal = 3;
        textArea4.invalidate();
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
        Unicode::snprintfFloat(textArea2Buffer, TEXTAREA2_SIZE, "Hum: %.1f", humidity);
        // Append " %" manually — TouchGFX snprintfFloat doesn't support %%
        uint32_t len = Unicode::strlen(textArea2Buffer);
        if (len + 2 < TEXTAREA2_SIZE)
        {
            textArea2Buffer[len]     = ' ';
            textArea2Buffer[len + 1] = '%';
            textArea2Buffer[len + 2] = '\0';
        }
        if (textArea2.isVisible())
            textArea2.invalidate();
        lastHum = humidity;
    }

    // CO2
    if (!sensorDataReady || (co2 > lastCO2 + CO2_DELTA) || (co2 < lastCO2 - CO2_DELTA))
    {
        Unicode::snprintf(textArea3Buffer, TEXTAREA3_SIZE, "CO2: %u ppm", co2);
        if (textArea3.isVisible())
            textArea3.invalidate();
        lastCO2 = co2;
    }

    // Heart rate / SpO2 — 0xFFFFFFFF sentinel means sensor not ready
    const uint32_t SENTINEL = 0xFFFFFFFF;
    bool hrValid   = (heartRate != SENTINEL);
    bool spo2Valid = (spo2     != SENTINEL);

    if (hrValid || spo2Valid)
    {
        // Valid data — reset counter and show live values
        hrInvalidCount = 0;

        bool lastHrValid   = (lastHR   != SENTINEL);
        bool lastSpo2Valid = (lastSpO2 != SENTINEL);
        bool hrChanged   = (hrValid && lastHrValid &&
            (heartRate > lastHR + HR_DELTA || heartRate < lastHR - HR_DELTA));
        bool spo2Changed = (spo2Valid && lastSpo2Valid &&
            absDiffU32(spo2, lastSpO2) >= SPO2_DELTA);
        bool validityChanged = (hrValid != lastHrValid) || (spo2Valid != lastSpo2Valid);

        if (!sensorDataReady || hrChanged || spo2Changed || validityChanged)
        {
            if (hrValid && spo2Valid)
                Unicode::snprintf(textArea4Buffer, TEXTAREA4_SIZE, "HR: %u bpm  SpO2: %u",
                                  heartRate, spo2);
            else if (hrValid)
                Unicode::snprintf(textArea4Buffer, TEXTAREA4_SIZE, "HR: %u bpm  SpO2: --",
                                  heartRate);
            else if (spo2Valid)
                Unicode::snprintf(textArea4Buffer, TEXTAREA4_SIZE, "HR: -- bpm  SpO2: %u",
                                  spo2);

            // Append " %" only when SpO2 is valid
            if (spo2Valid)
            {
                uint32_t len = Unicode::strlen(textArea4Buffer);
                if (len + 2 < TEXTAREA4_SIZE)
                {
                    textArea4Buffer[len]     = ' ';
                    textArea4Buffer[len + 1] = '%';
                    textArea4Buffer[len + 2] = '\0';
                }
            }
            if (textArea4.isVisible())
                textArea4.invalidate();
        }
        lastHR   = hrValid   ? heartRate : SENTINEL;
        lastSpO2 = spo2Valid ? spo2      : SENTINEL;
    }
    else
    {
        // No valid signal — count consecutive invalid frames
        hrInvalidCount++;
        if (hrInvalidCount == 900) // ~15s at 60fps
        {
            Unicode::strncpy(textArea4Buffer, "HR: -- bpm  SpO2: --", TEXTAREA4_SIZE);
            if (textArea4.isVisible())
                textArea4.invalidate();
            lastHR   = SENTINEL;
            lastSpO2 = SENTINEL;
        }
        // else: keep showing last valid values
    }

    sensorDataReady = true;
}
