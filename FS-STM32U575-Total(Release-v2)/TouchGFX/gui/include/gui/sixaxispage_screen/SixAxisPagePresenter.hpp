#ifndef SIXAXISPAGEPRESENTER_HPP
#define SIXAXISPAGEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class SixAxisPageView;

class SixAxisPagePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    SixAxisPagePresenter(SixAxisPageView& v);

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

    virtual ~SixAxisPagePresenter() {};
		virtual void updateSixAxis(float newPitch,float newRoll,float newYaw);
		//SixAxisPagePresenter的状态
		void SixAxisPagePresenterState(bool enable);
private:
    SixAxisPagePresenter();

    SixAxisPageView& view;
};

#endif // SIXAXISPAGEPRESENTER_HPP
