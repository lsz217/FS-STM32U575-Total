#include <gui/startuppage_screen/startuppageView.hpp>
#include <gui/startuppage_screen/startuppagePresenter.hpp>
#include <texts/TextKeysAndLanguages.hpp> // 包含文本资源
// 必须包含这些头文件来控制背光
extern "C" {
    #include "main.h"
    #include "tim.h"
}

startuppageView::startuppageView()
{
}
void startuppageView::setupScreen()
{
    startuppageViewBase::setupScreen();

    // 1. 【软件层】确保黑色遮罩是显示的
    blackCover.setVisible(true);
    
    // 2. 【硬件层】确保背光是关闭的
    // 直接调 Presenter -> Model -> 硬件
    presenter->setBacklightValue(0); 

    // 刷新一下
    blackCover.invalidate();
}

void startuppageView::function5()
{    
	   presenter->resetIdleTimer();
    // 1. 【硬件层】把背光拉满 (100)
    presenter->setBacklightValue(100); 

    // 2. 【软件层】把黑色遮罩隐藏，露出后面的白色 Box 或 UI
    blackCover.setVisible(false);
    
    // 3. 【交互层】把全屏按钮也关掉，否则用户点不到后面的 UI
    flexButton1.setVisible(false);

    // 这一步必须有，告诉绘图引擎重绘屏幕
	   buttonWithIcon1.setVisible(true);
    invalidate(); 
}
void startuppageView::goToSleep()
{
    // 1. 硬件息屏：把亮度调到 0
    presenter->setBacklightValue(0);

    // 2. 软件遮罩：把黑色 Box 显示出来
    blackCover.setVisible(true);
    
    // 3. 交互恢复：把全屏透明按钮显示出来，等待下次点击唤醒
    flexButton1.setVisible(true);

    // 4. 刷新屏幕
    invalidate();
}

void startuppageView::updateNfcStatus()
{

// 第 63 行改为：
// 这一行代码的意思是：把字库里 ResourceId2 (识别成功) 的内容，写进 textArea1 的黑板里
// 这行代码会把 ResourceId2 (识别成功) 的内容“搬”到 textArea1 里面
// 1. 修改通配符缓冲区的内容
// 注意：是 textArea1 (数字1)，不是 textAreal (字母L)
	
    // 1. 把“请刷NFC”藏起来
    textArea1.setVisible(false);
    textArea1.invalidate();

    // 2. 把“识别成功”显示出来
    textArea2.setVisible(true);
    textArea2.invalidate();


// 第 66 行改为：
textArea1.invalidate();
}

void startuppageView::tearDownScreen()
{
    startuppageViewBase::tearDownScreen();
}