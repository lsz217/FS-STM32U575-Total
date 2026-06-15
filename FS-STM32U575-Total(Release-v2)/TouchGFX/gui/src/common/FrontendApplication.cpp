#include <gui/common/FrontendApplication.hpp>
#include <gui/common/FrontendHeap.hpp>
#include <gui/sensorpage_screen/SensorPageView.hpp>
#include <gui/sensorpage_screen/SensorPagePresenter.hpp>
#include <gui/homepage_screen/HomePageView.hpp>
#include <gui/homepage_screen/HomePagePresenter.hpp>
#include <touchgfx/transitions/NoTransition.hpp>

FrontendApplication::FrontendApplication(Model& m, FrontendHeap& heap)
    : FrontendApplicationBase(m, heap)
{
}

void FrontendApplication::gotoSensorPageScreenNoTransition()
{
    sensorPageTransitionCallback = touchgfx::Callback<FrontendApplication>(this, &FrontendApplication::gotoSensorPageScreenNoTransitionImpl);
    pendingScreenTransitionCallback = &sensorPageTransitionCallback;
}

void FrontendApplication::gotoSensorPageScreenNoTransitionImpl()
{
    touchgfx::makeTransition<SensorPageView, SensorPagePresenter, touchgfx::NoTransition, Model>(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}

void FrontendApplication::gotoHomePageScreenNoTransition()
{
    homePageTransitionCallback = touchgfx::Callback<FrontendApplication>(this, &FrontendApplication::gotoHomePageScreenNoTransitionImpl);
    pendingScreenTransitionCallback = &homePageTransitionCallback;
}

void FrontendApplication::gotoHomePageScreenNoTransitionImpl()
{
    touchgfx::makeTransition<HomePageView, HomePagePresenter, touchgfx::NoTransition, Model>(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}
