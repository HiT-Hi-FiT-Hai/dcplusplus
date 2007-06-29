// $Revision: 1.8 $
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
#ifndef AspectRightClickable_h
#define AspectRightClickable_h

#include "AspectVoidVoidDispatcher.h"
#include "../SignalParams.h"
#include "AspectAdapter.h"

namespace SmartWin
{
// begin namespace SmartWin

/// \ingroup AspectClasses
/// Aspect class used by Widgets that have the possibility of being "Right Clicked".
/** E.g. the WidgetStatic have a "Right Clicked" Aspect therefore it realizes the
  * AspectRightClickable through inheritance.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectRightClickable
{
	typedef AspectVoidVoidDispatcher Dispatcher;
	typedef AspectAdapter<Dispatcher::F, EventHandlerClass, MessageMapType::IsControl> Adapter;
public:
	/// \ingroup EventHandlersAspectRightClickable
	/// Setting the event handler for the "Right Clicked" event
	/** All Widgets that realize this Aspect will raise this event when Widget is
	  * being Right Clicked. No parameters are passed.
	  */
	void onRightClicked( typename MessageMapType::itsVoidFunctionTakingVoid eventHandler ) {
		onRightClicked(Adapter::adapt0(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onRightClicked( typename MessageMapType::voidFunctionTakingVoid eventHandler ) {
		onRightClicked(Adapter::adapt0(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}

	void onRightClicked(const Dispatcher::F& f) {
#ifdef WINCE
	static Message msg = Message( WM_NOTIFY, GN_CONTEXTMENU );
#else
	static Message msg = Message( WM_NOTIFY, NM_RCLICK );
#endif
		MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
		ptrThis->setCallback(
			msg, Dispatcher(f)
		);
	}

protected:
	virtual ~AspectRightClickable()
	{}
};

// end namespace SmartWin
}

#endif
