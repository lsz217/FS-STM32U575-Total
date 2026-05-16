#include <gui/startuppage_screen/startuppageView.hpp>
#include <gui/startuppage_screen/startuppagePresenter.hpp>

startuppagePresenter::startuppagePresenter(startuppageView& v)
    : view(v)
{

}

void startuppagePresenter::activate()
{

}

void startuppagePresenter::deactivate()
{

}
void startuppagePresenter::buttonPressed()
{
    // 可以在这里调用 model，或者简单的测试：
    // model->doSomething();
}

void startuppagePresenter::setBacklightValue(uint8_t value)
{
    // 调用 Model 里的那个已经写好的 setBacklightValue
    model->setBacklightValue(value);
}
void startuppagePresenter::notifySleep()
{
    // 当 Model 告诉我们时间到了，我们就命令 View 去息屏
    view.goToSleep();
}
// Presenter.cpp
void startuppagePresenter::resetIdleTimer()
{
    model->resetTimer(); // 最终调回到 Model.cpp 里的那个 resetTimer
}

void startuppagePresenter::nfcTriggered()
{
    view.updateNfcStatus(); // 调用 View 的方法去改界面
}