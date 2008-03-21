#include "../../include/smartwin/widgets/WidgetMDIChild.h"

namespace SmartWin {

WidgetMDIChild::Seed::Seed(const SmartUtil::tstring& caption) :
	Widget::Seed(NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_MDICHILD, caption),
	background((HBRUSH)(COLOR_WINDOW + 1)),
	activate(true)
{
}

void WidgetMDIChild::createMDIChild( Seed cs )
{
	windowClass.reset(new WindowClass(WindowClass::getNewClassName(this), &ThisType::wndProc, NULL, cs.background, cs.icon, cs.smallIcon));
	
	getParent()->sendMessage(WM_SETREDRAW, FALSE);
	HWND active = (HWND)(cs.activate ? NULL : getParent()->sendMessage(WM_MDIGETACTIVE));
	HWND wnd = ::CreateMDIWindow( windowClass->getClassName(),
		cs.caption.c_str(),
		cs.style,
		cs.location.x(), cs.location.y(), cs.location.width(), cs.location.height(),
		getParent()->handle(),
		Application::instance().getAppHandle(),
		reinterpret_cast< LPARAM >( static_cast< Widget * >( this ) ) );
	
	if(active) {
		getParent()->sendMessage(WM_MDIACTIVATE, (WPARAM)active);
	}
	
	getParent()->sendMessage(WM_SETREDRAW, TRUE);
	invalidateWidget();
	if ( !wnd )
	{
		xCeption x( _T( "CreateWindowEx in WidgetMDIChild::createMDIChild fizzled..." ) );
		throw x;
	}
}

bool WidgetMDIChild::tryFire(const MSG& msg, LRESULT& retVal) {
	// Prevent some flicker...
    if(msg.message == WM_NCPAINT || msg.message == WM_SIZE)
    {
	    if(getParent()->isActiveMaximized()) {
		    if(msg.message == WM_NCPAINT) // non client area
		    return true;

		    if(msg.message == WM_SIZE) // client area
		    {
			    if((msg.wParam == SIZE_MAXIMIZED || msg.wParam == SIZE_RESTORED) && getParent()->getActive() == handle()) // active and maximized
			    	return BaseType::tryFire(msg, retVal);

			    sendMessage(WM_SETREDRAW, FALSE);
			    bool ret = BaseType::tryFire(msg, retVal);
			    sendMessage(WM_SETREDRAW, TRUE);
			    return ret;
		    }
	    }
    }
    return BaseType::tryFire(msg, retVal);
}

void WidgetMDIChild::activate() {
	HWND prev = getParent()->getActive();
	if(prev == handle())
		return;
	
	if(::IsIconic(handle())) {
		getParent()->sendMessage(WM_MDIRESTORE, reinterpret_cast<WPARAM>(this->handle()));
	}
	getParent()->sendMessage(WM_MDIACTIVATE, reinterpret_cast<WPARAM>(this->handle()));
}

}
