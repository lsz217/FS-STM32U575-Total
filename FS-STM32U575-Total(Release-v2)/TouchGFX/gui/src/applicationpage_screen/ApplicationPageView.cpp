#include <gui/applicationpage_screen/ApplicationPageView.hpp>
#include <texts\TextKeysAndLanguages.hpp>
#include <images/BitmapDatabase.hpp>


ApplicationPageView::ApplicationPageView()
{

}

void ApplicationPageView::setupScreen()
{
    ApplicationPageViewBase::setupScreen();
}

void ApplicationPageView::tearDownScreen()
{
    ApplicationPageViewBase::tearDownScreen();
}

void ApplicationPageView::Fanswitch()
{
	presenter->setDCFanStatus(openFAN.getState());
	
}
//振动电机操作
void ApplicationPageView::turnMotor()
{
		presenter->turnMotorStatus();	//振动电机状态的翻转	
}
//进入数码管设置界面
void ApplicationPageView::openModalNixieTube()
{
	//显示数码管设置界面
	modalNixieTube.show();
	//关闭APP页面的数据读取任务
  presenter->ApplicationPagePresenterState(false);	//关闭APP页面的电压、电流、温度、湿度与光照度的读取
	//数码管显示初始值
	this->NixieTubeDisplay(gshowValue,gshiftCount);
	//打开数码管显示
	presenter->NixieTubeTaskState(true);
}
//退出数码管设置界面
void ApplicationPageView::closedModalNixieTube()
{
	//关闭数码管显示
	presenter->NixieTubeTaskState(false);
	//隐藏数码管显示界面
	modalNixieTube.hide();
	//恢复APP页面的数据读取任务
  presenter->ApplicationPagePresenterState(true);	//恢复APP页面的电压、电流、温度、湿度与光照度的读取
}
//选中位数码管显示+
void ApplicationPageView::addNixieTubeVal()
{
	uint8_t gshiftValue = 0;	//操作值
	//通过Presenter获取gshowValue的初始值
	gshiftValue = (gshowValue >> gshiftCount) & 0x000F;	
	gshiftValue++;	//数据加
	//值判断
	if(gshiftValue > 9)
	{
			gshiftValue = 0; 	//数据归零
	}		
	//重新赋值
	gshowValue = gshowValue & (~(0x000F << gshiftCount));
	gshowValue = gshowValue | (gshiftValue << gshiftCount); 
	//选中的图片存储ID(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID~BITMAP_NIXIETUBE_CHECKED_0_54X76_ID+9),注意检查ID需要连号
	//未选中的图片存储ID(BITMAP_NIXIETUBE_0_54X76_ID~BITMAP_NIXIETUBE_0_54X76_ID+9),注意检查ID需要连号
	this->NixieTubeDisplay(gshowValue,gshiftCount);
}
//选中位数码管显示-
void ApplicationPageView::subNixieTubeVal()
{
	uint8_t gshiftValue = 0;	//操作值
	//通过Presenter获取gshowValue的初始值
	gshiftValue = (gshowValue >> gshiftCount) & 0x000F;	
	//值判断
	if(gshiftValue == 0)
	{
			gshiftValue = 9; 	//数据归零
	}	
	else
			gshiftValue--;	//数据减		
	//重新赋值
	gshowValue = gshowValue & (~(0x000F << gshiftCount));
	gshowValue = gshowValue | (gshiftValue << gshiftCount); 
	//选中的图片存储ID(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID~BITMAP_NIXIETUBE_CHECKED_0_54X76_ID+9),注意检查ID需要连号
	//未选中的图片存储ID(BITMAP_NIXIETUBE_0_54X76_ID~BITMAP_NIXIETUBE_0_54X76_ID+9),注意检查ID需要连号
	this->NixieTubeDisplay(gshowValue,gshiftCount);
}
//数码管个位被选中
void ApplicationPageView::selectedNixieTubeUnit()
{
		gshiftCount = 0;	//移位值为零
		this->NixieTubeDisplay(gshowValue,gshiftCount);
}
//数码管十位被选中
void ApplicationPageView::selectedNixieTubeTen()
{
		gshiftCount = 4;	//移位值为零
		this->NixieTubeDisplay(gshowValue,gshiftCount);
}
//数码管百位被选中
void ApplicationPageView::selectedNixieTubeHundred()
{
		gshiftCount = 8;	//移位值为零
		this->NixieTubeDisplay(gshowValue,gshiftCount);
}
//数码管千位被选中
void ApplicationPageView::selectedNixieTubeThousand()
{
		gshiftCount = 12;	//移位值为零
		this->NixieTubeDisplay(gshowValue,gshiftCount);
}
//刷新显示
void ApplicationPageView::NixieTubeDisplay(uint16_t pshowValue,uint8_t pshiftCount)
{
		//先用未选中的图标进行设置
		NixieTubeUnit.setBitmaps(touchgfx::Bitmap(BITMAP_NIXIETUBE_0_54X76_ID + (pshowValue & 0x000F)),touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID +  (pshowValue & 0x000F)));
		NixieTubeTen.setBitmaps(touchgfx::Bitmap(BITMAP_NIXIETUBE_0_54X76_ID + ((pshowValue >> 4) & 0x000F)),touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID +  ((pshowValue >> 4) & 0x000F)));
		NixieTubeHundred.setBitmaps(touchgfx::Bitmap(BITMAP_NIXIETUBE_0_54X76_ID + ((pshowValue >> 8) & 0x000F)),touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID +  ((pshowValue >> 8) & 0x000F)));
		NixieTubeThousand.setBitmaps(touchgfx::Bitmap(BITMAP_NIXIETUBE_0_54X76_ID + ((pshowValue >> 12) & 0x000F)),touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID +  ((pshowValue >> 12) & 0x000F)));
		//用选中的图标覆盖设置
		if(pshiftCount == 0) {
				NixieTubeUnit.setBitmaps(touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID + (pshowValue & 0x000F)),touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID +  (pshowValue & 0x000F)));
		}
		else if(pshiftCount == 4) {
				NixieTubeTen.setBitmaps(touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID + ((pshowValue >> 4) & 0x000F)),touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID +  ((pshowValue >> 4) & 0x000F)));

		}
		else if(pshiftCount == 8) {
				NixieTubeHundred.setBitmaps(touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID + ((pshowValue >> 8) & 0x000F)),touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID +  ((pshowValue >> 8) & 0x000F)));
		}
		else if(pshiftCount == 12) {
				NixieTubeThousand.setBitmaps(touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID + ((pshowValue >> 12) & 0x000F)),touchgfx::Bitmap(BITMAP_NIXIETUBE_CHECKED_0_54X76_ID +  ((pshowValue >> 12) & 0x000F)));
		}
		//显示
		NixieTubeUnit.invalidate();	//个位显示
		NixieTubeTen.invalidate();	//十位显示
		NixieTubeHundred.invalidate();	//百位显示
		NixieTubeThousand.invalidate();	//千位显示
		//通过Presenter传递gshowValue值到硬件显示
		presenter->setNixieTube(gshowValue);	//设置显示值
	}
