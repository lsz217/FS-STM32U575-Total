#ifndef FIVEKEYPAGEVIEW_HPP
#define FIVEKEYPAGEVIEW_HPP

#include <gui_generated/fivekeypage_screen/FiveKeyPageViewBase.hpp>
#include <gui/fivekeypage_screen/FiveKeyPagePresenter.hpp>

class FiveKeyPageView : public FiveKeyPageViewBase
{
public:
    FiveKeyPageView();
    virtual ~FiveKeyPageView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
		//更新五向键数据
		void updateFiveKey(uint16_t newFiveKeyVal);
protected:
};

#endif // FIVEKEYPAGEVIEW_HPP
