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
#ifndef AspectEraseBackground_h
#define AspectEraseBackground_h

#include "boost.h"
#include "../SignalParams.h"

namespace SmartWin
{
// begin namespace SmartWin

// Dispatcher class with specializations for dispatching event to event handlers of
// the AspectEraseBackground Since AspectEraseBackground is used both in
// WidgetWindowBase (container widgets) and Control Widgets we need to specialize
// which implementation to use here!!
template< class EventHandlerClass, class WidgetType, class MessageMapType, bool IsControl >
class AspectEraseBackgroundDispatcher
{
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectEraseBackgroundDispatcher<EventHandlerClass, WidgetType, MessageMapType, true/*Control Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingCanvas func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingCanvas >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		FreeCanvas canvas( This->handle(), reinterpret_cast< HDC >( params.Msg.WParam ) );

		func( ThisParent,
			This,
			canvas
			);
		return 1;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingCanvas func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingCanvas >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		FreeCanvas canvas( This->handle(), reinterpret_cast< HDC >( params.Msg.WParam ) );

		( ( * ThisParent ).*func )(
			This,
			canvas
			);
		return 1;
	}
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectEraseBackgroundDispatcher<EventHandlerClass, WidgetType, MessageMapType, false/*Container Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingCanvas func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingCanvas >( params.Function );

		FreeCanvas canvas( params.This->handle(), reinterpret_cast< HDC >( params.Msg.WParam ) );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		func(
			ThisParent,
			canvas
			);

		return 1;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingCanvas func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingCanvas >( params.FunctionThis );

		FreeCanvas canvas( params.This->handle(), reinterpret_cast< HDC >( params.Msg.WParam ) );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		( ( * ThisParent ).*func )(
			canvas
			);

		return 1;
	}
};

/// Aspect class used by Widgets that have the possibility of handling the erase
/// background property
/** \ingroup AspectClasses
  * E.g. the WidgetWindow have a AspectEraseBackground Aspect to it therefore
  * WidgetDataGrid realize the AspectEraseBackground through  inheritance. When the
  * Widget needs to erase its background this event will be called with a Canvas
  * object which can be used for  manipulating the colors etc the system uses to
  * erase the background of the Widget with.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectEraseBackground
{
	typedef AspectEraseBackgroundDispatcher< EventHandlerClass, WidgetType, MessageMapType, MessageMapType::IsControl > Dispatcher;
public:
	/// \ingroup EventHandlersAspectEraseBackground
	/// Setting the event handler for the "erase background" event
	/** This event handler will be called just before Widget is about to erase its
	  * background, the canvas passed can be used to draw upon etc to manipulate the
	  * background property of the Widget.
	  */
	void onEraseBackground( typename MessageMapType::itsVoidFunctionTakingCanvas eventHandler );
	void onEraseBackground( typename MessageMapType::voidFunctionTakingCanvas eventHandler );

protected:
	virtual ~AspectEraseBackground()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectEraseBackground< EventHandlerClass, WidgetType, MessageMapType >::onEraseBackground( typename MessageMapType::itsVoidFunctionTakingCanvas eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_ERASEBKGND ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectEraseBackground< EventHandlerClass, WidgetType, MessageMapType >::onEraseBackground( typename MessageMapType::voidFunctionTakingCanvas eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_ERASEBKGND ),
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
