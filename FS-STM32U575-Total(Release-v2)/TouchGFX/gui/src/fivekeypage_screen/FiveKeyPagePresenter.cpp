#include <gui/fivekeypage_screen/FiveKeyPageView.hpp>
#include <gui/fivekeypage_screen/FiveKeyPagePresenter.hpp>

FiveKeyPagePresenter::FiveKeyPagePresenter(FiveKeyPageView& v)
    : view(v)
{

}

void FiveKeyPagePresenter::activate()
{
		FiveKeyPagePresenterState(true);	
}

void FiveKeyPagePresenter::deactivate()
{
		FiveKeyPagePresenterState(false);	
}

//更新五向键数据
void FiveKeyPagePresenter::updateFiveKey(uint16_t newFiveKeyVal)
{
    view.updateFiveKey(newFiveKeyVal);
}


//FiveKeyPagePresenter状态
void FiveKeyPagePresenter::FiveKeyPagePresenterState(bool enable)
{
	if(enable == true)
		model->FiveKeyPageViewTask(true);
	else
		model->FiveKeyPageViewTask(false);
}
