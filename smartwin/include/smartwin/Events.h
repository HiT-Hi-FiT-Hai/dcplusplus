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

/// Mouse Event structure
/** Several event handlers supply an object of this type as one or more parameters to
  * their Event Handler. <br>
  * E.g. the "onLeftMouseUp" Event Handler takes an object of this type to give
  * extensive information regarding the Event.
  */
struct MouseEvent {
	MouseEvent(const MSG& msg);
	
	/// Types of buttons
	enum Button {
		OTHER, LEFT, RIGHT, MIDDLE, X1, X2
	};

	/// Position of mouse
	/** Position of mouse when event was raised
	  */
	ScreenCoordinate pos;

	/// is the CTRL key pressed
	/** true if CTRL key is pressed, otherwise false
	  */
	bool isControlPressed;

	/// is the SHIFT key pressed
	/** true if SHIFT key is pressed, otherwise false
	  */
	bool isShiftPressed;

	/// is the ALT key pressed
	/** true if ALT key is pressed, otherwise false
	  */
	bool isAltPressed;

	/// Indicates which mouse button was actually pressed
	Button ButtonPressed;
};


}

#endif /*EVENTS_H_*/
