#include "../../include/smartwin/widgets/ToolTip.h"

namespace SmartWin {

ToolTip::Seed::Seed() : 
	Widget::Seed(TOOLTIPS_CLASS, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX, WS_EX_TRANSPARENT)
{
}

void ToolTip::create( const Seed & cs )
{
	xAssert((cs.style & WS_POPUP) == WS_POPUP, _T("Widget must have WS_POPUP style"));

	PolicyType::create(cs);
}

void ToolTip::relayEvent(const MSG& msg) {
	if(msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
		sendMessage(TTM_RELAYEVENT, 0, reinterpret_cast<LPARAM>(&msg));
}

void ToolTip::setTool(Widget* widget, const Dispatcher::F& f) {
	addCallback(
		Message(WM_NOTIFY, TTN_GETDISPINFO), Dispatcher(f)
	);
	
	TOOLINFO ti = { sizeof(TOOLINFO) };
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.hwnd = getParent()->handle();
	ti.uId = reinterpret_cast<UINT_PTR>(widget->handle());
	ti.lpszText = LPSTR_TEXTCALLBACK;
	sendMessage(TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
	
}

}
