#include "../include/smartwin/Events.h"

namespace SmartWin {

SizedEvent::SizedEvent( const MSG& msg ) :
	size(Point::fromLParam(msg.lParam)),
	isMaximized(msg.wParam == SIZE_MAXIMIZED),
	isMinimized(msg.wParam == SIZE_MINIMIZED),
	isRestored(msg.wParam == SIZE_RESTORED)	
{
}

}

