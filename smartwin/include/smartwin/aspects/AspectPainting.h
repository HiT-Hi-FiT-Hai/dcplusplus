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
#ifndef AspectPainting_h
#define AspectPainting_h

#include "boost.h"
#include "../SignalParams.h"
#include "../CanvasClasses.h"

namespace SmartWin
{
// begin namespace SmartWin

// Dispatcher class with specializations for dispatching event to event handlers of
// the AspectPainting Since AspectPainting is used both in WidgetWindowBase
// (container widgets) and Control Widgets we need to specialize which
// implementation to use here!!
template< class EventHandlerClass, class WidgetType, class MessageMapType, bool IsControl >
class AspectPaintingDispatcher
{
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectPaintingDispatcher<EventHandlerClass, WidgetType, MessageMapType, true/*Control Widget*/>
{
public:
	static HRESULT dispatchPaintThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingCanvas func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingCanvas >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		SmartWin::PaintCanvas canvas( This->handle() );

		( ( * ThisParent ).*func )(
			This,
			canvas
			);

		return 0;
	}

	static HRESULT dispatchPaint( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingCanvas func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingCanvas >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		SmartWin::PaintCanvas canvas( This->handle() );

		func(
			ThisParent,
			This,
			canvas
			);

		return 0;
	}
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectPaintingDispatcher<EventHandlerClass, WidgetType, MessageMapType, false/*Container Widget*/>
{
public:
	static HRESULT dispatchPaintThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingCanvas func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingCanvas >( params.FunctionThis );

		SmartWin::PaintCanvas canvas( params.This->handle() );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		( ( * ThisParent ).*func )(
			canvas
			);

		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}

	static HRESULT dispatchPaint( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingCanvas func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingCanvas >( params.Function );

		SmartWin::PaintCanvas canvas( params.This->handle() );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		func(
			ThisParent,
			canvas
			);

		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}
};

/// Aspect class used by Widgets that can be custom painted.
/** \ingroup AspectClasses
  * When a Painting Event is raised the Widget needs to be repainted.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectPainting
{
	typedef AspectPaintingDispatcher< EventHandlerClass, WidgetType, MessageMapType, MessageMapType::IsControl > Dispatcher;
public:
	/// \ingroup EventHandlersAspectPainting
	/// Painting event handler setter
	/** If supplied, event handler is called with a Canvas & which you can use to
	  * paint stuff onto the window with. <br>
	  * Parameters passed is Canvas &
	  */
	void onPainting( typename MessageMapType::itsVoidFunctionTakingCanvas eventHandler );
	void onPainting( typename MessageMapType::voidFunctionTakingCanvas eventHandler );

protected:
	virtual ~AspectPainting()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectPainting< EventHandlerClass, WidgetType, MessageMapType >::onPainting( typename MessageMapType::itsVoidFunctionTakingCanvas eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_PAINT ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchPaintThis )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectPainting< EventHandlerClass, WidgetType, MessageMapType >::onPainting( typename MessageMapType::voidFunctionTakingCanvas eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_PAINT ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchPaint )
			)
		)
	);
}

// end namespace SmartWin
}

#endif
