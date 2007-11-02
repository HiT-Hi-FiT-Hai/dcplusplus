#include "../../include/smartwin/widgets/WidgetMDIFrame.h"
#include "../../include/smartwin/WidgetCreator.h"
#include "../../include/smartwin/widgets/WidgetMDIParent.h"

namespace SmartWin {

WidgetMDIFrame::Seed::Seed() :
	Widget::Seed(NULL, WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN),
	background(( HBRUSH )( COLOR_APPWORKSPACE + 1 )),
	menuName(NULL),
	cursor(NULL)
{
}

void WidgetMDIFrame::createInvisibleWindow( Seed cs )
{
	cs.style=  cs.style & ( ~ WS_VISIBLE );
	WidgetMDIFrame::createWindow( cs );
}

void WidgetMDIFrame::createWindow( Seed cs )
{
	windowClass.reset(new WindowClass(WindowClass::getNewClassName(this), &ThisType::wndProc, cs.menuName, cs.background, cs.icon, cs.iconSmall, cs.cursor));
	cs.className = windowClass->getClassName();
	Widget::create( cs );
	
	mdi = WidgetCreator<WidgetMDIParent>::create(this);
}

}
