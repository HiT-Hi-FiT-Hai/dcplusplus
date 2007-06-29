// $Revision: 1.2 $
/*
  Copyright (c) 2006, Thomas Hansen
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
#ifndef AspectChar_h
#define AspectChar_h

#include "../SignalParams.h"
#include "AspectAdapter.h"

namespace SmartWin
{
// begin namespace SmartWin


template<typename EventHandlerClass>
struct AspectCharDispatcher {
	typedef std::tr1::function<bool (int)> F;
	
	AspectCharDispatcher(const F& f_, EventHandlerClass* parent_) : f(f_), parent(parent_) { }
	
	HRESULT operator()(private_::SignalContent& params) {
		bool handled = f(static_cast<int>(params.Msg.WParam));
		if ( handled ) // TODO: Check up this logic
			return parent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
		else
		{
			params.RunDefaultHandling = true;
			return 0;
		}
	}

	F f;
	EventHandlerClass* parent;
};

/// Aspect class used by Widgets that have the possibility of trapping "char events".
/** \ingroup AspectClasses
  * E.g. the WidgetTextBox can trap "char events" therefore they realize the
  * AspectChar through inheritance.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectChar
{
	typedef AspectCharDispatcher<EventHandlerClass> Dispatcher;
	typedef AspectAdapter<typename Dispatcher::F, EventHandlerClass, MessageMapType::IsControl > Adapter;
public:
	/// \ingroup EventHandlersAspectChar
	/// Setting the event handler for the "char" event
	/** If supplied event handler is called when control has the focus and character
	  * event occurs <br>
	  * parameter passed is int which is the character of the nonsystem key being
	  * pressed. Return value must be of type bool, if event handler returns true
	  * event is defined as "handled" meaning the system will not try itself to
	  * handle the event. <br>
	  * Certain widgets, such as WidgetTextBox, will not report VK_RETURN unless
	  * you include ES_WANTRETURN in the style field of of the creational structure
	  * passed when you createTextBox( cs ).
	  */
	void onChar( typename MessageMapType::itsBoolFunctionTakingInt eventHandler ) {
		onChar(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onChar( typename MessageMapType::boolFunctionTakingInt eventHandler ) {
		onChar(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}

	void onChar(const typename Dispatcher::F& f) {
		MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
		ptrThis->setCallback(
			Message( WM_CHAR ), Dispatcher(f, internal_::getTypedParentOrThrow<EventHandlerClass*>(this) )
		);
	}
protected:
	virtual ~AspectChar()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end namespace SmartWin
}

#endif
