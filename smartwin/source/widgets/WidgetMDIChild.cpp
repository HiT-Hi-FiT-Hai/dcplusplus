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
	Application::instance().generateLocalClassName( cs );
	itsRegisteredClassName = cs.getClassName();

	//TODO: use CreationalInfo parameters
	SMARTWIN_WNDCLASSEX wc;
	wc.cbSize = sizeof( SMARTWIN_WNDCLASSEX );
	wc.style = 0;
	wc.lpfnWndProc = &ThisType::wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = Application::instance().getAppHandle();
	wc.hIcon = cs.icon;
	wc.hCursor = NULL;
	wc.hbrBackground = cs.background;
	wc.lpszMenuName = 0;
	wc.lpszClassName = itsRegisteredClassName.c_str();
	wc.hIconSm = cs.smallIcon;

	ATOM registeredClass = SmartWinRegisterClass( & wc );
	if ( 0 == registeredClass )
	{
		xCeption x( _T( "WidgetMDIChild::createMDIChild() SmartWinRegisterClass fizzled..." ) );
		throw x;
	}

	this->Widget::itsHandle = ::CreateMDIWindow( itsRegisteredClassName.c_str(),
		cs.caption.c_str(),
		cs.style,
		cs.location.pos.x, cs.location.pos.y, cs.location.size.x, cs.location.size.y,
		this->Widget::itsParent->handle(),
		Application::instance().getAppHandle(),
		reinterpret_cast< LPARAM >( dynamic_cast< Widget * >( this ) ) );
	if ( !this->Widget::itsHandle )
	{
		xCeption x( _T( "CreateWindowEx in WidgetMDIChild::createMDIChild fizzled..." ) );
		throw x;
	}
	Application::instance().addLocalWindowClassToUnregister( cs );
	//::ShowWindow( this->Widget::itsHandle, SW_SHOW );
	//this->Widget::isChild = true;
	this->registerWidget();
}

}
