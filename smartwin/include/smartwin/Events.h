#ifndef EVENTS_H_
#define EVENTS_H_

#include "WindowsHeaders.h"
#include "Point.h"

namespace SmartWin {

struct SizedEvent {
	SizedEvent(const MSG& msg);
	
	/// Sise
	/** New size of the window
	  */
	Point size;

	/// is window maximized
	/** true if window was being maximized, otherwise false
	  */
	bool isMaximized;

	/// is window minimized
	/** true if window was being minimized, otherwise false
	  */
	bool isMinimized;

	/// is window restored
	/** true if window was being restored, otherwise false
	  */
	bool isRestored;
};

}

#endif /*EVENTS_H_*/
