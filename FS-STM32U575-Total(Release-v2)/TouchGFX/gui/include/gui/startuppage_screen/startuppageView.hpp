#ifndef STARTUPPAGEVIEW_HPP
#define STARTUPPAGEVIEW_HPP

#include <gui_generated/startuppage_screen/startuppageViewBase.hpp>
#include <gui/startuppage_screen/startuppagePresenter.hpp>

class startuppageView : public startuppageViewBase
{
public:
    startuppageView();
    virtual ~startuppageView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void function5();
			void goToSleep();
			
 void updateNfcStatus() ; // <--- 添加这一行声明
protected:
    // 3. 定义一个计数器变量
    int bootDelayCounter;
protected:
};

#endif // STARTUPPAGEVIEW_HPP
