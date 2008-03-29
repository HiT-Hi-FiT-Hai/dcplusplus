#include "../../include/smartwin/widgets/MDIFrame.h"
#include "../../include/smartwin/WidgetCreator.h"
#include "../../include/smartwin/widgets/MDIParent.h"

namespace SmartWin {

MDIFrame::Seed::Seed() :
	Widget::Seed(NULL, WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN),
	background(( HBRUSH )( COLOR_APPWORKSPACE + 1 )),
	menuName(NULL),
	cursor(NULL)
{
}

void MDIFrame::createInvisibleWindow( Seed cs )
{
	cs.style=  cs.style & ( ~ WS_VISIBLE );
	MDIFrame::createWindow( cs );
}

void MDIFrame::createWindow( Seed cs )
{
	windowClass.reset(new WindowClass(WindowClass::getNewClassName(this), &ThisType::wndProc, cs.menuName, cs.background, cs.icon, cs.iconSmall, cs.cursor));
	cs.className = windowClass->getClassName();
	Widget::create( cs );
	
	mdi = WidgetCreator<MDIParent>::create(this);
}

}
