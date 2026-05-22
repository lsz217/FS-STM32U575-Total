#ifndef STARTUPPAGEVIEW_HPP
#define STARTUPPAGEVIEW_HPP

#include <gui_generated/startuppage_screen/startuppageViewBase.hpp>
#include <gui/startuppage_screen/startuppagePresenter.hpp>

class startuppageView : public startuppageViewBase
{
public:
    startuppageView();
    virtual ~startuppageView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();

    virtual void function5();
    void goToSleep();
    void updateNfcStatus();
    void navigateToHomePage();

protected:
    int bootDelayCounter;
    int nfcSuccessDelay;

private:
    // flexButton2 (透明NFC按钮) 的回调
    touchgfx::Callback<startuppageView, const touchgfx::AbstractButtonContainer&> nfcButtonCallback;
    void nfcButtonCallbackHandler(const touchgfx::AbstractButtonContainer& src);
};

#endif // STARTUPPAGEVIEW_HPP
