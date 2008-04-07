#include "../include/smartwin/Events.h"

namespace SmartWin {

SizedEvent::SizedEvent( const MSG& msg ) :
	size(Point::fromLParam(msg.lParam)),
	isMaximized(msg.wParam == SIZE_MAXIMIZED),
	isMinimized(msg.wParam == SIZE_MINIMIZED),
	isRestored(msg.wParam == SIZE_RESTORED)	
{
}

MouseEvent::MouseEvent(const MSG& msg) : pos(Point::fromLParam(msg.lParam)) {
	WPARAM wP = msg.wParam;

	isShiftPressed = ( ( wP & MK_SHIFT ) == MK_SHIFT );
	::ClientToScreen(msg.hwnd, &pos.getPoint());
	
	
	// These might be an issue when porting to Windows CE since CE does only support LEFT (or something...)
	ButtonPressed = (
		MK_LBUTTON & wP ? MouseEvent::LEFT : (
			MK_RBUTTON & wP ? MouseEvent::RIGHT : (
				MK_MBUTTON & wP ? MouseEvent::MIDDLE : (
					MK_XBUTTON1 & wP ? MouseEvent::X1 : (
						MK_XBUTTON2 & wP ? MouseEvent::X2 : MouseEvent::OTHER
					)
				)
			)
		)
	);
}

}

