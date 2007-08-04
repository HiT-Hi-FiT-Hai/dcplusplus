#include "../../include/smartwin/widgets/WidgetMDIFrame.h"
#include "../../include/smartwin/WidgetCreator.h"
#include "../../include/smartwin/widgets/WidgetMDIParent.h"

namespace SmartWin {

const WidgetMDIFrame::Seed & WidgetMDIFrame::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
		d_DefaultValues.background = ( HBRUSH )( COLOR_APPWORKSPACE + 1 );
#ifndef WINCE
		d_DefaultValues.cursor = NULL;
#else
		d_DefaultValues.cursor = 0;
#endif
		d_DefaultValues.menuName = NULL;

		d_NeedsInit = false;
	}
	return d_DefaultValues;
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
