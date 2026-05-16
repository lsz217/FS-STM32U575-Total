#include <gui/sixaxispage_screen/SixAxisPageView.hpp>

SixAxisPageView::SixAxisPageView()
{

}

void SixAxisPageView::setupScreen()
{
    SixAxisPageViewBase::setupScreen();
}

void SixAxisPageView::tearDownScreen()
{
    SixAxisPageViewBase::tearDownScreen();
}
//更新欧拉角数据
void SixAxisPageView::updateSixAxis(float newPitch,float newRoll,float newYaw)
{
		//更新芯片内部参数
		Unicode::snprintfFloat(textAreaPitchBuffer, TEXTAREAPITCH_SIZE, "%.1f",newPitch);
	  textAreaPitch.invalidate();
		//更新芯片内部参数
		Unicode::snprintfFloat(textAreaRollBuffer, TEXTAREAROLL_SIZE, "%.1f",newRoll);
	  textAreaRoll.invalidate();
		//更新芯片内部参数
		Unicode::snprintfFloat(textAreaYawBuffer, TEXTAREAYAW_SIZE, "%.1f",newYaw);
	  textAreaYaw.invalidate();
}