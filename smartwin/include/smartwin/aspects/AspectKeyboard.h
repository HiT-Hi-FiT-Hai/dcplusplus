/*
  Copyright (c) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

	  * Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.
	  * Redistributions in binary form must reproduce the above copyright notice, 
		this list of conditions and the following disclaimer in the documentation 
		and/or other materials provided with the distribution.
	  * Neither the name of the SmartWin++ nor the names of its contributors 
		may be used to endorse or promote products derived from this software 
		without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef AspectKeyboard_h
#define AspectKeyboard_h

namespace SmartWin
{
// begin namespace SmartWin

/** 
 * Base functionality that doesn't depend on template parameters
 */
class AspectKeyboardBase {
public:
	static bool isKeyPressed(int vkey) {
		return (::GetKeyState(vkey) & 0x8000) == 0x8000;
	}
	
	static bool isShiftPressed() { return isKeyPressed(VK_SHIFT); }
	static bool isControlPressed() { return isKeyPressed(VK_CONTROL); }
	static bool isAltPressed() { return isKeyPressed(VK_MENU); }
	
	/// Get ascii character from a Virtual Key
	/** Use this to convert from the input to the response to onKeyPressed to a
	  * character. <br>
	  * Virtual Keys do not take into account the shift status of the keyboard, and
	  * always report UPPERCASE letters.
	  */
	static char virtualKeyToChar( int vkey ) {
		char theChar = 0;
		// Doing Alphabetic keys separately is not needed, but saves some steps.
		if ( ( vkey >= 'A' ) && ( vkey <= 'Z' ) )
		{
			// Left or Right shift key pressed
			bool shifted = 0x8000 == ( 0x8000 & ::GetKeyState( VK_SHIFT ) );
			bool caps_locked = 1 & ::GetKeyState( VK_CAPITAL ); // Caps lock toggled on.

			// The vkey comes as uppercase, if that is desired, leave it.
			if ( ( shifted || caps_locked ) && shifted != caps_locked )
			{
				theChar = vkey;
			}
			else
			{
				theChar = ( vkey - 'A' ) + 'a'; // Otherwise, convert to lowercase
			}
		}
		else
		{
			BYTE keyboardState[256];
			::GetKeyboardState( keyboardState );

			WORD wordchar;
			int retv = ::ToAscii( vkey, ::MapVirtualKey( vkey, 0 ), keyboardState, & wordchar, 0 );
			if ( 1 == retv )
			{
				theChar = wordchar & 0xff;
			}
		}
		return theChar;
	}

	/// Checks if control is pressed
	/** Use this function if you need to determine if any of the CTRL keys are pressed.
	 * @deprecated in favor of isControlPressed
	  */
	static bool getControlPressed() { return isControlPressed(); }

	/// Checks if shift is pressed
	/** Use this function if you need to determine if any of the SHIFT keys are pressed.
	 * @deprecated in favor of isShiftPressed
	  */
	static bool getShiftPressed() { return isShiftPressed(); }

	/// Checks if Caps Lock is on
	/** Use this function if you need to determine if Caps Lock is ON
	  */
	static bool getCapsLockOn() { return 0x1 == ( 0x1 & ::GetKeyState( VK_CAPITAL ) ); }
};

/// Aspect class used by Widgets that have the possibility of trapping keyboard events.
/** \ingroup AspectClasses
  * E.g. the WidgetListView can trap "key pressed events" therefore they realize the AspectKeyboard through inheritance.
  */
template< class WidgetType >
class AspectKeyboard : public AspectKeyboardBase
{
	struct Dispatcher
	{
		typedef std::tr1::function<bool (int)> F;

		Dispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			return f(static_cast< int >( msg.wParam ));
		}

		F f;
	};

public:
	/// \ingroup EventHandlersAspectKeyboard
	/// Setting the event handler for the "key pressed" event
	/** If supplied event handler is called when control has the focus and a key is
	  * being pressed (before it is released) <br>
	  * parameter passed is int which is the virtual-key code of the nonsystem key
	  * being pressed. Return value must be of type bool, if event handler returns
	  * true event is defined as "handled" meaning the system will not try itself to
	  * handle the event.<br>
	  *
	  * Certain widgets, such as WidgetTextBox, will not report VK_RETURN unless you
	  * include ES_WANTRETURN in the style field of of the creational structure
	  * passed when you createTextBox( cs ).
	  *
	  * Use virtualKeyToChar to transform virtual key code to a char, though this
	  * will obviously not work for e.g. arrow keys etc...
	  */
	void onKeyDown(const typename Dispatcher::F& f) {
		onKey(WM_KEYDOWN, f);
	}

	void onChar(const typename Dispatcher::F& f) {
		onKey(WM_CHAR, f);
	}
	
	void onKeyUp(const typename Dispatcher::F& f) {
		onKey(WM_KEYUP, f);
	}

protected:
	virtual ~AspectKeyboard()
	{}
	
	void onKey(UINT msg, const typename Dispatcher::F& f) {
		static_cast<WidgetType*>(this)->setCallback(
			Message( msg ), Dispatcher(f)
		);
	}
};

// end namespace SmartWin
}

#endif
