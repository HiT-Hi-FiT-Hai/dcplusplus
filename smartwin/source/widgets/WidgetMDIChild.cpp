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
		d_DefaultValues.activate = true;
		//TODO: initialize the values here
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetMDIChild::createMDIChild( Seed cs )
{
	windowClass.reset(new WindowClass(WindowClass::getNewClassName(this), &ThisType::wndProc, NULL, cs.background, cs.icon, cs.smallIcon));
	
	getParent()->sendMessage(WM_SETREDRAW, FALSE);
	HWND active = (HWND)(cs.activate ? NULL : getParent()->sendMessage(WM_MDIGETACTIVE));
	HWND wnd = ::CreateMDIWindow( windowClass->getClassName(),
		cs.caption.c_str(),
		cs.style,
		cs.location.pos.x, cs.location.pos.y, cs.location.size.x, cs.location.size.y,
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
	setHandle(wnd);
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