//更新电压、电流、温湿度、光照度数据
void ApplicationPageView::updateAppPageInfo(uint16_t CurrentVal, uint16_t VoltageVal, float newHum, float newTem, uint16_t newALS)
{
		//更新电流
		Unicode::snprintfFloat(textCurrentBuffer, TEXTCURRENT_SIZE, "%.3f",((CurrentVal)*3.3f/4095)/10);//转换mA->A
	  textCurrent.invalidate();
		//更新电压
		Unicode::snprintfFloat(textVoltageBuffer, TEXTVOLTAGE_SIZE, "%.3f",(VoltageVal)*3.3f/4095);
	  textVoltage.invalidate();
		//更新湿度
		Unicode::snprintfFloat(textHumidityBuffer, TEXTHUMIDITY_SIZE, "%.1f",newHum);
	  textHumidity.invalidate();
		//更新温度
		Unicode::snprintfFloat(textTempBuffer, TEXTTEMP_SIZE, "%.1f",newTem);
	  textTemp.invalidate();
	//更新光强度
		Unicode::snprintf(textALSBuffer, TEXTALS_SIZE, "%05d",newALS);
	  textALS.invalidate();
}
//健康监测信息上传
void ApplicationPageView::updateHeartRateInfo(uint32_t newHeartRate, uint32_t newSPO2)
{
		//更新脉搏
		Unicode::snprintf(textPulseBuffer, TEXTPULSE_SIZE, "%d",newHeartRate);
	  textPulse.invalidate();
		//更新血氧饱和度
		Unicode::snprintf(textSPOZBuffer, TEXTSPOZ_SIZE, "%d",newSPO2);
	  textSPOZ.invalidate();
		//测量提示-重新测量
		textInfo.setTypedText(TypedText(T_TEXTINFO1));
		textInfo.invalidate();
}
//启动健康检测
void ApplicationPageView::beginHeartRate()
{
	//使能健康监测任务
	presenter->HeartRateTaskState(true);	//健康监测任务使能
	//测量提示-信息读取中
	textInfo.setTypedText(TypedText(T_TEXTINFO2));
	textInfo.invalidate();
}
//打开健康检测页面
void ApplicationPageView::openModalHeartRate()
{
	//关闭APP页面的数据读取任务
  presenter->ApplicationPagePresenterState(false);	//关闭APP页面的电压、电流、温度、湿度与光照度的读取
	//显示健康检测页面
	modalHeartRate.show();
}
//退出健康检测页面
void ApplicationPageView::closedModalHeartRate()
{
	//关闭健康监测任务
	presenter->HeartRateTaskState(false);	//健康监测任务禁能
	//恢复APP页面的数据读取任务
  presenter->ApplicationPagePresenterState(true);	//恢复APP页面的电压、电流、温度、湿度与光照度的读取
	//隐藏健康检测页面
	modalHeartRate.hide();
}
//更新火焰状态
void ApplicationPageView::updateFireStatus(bool newStatus)
{
		if(newStatus == true){
			flameMonitor.setBitmap(touchgfx::Bitmap(BITMAP_FLAME_54X54_ID));	//发现火情
		}
		else{
			flameMonitor.setBitmap(touchgfx::Bitmap(BITMAP_FLAME_NO_FIRE_54X54_ID));	//没有火情
		}
		flameMonitor.invalidate();
}