#include "../../include/smartwin/widgets/WidgetMDIParent.h"

namespace SmartWin {

const WidgetMDIParent::Seed & WidgetMDIParent::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, _T( "MDICLIENT" ) );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL;
		d_DefaultValues.exStyle = WS_EX_CLIENTEDGE;
		d_DefaultValues.idFirstChild = 0;
		d_DefaultValues.windowMenu = NULL;
		//TODO: initialize the values here
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetMDIParent::create( const Seed & cs )
{
	CLIENTCREATESTRUCT ccs;
	ccs.hWindowMenu = cs.windowMenu;
	ccs.idFirstChild = cs.idFirstChild;
	
	this->Widget::itsHandle = ::CreateWindowEx( cs.exStyle,
		cs.getClassName().c_str(),
		cs.caption.c_str(),
		cs.style,
		cs.location.pos.x, cs.location.pos.y, cs.location.size.x, cs.location.size.y,
		this->Widget::itsParent ? this->Widget::itsParent->handle() : 0,
		NULL,
		Application::instance().getAppHandle(),
		reinterpret_cast< LPVOID >( &ccs ) );
	if ( !this->Widget::itsHandle )
	{
		// The most common error is to forget WS_CHILD in the styles
		xCeption x( _T( "CreateWindowEx in Widget::create fizzled ..." ) );
		throw x;
	}
	this->Widget::isChild = ( ( cs.style & WS_CHILD ) == WS_CHILD );
	Application::instance().registerWidget( this );

	ThisType::createMessageMap();
}

}
