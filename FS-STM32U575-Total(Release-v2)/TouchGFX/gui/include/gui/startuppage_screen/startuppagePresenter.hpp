#ifndef STARTUPPAGEPRESENTER_HPP
#define STARTUPPAGEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class startuppageView;

class startuppagePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    startuppagePresenter(startuppageView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate()override;

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate()override;

    virtual ~startuppagePresenter() {};
        // 如果你想让 Presenter 处理逻辑，在这里加一个函数
    void buttonPressed();

			    void setBacklightValue(uint8_t value); 
			// 加上这个虚函数重写
virtual void notifySleep() override;
    void resetIdleTimer();
			virtual void nfcTriggered() override;
	virtual void navigateToHome() override;

private:
    startuppagePresenter();

    startuppageView& view;
};

#endif // STARTUPPAGEPRESENTER_HPP
