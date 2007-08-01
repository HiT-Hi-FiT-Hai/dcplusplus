#include "../../include/smartwin/widgets/WidgetToolTip.h"

namespace SmartWin {

const WidgetToolTip::Seed & WidgetToolTip::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = TOOLTIPS_CLASS;
		d_DefaultValues.exStyle = WS_EX_TRANSPARENT;
		d_DefaultValues.style = WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX;
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetToolTip::create( const Seed & cs )
{
	xAssert((cs.style & WS_POPUP) == WS_POPUP, _T("Widget must have WS_POPUP style"));

	PolicyType::create(cs);
}

void WidgetToolTip::setTool(Widget* widget, const Dispatcher::F& f) {
	setCallback(
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
