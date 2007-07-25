#include "../../include/smartwin/widgets/WidgetMDIChild.h"

namespace SmartWin {

const WidgetMDIChild::Seed & WidgetMDIChild::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.exStyle = WS_EX_MDICHILD;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE;
		d_DefaultValues.background = ( HBRUSH )( COLOR_WINDOW + 1 );
		d_DefaultValues.icon = NULL;
		d_DefaultValues.smallIcon = NULL;
		//TODO: initialize the values here
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetMDIChild::createMDIChild( Seed cs )
{
	windowClass.reset(new WindowClass(WindowClass::getNewClassName(this), &ThisType::wndProc, NULL, cs.background, cs.icon, cs.smallIcon));
	
	HWND wnd = ::CreateMDIWindow( windowClass->getClassName(),
		cs.caption.c_str(),
		cs.style,
		cs.location.pos.x, cs.location.pos.y, cs.location.size.x, cs.location.size.y,
		getParent()->handle(),
		Application::instance().getAppHandle(),
		reinterpret_cast< LPARAM >( static_cast< Widget * >( this ) ) );
	if ( !wnd )
	{
		xCeption x( _T( "CreateWindowEx in WidgetMDIChild::createMDIChild fizzled..." ) );
		throw x;
	}
	setHandle(wnd);
}

}
