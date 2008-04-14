/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  SmartWin++

  Copyright (c) 2005 Thomas Hansen

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor SmartWin++ nor the names of its contributors 
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

#ifndef DWT_AspectMouse_h
#define DWT_AspectMouse_h

#include "../Events.h"
#include "../Message.h"
#include "../Dispatchers.h"

namespace dwt {

/// Aspect class used by Widgets that have the possibility of trapping "mouse
/// clicked" events.
/** \ingroup AspectClasses
  * E.g. the Window can trap "mouse clicked events" therefore it realize the
  * AspectMouse through inheritance.
  */
template< class WidgetType >
class AspectMouse
{
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	
	typedef Dispatchers::ConvertBase<MouseEvent> Dispatcher;
public:
	/// \ingroup EventHandlersAspectMouse
	/// Left mouse button pressed and released event handler setter
	/** If supplied, function will be called when user releases the Left Mouse button
	  * after clicking onto the client area of the Widget. <br>
	  * The parameter passed is const MouseEvent & which contains the state of
	  * the mouse.
	  */
	void onLeftMouseUp(const typename Dispatcher::F& f) {
		onMouse(WM_LBUTTONUP, f);
	}
	
	/// \ingroup EventHandlersAspectMouse
	/// Right mouse button pressed and released event handler setter
	/** If supplied, function will be called when user releases the Right Mouse
	  * button after clicking onto the client area of the Widget. <br>
	  * The parameter passed is const MouseEvent & which contains the state of
	  * the mouse.
	  */
	void onRightMouseUp(const typename Dispatcher::F& f) {
		onRightMouseUp(WM_RBUTTONUP, f);
	}

	/// \ingroup EventHandlersAspectMouse
	/// Middle mouse button pressed and released event handler setter
	/** If supplied, function will be called when user releases the middle Mouse
	  * button after clicking onto the client area of the Widget. <br>
	  * The parameter passed is const MouseEvent & which contains the state of
	  * the mouse.
	  */
	void onMiddleMouseUp(const typename Dispatcher::F& f) {
		onMiddleMouseUp(WM_MBUTTONUP, f);
	}

	/// \ingroup EventHandlersAspectMouse
	/// Left mouse button pressed event handler setter
	/** If supplied, function will be called when user press the Left Mouse button in
	  * the client area of the widget. <br>
	  * The parameter passed is const MouseEvent & which contains the state of
	  * the mouse.
	  */
	void onLeftMouseDown(const typename Dispatcher::F& f) {
		onMouse(WM_LBUTTONDOWN, f);
	}

	/// \ingroup EventHandlersAspectMouse
	/// Right mouse button pressed event handler setter
	/** If supplied, function will be called when user press the Right Mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEvent & which contains the state of
	  * the mouse.
	  */
	void onRightMouseDown(const typename Dispatcher::F& f) {
		onMouse(WM_RBUTTONDOWN, f);
	}

	/// Middle mouse button pressed event handler setter
	/** If supplied, function will be called when user press the Middle Mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEvent & which contains the state of
	  * the mouse.
	  */
	void onMiddleMouseDown(const typename Dispatcher::F& f) {
		onMouse(WM_MBUTTONDOWN, f);
	}

	/// Left mouse button double-clicked event handler setter
	/** If supplied, function will be called when user double clicks the Left mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEvent & which contains the state of
	  * the mouse.
	  */
	void onLeftMouseDblClick(const typename Dispatcher::F& f) {
		onMouse(WM_LBUTTONDBLCLK, f);
	}

	/// Right mouse button double-clicked event handler setter
	/** If supplied, function will be called when user  double clicks the Right mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEvent & which contains the state of
	  * the mouse.
	  */
	void onRightMouseDblClick(const typename Dispatcher::F& f) {
		onMouse(WM_RBUTTONDBLCLK, f);
	}

	/// \ingroup EventHandlersAspectMouse
	/// Mouse moved event handler setter
	/** If supplied, function will be called when user moves the mouse. <br>
	  * The parameter passed is const MouseEvent & which contains the state of
	  * the mouse.
	  */
	void onMouseMove(const typename Dispatcher::F& f) {
		onMouse(WM_MOUSEMOVE, f);
	}

protected:
	
	void onMouse(UINT msg, const typename Dispatcher::F& f) {
		W().addCallback(Message( msg ), Dispatcher(f));
	}
	
	virtual ~AspectMouse() { }
};

}

#endif
