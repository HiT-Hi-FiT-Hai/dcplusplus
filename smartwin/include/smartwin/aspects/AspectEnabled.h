// $Revision: 1.14 $
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
#ifndef AspectEnabled_h
#define AspectEnabled_h

#include "boost.h"
#include "../SignalParams.h"

namespace SmartWin
{
// begin namespace SmartWin

// Since AspectEnabled is used both in WidgetWindowBase (container widgets) and
// Control Widgets we need to specialize which implementation to use here!!
template< class EventHandlerClass, class WidgetType, class MessageMapType, bool IsControl >
class AspectEnableDispatcher
{
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectEnableDispatcher<EventHandlerClass, WidgetType, MessageMapType, true/*Control Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingBool func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingBool >( params.Function );

		func(
			internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ),
			boost::polymorphic_cast< WidgetType * >( params.This ),
			static_cast< BOOL >( params.Msg.WParam ) == TRUE
			);

		return 0;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingBool func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingBool >( params.FunctionThis );

		( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ) ).*func )(
			boost::polymorphic_cast< WidgetType * >( params.This ),
			static_cast< BOOL >( params.Msg.WParam ) == TRUE
			);

		return 0;
	}
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectEnableDispatcher<EventHandlerClass, WidgetType, MessageMapType, false/*Container Widget*/>
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

/// Aspect class used by Widgets that have the possibility of changing the enabled
/// property
/** \ingroup AspectClasses
  * The WidgetDataGrid has an enabled Aspect to it; therefore it realizes this
  * AspectEnabled through inheritance. <br>
  * When a Widget is enabled it is possible to interact with it in some way, e.g. a
  * button can be pushed, a ComboBox can change value etc. When the Widget is not
  * enabled it cannot change its "value" or be interacted with but is normally still
  * visible.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectEnabled
{
	typedef AspectEnableDispatcher< EventHandlerClass, WidgetType, MessageMapType, MessageMapType::IsControl > Dispatcher;
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
	void onEnabled( typename MessageMapType::itsVoidFunctionTakingBool eventHandler );
	void onEnabled( typename MessageMapType::voidFunctionTakingBool eventHandler );

protected:
	virtual ~AspectEnabled()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectEnabled< EventHandlerClass, WidgetType, MessageMapType >::setEnabled( bool enabled )
{
	::EnableWindow( static_cast< WidgetType * >( this )->handle(), enabled ? TRUE : FALSE );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
bool AspectEnabled< EventHandlerClass, WidgetType, MessageMapType >::getEnabled() const
{
	return ::IsWindowEnabled( static_cast< const WidgetType * >( this )->handle() ) != 0;
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectEnabled< EventHandlerClass, WidgetType, MessageMapType >::onEnabled( typename MessageMapType::itsVoidFunctionTakingBool eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_ENABLE ),
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
void AspectEnabled< EventHandlerClass, WidgetType, MessageMapType >::onEnabled( typename MessageMapType::voidFunctionTakingBool eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_ENABLE ),
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
