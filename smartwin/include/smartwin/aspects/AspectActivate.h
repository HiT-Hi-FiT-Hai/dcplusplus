// $Revision: 1.13 $
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
#ifndef AspectActivate_h
#define AspectActivate_h

#include "boost.h"
#include "../SignalParams.h"

namespace SmartWin
{
// begin namespace SmartWin

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectActivateDispatcher
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingBool func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingBool >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		func(
			ThisParent,
			LOWORD( params.Msg.WParam ) == WA_ACTIVE || LOWORD( params.Msg.WParam ) == WA_CLICKACTIVE
			);

		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingBool func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingBool >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		( ( * ThisParent ).*func )(
			LOWORD( params.Msg.WParam ) == WA_ACTIVE || LOWORD( params.Msg.WParam ) == WA_CLICKACTIVE
			);

		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}
};

/// Aspect class used by Widgets that can be activated.
/** \ingroup AspectClasses
  * When a Widget is being activated it means that it becomes the "active" Widget
  * meaning that it receives keyboard input and normally if it is a text Widget gets
  * to own the caret. This Aspect is closely related to the AspectFocus Aspect.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectActivate
{
	typedef AspectActivateDispatcher< EventHandlerClass, WidgetType, MessageMapType > Dispatcher;
public:
	/// Activates the Widget
	/** Changes the activated property of the Widget. <br>
	  * Use this function to change the activated property of the Widget to true or
	  * with other words make this Widget  the currently active Widget.
	  */
	void setActive();

	/// Retrieves the activated property of the Widget
	/** Use this function to check if the Widget is active or not. <br>
	  * If the Widget is active this function will return true.
	  */
	bool getActive() const;

	/// \ingroup EventHandlersAspectActivate
	/// Setting the member event handler for the "activated" event
	/** Sets the event handler for changes to the active property of the Widget, if
	  * the active status of the Widget changes the supplied event handler will be
	  * called with either true or false indicating the active state of the Widget.
	  * Parameter passed is bool
	  */
	void onActivate( typename MessageMapType::itsVoidFunctionTakingBool eventHandler );
	void onActivate( typename MessageMapType::voidFunctionTakingBool eventHandler );

protected:
	virtual ~AspectActivate()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectActivate< EventHandlerClass, WidgetType, MessageMapType >::setActive()
{
	::SetActiveWindow( static_cast< WidgetType * >( this )->handle() );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
bool AspectActivate< EventHandlerClass, WidgetType, MessageMapType >::getActive() const
{
	return ::GetActiveWindow() == static_cast< const WidgetType * >( this )->handle();
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectActivate< EventHandlerClass, WidgetType, MessageMapType >::onActivate( typename MessageMapType::itsVoidFunctionTakingBool eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_ACTIVATE ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectActivate< EventHandlerClass, WidgetType, MessageMapType >::onActivate( typename MessageMapType::voidFunctionTakingBool eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_ACTIVATE ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}

// end namespace SmartWin
}

#endif
