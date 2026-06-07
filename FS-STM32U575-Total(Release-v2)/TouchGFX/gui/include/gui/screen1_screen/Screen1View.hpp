#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <touchgfx/widgets/graph/GraphScroll.hpp>
#include <touchgfx/widgets/graph/GraphElements.hpp>
#include <touchgfx/widgets/canvas/PainterRGB565.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();

    void addTempPoint(float temperature);
    void addHRPoint(int heartRate);

protected:
    // 温度折线图
    touchgfx::GraphScroll<100> tempGraph;
    touchgfx::GraphElementLine tempLine;
    touchgfx::PainterRGB565 tempPainter;

    // 心率折线图
    touchgfx::GraphScroll<100> hrGraph;
    touchgfx::GraphElementLine hrLine;
    touchgfx::PainterRGB565 hrPainter;

    // 数据节流计数
    int16_t tickCounter;
    float pendingTemp;
    int   pendingHR;
    bool  tempUpdated;
    bool  hrUpdated;
};

#endif // SCREEN1VIEW_HPP
