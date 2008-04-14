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

#ifndef DWT_AspectFocus_h
#define DWT_AspectFocus_h

#include "../Message.h"
#include "../Dispatchers.h"

namespace dwt {

/// Aspect class used by Widgets that have the possibility of retrieving the focus
/** \ingroup AspectClasses
  * E.g. the Table have a Focus Aspect to it therefore Table
  * realize the AspectFocus through inheritance. This Aspect is closely related to
  * the AspectActivate and when a Widget is being activated it means that it is the
  * "active" Widget meaning that it receives keyboard input for one and normally if
  * it is a text Widget gets to own the carret.
  */
template< class WidgetType >
class AspectFocus
{
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	HWND H() { return W().handle(); }
	
	typedef Dispatchers::VoidVoid<0, false> FocusDispatcher;
	typedef Dispatchers::VoidVoid<0, true> KillFocusDispatcher;

public:
	/// Gives the Widget the keyboard focus
	/** Use this function if you wish to give the Focus to a specific Widget
	  */
	void setFocus();

	/// Retrieves the focus property of the Widget
	/** Use this function to check if the Widget has focus or not. If the Widget has
	  * focus this function will return true.
	  */
	bool getFocus() const;

	/// \ingroup EventHandlersAspectAspectFocus
	/// Sets the event handler for what function to be called when control loses focus.
	/** This function will be called just after the Widget is losing focus and just
	  * before the other Widget which is supposed to get focus retrieves it. No
	  * parameters are passed.
	  */
	void onKillFocus(const typename KillFocusDispatcher::F& f) {
		W().addCallback(Message( WM_KILLFOCUS ), KillFocusDispatcher(f));
	}

	/// \ingroup EventHandlersAspectAspectFocus
	/// Sets the event handler for what function to be called when control loses focus.
	/** This function will be called just after the Widget has gained focus. No
	  * parameters are passed.
	  */	
	void onFocus(const typename FocusDispatcher::F& f) {
		W().addCallback(Message( WM_SETFOCUS ), FocusDispatcher(f));
	}

protected:
	virtual ~AspectFocus() { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class WidgetType >
void AspectFocus< WidgetType >::setFocus()
{
	::SetFocus( H() );
}

template< class WidgetType >
bool AspectFocus< WidgetType >::getFocus() const
{
	return ::GetFocus() == H();
}

}

#endif