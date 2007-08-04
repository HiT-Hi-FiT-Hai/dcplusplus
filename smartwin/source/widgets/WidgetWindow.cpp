#include "../../include/smartwin/widgets/WidgetWindow.h"

namespace SmartWin {

const WidgetWindow::Seed & WidgetWindow::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.style = WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
		d_DefaultValues.background = ( HBRUSH )( COLOR_APPWORKSPACE + 1 );
		d_DefaultValues.caption = _T( "" );
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

void WidgetWindow::createWindow( Seed cs )
{
	windowClass.reset(new WindowClass(WindowClass::getNewClassName(this), &ThisType::wndProc, cs.menuName, cs.background, cs.icon, cs.smallIcon, cs.cursor));
	cs.className = windowClass->getClassName();
	Widget::create( cs );
}

}
