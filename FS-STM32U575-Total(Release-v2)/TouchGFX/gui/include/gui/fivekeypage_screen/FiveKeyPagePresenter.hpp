#ifndef FIVEKEYPAGEPRESENTER_HPP
#define FIVEKEYPAGEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class FiveKeyPageView;

class FiveKeyPagePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    FiveKeyPagePresenter(FiveKeyPageView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~FiveKeyPagePresenter() {};
		//更新五向键数据
		virtual void updateFiveKey(uint16_t newFiveKeyVal);
		//FiveKeyPagePresenter的状态
		void FiveKeyPagePresenterState(bool enable);
private:
    FiveKeyPagePresenter();

    FiveKeyPageView& view;
};

#endif // FIVEKEYPAGEPRESENTER_HPP
