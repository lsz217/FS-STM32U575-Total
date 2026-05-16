#include <gui/fivekeypage_screen/FiveKeyPageView.hpp>
#include <images/BitmapDatabase.hpp>
FiveKeyPageView::FiveKeyPageView()
{

}

void FiveKeyPageView::setupScreen()
{
    FiveKeyPageViewBase::setupScreen();
}

void FiveKeyPageView::tearDownScreen()
{
    FiveKeyPageViewBase::tearDownScreen();
}
//更新五向键数据
void FiveKeyPageView::updateFiveKey(uint16_t newFiveKeyVal)
{
		//图片显示
		if(newFiveKeyVal > 0 && newFiveKeyVal< 320){
			FiveKeyImage.setBitmap(touchgfx::Bitmap(BITMAP_FIVE_KEYOK_200X200_ID));		//0.00V-0.25V
		} 
		else if(newFiveKeyVal == 0){
			FiveKeyImage.setBitmap(touchgfx::Bitmap(BITMAP_FIVE_KEYOK_200X200_ID));		//0.00V-0.25V
		} 
		else if(newFiveKeyVal > 320 && newFiveKeyVal< 960){
			FiveKeyImage.setBitmap(touchgfx::Bitmap(BITMAP_FIVE_KEYLEFT_200X200_ID));	//0.25V-0.75V
		}
		else if(newFiveKeyVal > 960 && newFiveKeyVal< 1600){
			FiveKeyImage.setBitmap(touchgfx::Bitmap(BITMAP_FIVE_KEYUP_200X200_ID));	//0.725V-1.25V
		}
		else if(newFiveKeyVal > 1600 && newFiveKeyVal< 2240){
			FiveKeyImage.setBitmap(touchgfx::Bitmap(BITMAP_FIVE_KEYRIGHT_200X200_ID));	//1.25V-1.75V
		}
		else if(newFiveKeyVal > 2240 && newFiveKeyVal< 2880){
			FiveKeyImage.setBitmap(touchgfx::Bitmap(BITMAP_FIVE_KEYDOWN_200X200_ID));	//1.75V-2.25V
		}
		else {
			FiveKeyImage.setBitmap(touchgfx::Bitmap(BITMAP_FIVE_KEYNOPPRESS_200X200_ID));	//2.25V-3.30V
		}
		FiveKeyBackground.invalidate();
		//显示电压值
		Unicode::snprintf(textFiveKeyValBuffer1, TEXTFIVEKEYVALBUFFER1_SIZE, "%d",((newFiveKeyVal)*33000/4095)/10000);
		Unicode::snprintf(textFiveKeyValBuffer2, TEXTFIVEKEYVALBUFFER2_SIZE, "%04d", ((newFiveKeyVal)*33000/4095)%10000);
	  textFiveKeyVal.invalidate();
		//
}