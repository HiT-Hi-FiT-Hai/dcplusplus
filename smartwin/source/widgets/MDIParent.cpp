#include "../../include/smartwin/widgets/MDIParent.h"
#include "../../include/smartwin/Application.h"

namespace SmartWin {

MDIParent::Seed::Seed() : 
	Widget::Seed(_T("MDICLIENT"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL, WS_EX_CLIENTEDGE),
	idFirstChild(0),
	windowMenu(NULL)
{
}

void MDIParent::create( const Seed & cs )
{
	CLIENTCREATESTRUCT ccs;
	ccs.hWindowMenu = cs.windowMenu;
	ccs.idFirstChild = cs.idFirstChild;
	
	HWND wnd = ::CreateWindowEx( cs.exStyle,
		cs.className,
		cs.caption.c_str(),
		cs.style,
		cs.location.x(), cs.location.y(), cs.location.width(), cs.location.height(),
		this->getParent() ? this->getParent()->handle() : 0,
		NULL,
		Application::instance().getAppHandle(),
		reinterpret_cast< LPVOID >( &ccs ) );
	if ( !wnd )
	{
		// The most common error is to forget WS_CHILD in the styles
		throw xCeption( _T( "CreateWindowEx in Widget::create fizzled ..." ) );
	}
}

}
