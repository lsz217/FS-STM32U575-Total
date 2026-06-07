#include <gui/screen1_screen/Screen1View.hpp>
#include <touchgfx/Color.hpp>

Screen1View::Screen1View()
    : tickCounter(0), pendingTemp(0), pendingHR(0), tempUpdated(false), hrUpdated(false)
{
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    // 隐藏基类原有的 GraphWrapAndClear，用我们自己的
    dynamicGraph1.setVisible(false);

    // ===== 温度折线图（上半屏）=====
    tempPainter.setColor(touchgfx::Color::getColorFromRGB(255, 60, 60));
    tempLine.setPainter(tempPainter);
    tempLine.setLineWidth(2);
    tempGraph.addGraphElement(tempLine);

    tempGraph.setPosition(0, 0, 320, 120);
    tempGraph.setGraphAreaMargin(5, 35, 5, 25);
    tempGraph.setGraphRangeY(0, 60);

    add(tempGraph);

    // ===== 心率折线图（下半屏）=====
    hrPainter.setColor(touchgfx::Color::getColorFromRGB(60, 140, 255));
    hrLine.setPainter(hrPainter);
    hrLine.setLineWidth(2);
    hrGraph.addGraphElement(hrLine);

    hrGraph.setPosition(0, 120, 320, 120);
    hrGraph.setGraphAreaMargin(5, 35, 5, 25);
    hrGraph.setGraphRangeY(40, 180);

    add(hrGraph);
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleTickEvent()
{
    // 约 1 秒喂一个数据点（60 tick ≈ 1 秒 @ 60fps）
    tickCounter++;
    if (tickCounter >= 60)
    {
        tickCounter = 0;

        if (tempUpdated)
        {
            tempGraph.addDataPoint(pendingTemp);
            tempUpdated = false;
        }
        if (hrUpdated)
        {
            hrGraph.addDataPoint(pendingHR);
            hrUpdated = false;
        }
    }
}

void Screen1View::addTempPoint(float temperature)
{
    pendingTemp = temperature;
    tempUpdated = true;
}

void Screen1View::addHRPoint(int heartRate)
{
    pendingHR = heartRate;
    hrUpdated = true;
}
