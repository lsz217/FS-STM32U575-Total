#include <gui/sixaxispage_screen/SixAxisPageView.hpp>
#include <gui/sixaxispage_screen/SixAxisPagePresenter.hpp>

SixAxisPagePresenter::SixAxisPagePresenter(SixAxisPageView& v)
    : view(v)
{

}

void SixAxisPagePresenter::activate()
{
		SixAxisPagePresenterState(true);	
}

void SixAxisPagePresenter::deactivate()
{
		SixAxisPagePresenterState(false);	
}
//更新欧拉角数据
void SixAxisPagePresenter::updateSixAxis(float newPitch,float newRoll,float newYaw)
{
    view.updateSixAxis(newPitch, newRoll, newYaw);
}
//SixAxisPagePresenter状态
void SixAxisPagePresenter::SixAxisPagePresenterState(bool enable)
{
	if(enable == true)
		model->SixAxisPageViewTask(true);
	else
		model->SixAxisPageViewTask(false);
}