// $Revision: 1.16 $
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
#ifndef AspectVisible_h
#define AspectVisible_h

#include "boost.h"
#include "../SignalParams.h"

namespace SmartWin
{
// begin namespace SmartWin

// Dispatcher class with specializations for dispatching event to event handlers of
// the AspectVisible. Since AspectVisible is used both in WidgetWindowBase
// (container widgets) and Control Widgets we need to specialize which
// implementation to use here!!
template< class EventHandlerClass, class WidgetType, class MessageMapType, bool IsControl >
class AspectVisibleDispatcher
{
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectVisibleDispatcher< EventHandlerClass, WidgetType, MessageMapType, true/*Control Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingBool func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingBool >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		func(
			ThisParent,
			This,
			static_cast< BOOL >( params.Msg.WParam ) == TRUE
			);

		return 0;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingBool func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingBool >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		( ( * ThisParent ).*func )(
			This,
			static_cast< BOOL >( params.Msg.WParam ) == TRUE
			);

		return 0;
	}
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectVisibleDispatcher<EventHandlerClass, WidgetType, MessageMapType, false/*Container Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingBool func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingBool >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		func(
			ThisParent,
			static_cast< BOOL >( params.Msg.WParam ) == TRUE
			);

		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingBool func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingBool >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		( ( * ThisParent ).*func )(
			static_cast< BOOL >( params.Msg.WParam ) == TRUE
			);

		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}
};

/// \ingroup AspectClasses
/// Aspect class used by Widgets that have the possibility of manipulating the
/// visibility property
/** E.g. the WidgetDataGrid have a Visibility Aspect to it therefore WidgetDataGrid
  * realizes AspectVisible through inheritance. <br>
  * Most Widgets realize this Aspect since they can become visible and invisible.
  * When the visibilty state of the Widget changes in one way or another the visible
  * event is raised. <br>
  * Use the onVisibilityChanged function to set an event handler for trapping this
  * event.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectVisible
{
	typedef AspectVisibleDispatcher< EventHandlerClass, WidgetType, MessageMapType, MessageMapType::IsControl > Dispatcher;
public:
	/// Sets the visibility property of the Widget
	/** Changes the visibility property of the Widget. <br>
	  * Use this function to change the visibility property of the Widget
	  */
	void setVisible( bool visible );

	/// Retrieves the visible property of the Widget
	/** Use this function to check if the Widget is visible or not. <br>
	  * If the Widget is visible this function will return true.
	  */
	bool getVisible() const;

	/// \ingroup EventHandlersAspectVisible
	/// Setting the event handler for the "visible" event
	/** When the visible state of the Widget has changed, this event will be raised.
	  * <br>
	  * A boolean parameter passed indicates if the Widget is visible or not. <br>
	  * If the boolean value is true, the Widget is visible, otherwise it is
	  * invisible.
	  */
	void onVisibilityChanged( typename MessageMapType::itsVoidFunctionTakingBool eventHandler );
	void onVisibilityChanged( typename MessageMapType::voidFunctionTakingBool eventHandler );

protected:
	virtual ~AspectVisible()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectVisible< EventHandlerClass, WidgetType, MessageMapType >::setVisible( bool visible )
{
	::ShowWindow( static_cast< WidgetType * >( this )->handle(), visible ? SW_SHOW : SW_HIDE );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
bool AspectVisible< EventHandlerClass, WidgetType, MessageMapType >::getVisible() const
{
	return ::IsWindowVisible( static_cast< const WidgetType * >( this )->handle() ) != 0;
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectVisible< EventHandlerClass, WidgetType, MessageMapType >::onVisibilityChanged( typename MessageMapType::itsVoidFunctionTakingBool eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_SHOWWINDOW ),
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
void AspectVisible< EventHandlerClass, WidgetType, MessageMapType >::onVisibilityChanged( typename MessageMapType::voidFunctionTakingBool eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_SHOWWINDOW ),
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
