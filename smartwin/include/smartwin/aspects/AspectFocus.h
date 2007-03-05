// $Revision: 1.15 $
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
#ifndef AspectFocus_h
#define AspectFocus_h

#include "boost.h"

#include "AspectVoidVoidDispatcher.h"
#include "../SignalParams.h"

namespace SmartWin
{
// begin namespace SmartWin

// Dispatcher class with specializations for dispatching event to event handlers of
// the AspectFocus Since AspectFocus is used both in WidgetWindowBase (container
// widgets) and Control Widgets we need to specialize which implementation to use
// here!!
template< class EventHandlerClass, class WidgetType, class MessageMapType, bool IsControl >
class AspectFocusDispatcher
{
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectFocusDispatcher<EventHandlerClass, WidgetType, MessageMapType, true/*Control Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingVoid func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingVoid >( ( void( * )() ) params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		func(
			ThisParent,
			This
			);

		params.RunDefaultHandling = true; // WM_SETFOCUS Must dispatch to Windows Message Procedure in order to display caret
		return 0;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingVoid func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingVoid >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		( ( * ThisParent ).*func )( This );

		params.RunDefaultHandling = true; // WM_SETFOCUS Must dispatch to Windows Message Procedure in order to display caret
		return 0;
	}
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectFocusDispatcher< EventHandlerClass, WidgetType, MessageMapType, false >
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingVoid func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingVoid >( params.Function );

		func(
			internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This )
			);
		params.RunDefaultHandling = true; // WM_SETFOCUS Must dispatch to Windows Message Procedure in order to display caret
		MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( params.This );
		return This->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingVoid func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingVoid >( params.FunctionThis );

		( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ) ).*func )(
			);
		params.RunDefaultHandling = true; // WM_SETFOCUS Must dispatch to Windows Message Procedure in order to display caret
		MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( params.This );
		return This->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}
};

/// Aspect class used by Widgets that have the possibility of retrieving the focus
/** \ingroup AspectClasses
  * E.g. the WidgetDataGrid have a Focus Aspect to it therefore WidgetDataGrid
  * realize the AspectFocus through inheritance. This Aspect is closely related to
  * the AspectActivate and when a Widget is being activated it means that it is the
  * "active" Widget meaning that it receives keyboard input for one and normally if
  * it is a text Widget gets to own the carret.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectFocus
{
	typedef AspectFocusDispatcher< EventHandlerClass, WidgetType, MessageMapType, MessageMapType::IsControl > Dispatcher;
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
	void onKillFocus( typename MessageMapType::itsVoidFunctionTakingVoid eventHandler );
	void onKillFocus( typename MessageMapType::voidFunctionTakingVoid eventHandler );

	/// \ingroup EventHandlersAspectAspectFocus
	/// Sets the event handler for what function to be called when control loses focus.
	/** This function will be called just after the Widget has gained focus. No
	  * parameters are passed.
	  */
	void onFocus( typename MessageMapType::itsVoidFunctionTakingVoid eventHandler );
	void onFocus( typename MessageMapType::voidFunctionTakingVoid eventHandler );

protected:
	virtual ~AspectFocus()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectFocus< EventHandlerClass, WidgetType, MessageMapType >::setFocus()
{
	::SetFocus( static_cast< WidgetType * >( this )->handle() );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
bool AspectFocus< EventHandlerClass, WidgetType, MessageMapType >::getFocus() const
{
	return ::GetFocus() == static_cast< const WidgetType * >( this )->handle();
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectFocus< EventHandlerClass, WidgetType, MessageMapType >::onKillFocus( typename MessageMapType::itsVoidFunctionTakingVoid eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_KILLFOCUS ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectFocus< EventHandlerClass, WidgetType, MessageMapType >::onKillFocus( typename MessageMapType::voidFunctionTakingVoid eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_KILLFOCUS ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectFocus< EventHandlerClass, WidgetType, MessageMapType >::onFocus( typename MessageMapType::itsVoidFunctionTakingVoid eventHandler )
{
	// Here we can just reuse the same dispatchers since the params are the same...
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_SETFOCUS ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectFocus< EventHandlerClass, WidgetType, MessageMapType >::onFocus( typename MessageMapType::voidFunctionTakingVoid eventHandler )
{
	// Here we can just reuse the same dispatchers since the params are the same...
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_SETFOCUS ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}

// end namespace SmartWin
}

#endif
