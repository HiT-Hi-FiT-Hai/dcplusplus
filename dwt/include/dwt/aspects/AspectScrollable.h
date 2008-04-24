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

#ifndef DWT_AspectScrollable_h
#define DWT_AspectScrollable_h

#include "../Dispatchers.h"
#include "../Message.h"
#include "../util/check.h"

namespace dwt {

/// Aspect class used by Widgets that have the possibility of scrolling
/** \ingroup AspectClasses
  * E.g. the Slider have a scroll Aspect to it therefore Slider realize
  * the AspectScrollable through inheritance.
  */
template< class WidgetType >
class AspectScrollable
{
	WidgetType& W() { return *static_cast<WidgetType*>(this); }

	typedef Dispatchers::VoidVoid<> Dispatcher;
public:
	bool scrollIsAtEnd();

	/// \ingroup EventHandlersAspectScrollable
	/// Setting the event handler for the "scrolling horizontally" event
	/** A scrolling event occurs when for instance a Sliders value is being
	  * manipulated through user interaction. Such an action would raise this event.
	  * <br>
	  * No parameters are passed.
	  */
	void onScrollHorz(const Dispatcher::F& f) {
		W().addCallback(Message( WM_HSCROLL ), Dispatcher(f));
	}

	/// \ingroup EventHandlersAspectScrollable
	/// Setting the event handler for the "scrolling vertically" event
	/** A scrolling event occurs when for instance a Sliders value is being
	  * manipulated through user interaction. Such an action would raise this event.
	  * <br>
	  * No parameters are passed.
	  */
	void onScrollVert(const Dispatcher::F& f) {
		W().addCallback(Message( WM_VSCROLL ), Dispatcher(f));
	}

protected:
	virtual ~AspectScrollable()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class WidgetType >
bool AspectScrollable< WidgetType >::scrollIsAtEnd()
{
	SCROLLINFO scrollInfo = { sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE | SIF_POS };
	BOOL ret = ::GetScrollInfo(static_cast<WidgetType*>(this)->handle(), SB_VERT, &scrollInfo);
	dwtassert(ret != FALSE, _T("Can't get scroll info in scrollIsAtEnd"));
	return (scrollInfo.nPos == static_cast<int>(scrollInfo.nMax - std::max(scrollInfo.nPage - 1, 0u)));
}

}

#endif
