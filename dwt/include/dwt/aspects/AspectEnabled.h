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

#ifndef DWT_AspectEnabled_h
#define DWT_AspectEnabled_h

#include "../Message.h"
#include "../Dispatchers.h"

namespace dwt {

/// Aspect class used by Widgets that have the possibility of changing the enabled
/// property
/** \ingroup AspectClasses
  * The Table has an enabled Aspect to it; therefore it realizes this
  * AspectEnabled through inheritance. <br>
  * When a Widget is enabled it is possible to interact with it in some way, e.g. a
  * button can be pushed, a ComboBox can change value etc. When the Widget is not
  * enabled it cannot change its "value" or be interacted with but is normally still
  * visible.
  */
template< class WidgetType >
class AspectEnabled
{
	WidgetType& W() { return *static_cast<WidgetType*>(this); }

	static bool isEnabled(const MSG& msg) { return msg.wParam > 0; }
	
	typedef Dispatchers::ConvertBase<bool, &AspectEnabled<WidgetType>::isEnabled> Dispatcher;
	friend class Dispatchers::ConvertBase<bool, &AspectEnabled<WidgetType>::isEnabled>;

public:
	/// Sets the enabled property of the Widget
	/** Changes the enabled property of the Widget. Use this function to change the
	  * enabled property of the Widget
	  */
	void setEnabled( bool enabled );

	/// Retrieves the enabled property of the Widget
	/** Use this function to check if the Widget is Enabled or not. If the Widget is
	  * enabled this function will return true.
	  */
	bool getEnabled() const;

	/// \ingroup EventHandlersAspectEnabled
	/// Setting the event handler for the "enabled" event
	/** This event handler will be raised when the enabled property of the Widget is
	  * being changed. <br>
	  * The bool value passed to your Event Handler defines if the widget has just
	  * been enabled or if it has been disabled! <br>
	  * No parameters are passed.
	  */
	void onEnabled(const typename Dispatcher::F& f) {
		W().addCallback(Message( WM_ENABLE ), Dispatcher(f));
	}

protected:
	virtual ~AspectEnabled() { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class WidgetType >
void AspectEnabled< WidgetType >::setEnabled( bool enabled )
{
	::EnableWindow( W().handle(), enabled ? TRUE : FALSE );
}

template< class WidgetType >
bool AspectEnabled< WidgetType >::getEnabled() const
{
	return ::IsWindowEnabled( W().handle() ) != 0;
}

}

#endif
