#include "../../include/smartwin/widgets/WidgetWindow.h"

namespace SmartWin {

WidgetWindow::Seed::Seed() : 
	Widget::Seed(NULL, WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN),
	background(( HBRUSH )( COLOR_APPWORKSPACE + 1 )),
	menuName(NULL),
	cursor(NULL)
{
}

void WidgetWindow::createWindow( Seed cs )
{
	windowClass.reset(new WindowClass(WindowClass::getNewClassName(this), &ThisType::wndProc, cs.menuName, cs.background, cs.icon, cs.smallIcon, cs.cursor));
	cs.className = windowClass->getClassName();
	BaseType::create( cs );
}

}
