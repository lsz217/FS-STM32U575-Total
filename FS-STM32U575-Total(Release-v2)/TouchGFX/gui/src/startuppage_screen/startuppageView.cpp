#include <gui/startuppage_screen/startuppageView.hpp>
#include <gui/startuppage_screen/startuppagePresenter.hpp>
#include <texts/TextKeysAndLanguages.hpp> // 包含文本资源
// 必须包含这些头文件来控制背光
extern "C" {
    #include "main.h"
    #include "tim.h"
}

startuppageView::startuppageView()
    : bootDelayCounter(0), nfcSuccessDelay(0),
      nfcButtonCallback(this, &startuppageView::nfcButtonCallbackHandler)
{
}
void startuppageView::setupScreen()
{
    startuppageViewBase::setupScreen();

    // 1. 【软件层】确保黑色遮罩是显示的
    blackCover.setVisible(true);

    // 2. 【硬件层】确保背光是关闭的
    presenter->setBacklightValue(0);

    // 3. 给 flexButton2（透明NFC按钮）绑定点击动作：弹出NFC提示弹窗
    flexButton2.setAction(nfcButtonCallback);

    // 刷新一下
    blackCover.invalidate();
}

void startuppageView::handleTickEvent()
{
    if (nfcSuccessDelay > 0)
    {
        nfcSuccessDelay--;
        if (nfcSuccessDelay == 0)
        {
            // 1秒倒计时结束，跳转到HomePage
            application().gotoHomePageScreenNoTransition();
        }
    }
}

void startuppageView::function5()
{
    presenter->resetIdleTimer();
    // 1. 【硬件层】把背光拉满 (100)
    presenter->setBacklightValue(100);

    // 2. 【软件层】把黑色遮罩隐藏，露出锁屏UI底图
    blackCover.setVisible(false);

    // 3. 【交互层】把全屏唤醒按钮隐藏
    flexButton1.setVisible(false);

    // 4. 显示透明NFC触摸按钮，让用户点击进入刷卡
    flexButton2.setVisible(true);

    invalidate();
}
void startuppageView::goToSleep()
{
    // 1. 硬件息屏：把亮度调到 0
    presenter->setBacklightValue(0);

    // 2. 软件遮罩：把黑色 Box 显示出来
    blackCover.setVisible(true);

    // 3. 交互恢复：全屏唤醒按钮显示，NFC按钮隐藏
    flexButton1.setVisible(true);
    flexButton2.setVisible(false);

    // 4. 刷新屏幕
    invalidate();
}

void startuppageView::updateNfcStatus()
{
    // 1. 把"请刷NFC"藏起来
    textArea1.setVisible(false);
    textArea1.invalidate();

    // 2. 把"识别成功"显示出来
    textArea2.setVisible(true);
    textArea2.invalidate();

    // 3. 启动1秒倒计时（60帧 ≈ 1秒@60fps）
    nfcSuccessDelay = 60;
}

void startuppageView::nfcButtonCallbackHandler(const touchgfx::AbstractButtonContainer& src)
{
    // 点击透明NFC按钮 → 弹出NFC提示弹窗
    modalWindow1.show();
}

void startuppageView::tearDownScreen()
{
    startuppageViewBase::tearDownScreen();
}

void startuppageView::navigateToHomePage()
{
    application().gotoHomePageScreenNoTransition();
}
