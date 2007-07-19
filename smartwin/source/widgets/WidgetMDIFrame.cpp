#include "../../include/smartwin/widgets/WidgetMDIFrame.h"
#include "../../include/smartwin/WidgetCreator.h"

namespace SmartWin {

const WidgetMDIFrame::Seed & WidgetMDIFrame::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
		d_DefaultValues.background = ( HBRUSH )( COLOR_APPWORKSPACE + 1 );
		d_DefaultValues.caption = _T( "" );
#ifndef WINCE
		d_DefaultValues.cursor = NULL;
		d_DefaultValues.icon = NULL;
#else
		d_DefaultValues.cursor = 0;
		d_DefaultValues.icon = 0;
#endif
		d_DefaultValues.menuName = _T( "" ); //TODO: does menu &"" work as good as menu NULL ?

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
	Application::instance().generateLocalClassName( cs );
	itsRegisteredClassName = cs.getClassName();

	SMARTWIN_WNDCLASSEX ws;

#ifndef WINCE
	ws.cbSize = sizeof( SMARTWIN_WNDCLASSEX );
#endif //! WINCE
	// This are window class styles, not window styles ...
	ws.style = CS_DBLCLKS;	// Allow double click messages
	ws.lpfnWndProc = &ThisType::wndProc;
	ws.cbClsExtra = 0;
	ws.cbWndExtra = 0;
	ws.hInstance = Application::instance().getAppHandle();
#ifdef WINCE
	ws.hIcon = 0;
#else
	ws.hIcon = cs.icon;
#endif //! WINCE
	ws.hCursor = cs.cursor;
	ws.hbrBackground = cs.background;
	ws.lpszMenuName = cs.menuName.empty() ? 0 : cs.menuName.c_str();
	ws.lpszClassName = itsRegisteredClassName.c_str();
#ifndef WINCE
	//TODO: fix this
	ws.hIconSm = cs.icon;
#endif //! WINCE

	ATOM registeredClass = SmartWinRegisterClass( & ws );
	if ( 0 == registeredClass )
	{
		xCeption x( _T( "WidgetMDIFrameBase.createWindow() SmartWinRegisterClass fizzled..." ) );
		throw x;
	}
	Application::instance().addLocalWindowClassToUnregister( cs );
	Widget::create( cs );
	
	mdi = WidgetCreator<WidgetMDIParent>::create(this);
}

}
