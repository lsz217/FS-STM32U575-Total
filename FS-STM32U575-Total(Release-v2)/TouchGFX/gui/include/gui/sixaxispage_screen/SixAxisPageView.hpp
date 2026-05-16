#ifndef SIXAXISPAGEVIEW_HPP
#define SIXAXISPAGEVIEW_HPP

#include <gui_generated/sixaxispage_screen/SixAxisPageViewBase.hpp>
#include <gui/sixaxispage_screen/SixAxisPagePresenter.hpp>

class SixAxisPageView : public SixAxisPageViewBase
{
public:
    SixAxisPageView();
    virtual ~SixAxisPageView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
		virtual void updateSixAxis(float newPitch,float newRoll,float newYaw);
protected:
};

#endif // SIXAXISPAGEVIEW_HPP
