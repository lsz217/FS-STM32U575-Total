#include <gui/common/FrontendApplication.hpp>
#include <gui/common/FrontendHeap.hpp>
#include <gui/sensorpage_screen/SensorPageView.hpp>
#include <gui/sensorpage_screen/SensorPagePresenter.hpp>
#include <touchgfx/transitions/NoTransition.hpp>

FrontendApplication::FrontendApplication(Model& m, FrontendHeap& heap)
    : FrontendApplicationBase(m, heap),
      sensorPageTransitionCallback(this, &FrontendApplication::gotoSensorPageScreenNoTransitionImpl)
{
}

void FrontendApplication::gotoSensorPageScreenNoTransition()
{
    pendingScreenTransitionCallback = &sensorPageTransitionCallback;
}

void FrontendApplication::gotoSensorPageScreenNoTransitionImpl()
{
    touchgfx::makeTransition<SensorPageView, SensorPagePresenter, touchgfx::NoTransition, Model >(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}
