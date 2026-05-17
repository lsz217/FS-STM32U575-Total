#ifndef FRONTENDAPPLICATION_HPP
#define FRONTENDAPPLICATION_HPP

#include <gui_generated/common/FrontendApplicationBase.hpp>

class FrontendHeap;

using namespace touchgfx;

class FrontendApplication : public FrontendApplicationBase
{
public:
    FrontendApplication(Model& m, FrontendHeap& heap);
    virtual ~FrontendApplication() { }

    virtual void handleTickEvent()
    {
        model.tick();
        FrontendApplicationBase::handleTickEvent();
    }

    // SensorPage transition — user code because Designer may not generate it
    void gotoSensorPageScreenNoTransition();
    void gotoSensorPageScreenNoTransitionImpl();

private:
    touchgfx::Callback<FrontendApplication> sensorPageTransitionCallback;
};

#endif // FRONTENDAPPLICATION_HPP
