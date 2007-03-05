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
#ifndef AspectMouseClicks_h
#define AspectMouseClicks_h

#include "boost.h"
#include "../SignalParams.h"

namespace SmartWin
{
// begin namespace SmartWin

// Dispatcher class with specializations for dispatching event to event handlers of
// the AspectActivate Since AspectMouse is used both in WidgetWindowBase (container
// widgets) and Control Widgets we need to specialize which implementation to use
// here!!
template< class EventHandlerClass, class WidgetType, class MessageMapType, bool IsControl >
class AspectMouseDispatcher
{
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectMouseDispatcher<EventHandlerClass, WidgetType, MessageMapType, true/*Control Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingMouseEventResult func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingMouseEventResult >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		func( ThisParent,
			This,
			private_::createMouseEventResultFromMessageParams( params.Msg.LParam, params.Msg.WParam )
			);

		return 0;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingMouseEventResult func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingMouseEventResult >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		( ( * ThisParent ).*func )(
			This,
			private_::createMouseEventResultFromMessageParams( params.Msg.LParam, params.Msg.WParam )
			);

		return 0;
	}
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectMouseDispatcher<EventHandlerClass, WidgetType, MessageMapType, false/*Container Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingMouseEventResult func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingMouseEventResult >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		func(
			ThisParent,
			private_::createMouseEventResultFromMessageParams( params.Msg.LParam, params.Msg.WParam )
			);

		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingMouseEventResult func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingMouseEventResult >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		( ( * ThisParent ).*func )(
			private_::createMouseEventResultFromMessageParams( params.Msg.LParam, params.Msg.WParam )
			);

		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}
};

/// Aspect class used by Widgets that have the possibility of trapping "mouse
/// clicked" events.
/** \ingroup AspectClasses
  * E.g. the WidgetWindow can trap "mouse clicked events" therefore it realize the
  * AspectMouseClicks through inheritance.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectMouseClicks
{
	typedef AspectMouseDispatcher< EventHandlerClass, WidgetType, MessageMapType, MessageMapType::IsControl > Dispatcher;
public:
	/// \ingroup EventHandlersAspectMouseClicks
	/// Left mouse button pressed and released event handler setter
	/** If supplied, function will be called when user releases the Left Mouse button
	  * after clicking onto the client area of the Widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onLeftMouseUp( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler );
	void onLeftMouseUp( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler );

	/// \ingroup EventHandlersAspectMouseClicks
	/// Right mouse button pressed and released event handler setter
	/** If supplied, function will be called when user releases the Right Mouse
	  * button after clicking onto the client area of the Widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onRightMouseUp( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler );
	void onRightMouseUp( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler );

	/// \ingroup EventHandlersAspectMouseClicks
	/// Middle mouse button pressed and released event handler setter
	/** If supplied, function will be called when user releases the middle Mouse
	  * button after clicking onto the client area of the Widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onMiddleMouseUp( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler );
	void onMiddleMouseUp( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler );

	/// \ingroup EventHandlersAspectMouseClicks
	/// Left mouse button pressed event handler setter
	/** If supplied, function will be called when user press the Left Mouse button in
	  * the client area of the widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onLeftMouseDown( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler );
	void onLeftMouseDown( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler );

	/// \ingroup EventHandlersAspectMouseClicks
	/// Right mouse button pressed event handler setter
	/** If supplied, function will be called when user press the Right Mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onRightMouseDown( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler );
	void onRightMouseDown( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler );

	/// Middle mouse button pressed event handler setter
	/** If supplied, function will be called when user press the Middle Mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onMiddleMouseDown( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler );
	void onMiddleMouseDown( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler );



	/// Left mouse button double-clicked event handler setter
	/** If supplied, function will be called when user double clicks the Left mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onLeftMouseDblClick( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler );
	void onLeftMouseDblClick( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler );

	/// Right mouse button double-clicked event handler setter
	/** If supplied, function will be called when user  double clicks the Right mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onRightMouseDblClick( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler );
	void onRightMouseDblClick( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler );



	/// \ingroup EventHandlersAspectMouseClicks
	/// Mouse moved event handler setter
	/** If supplied, function will be called when user moves the mouse. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onMouseMove( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler );
	void onMouseMove( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler );

protected:
	virtual ~AspectMouseClicks()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onLeftMouseUp( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_LBUTTONUP ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}


template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onLeftMouseUp( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_LBUTTONUP ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onRightMouseUp( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_RBUTTONUP ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onRightMouseUp( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_RBUTTONUP ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onMiddleMouseUp( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_MBUTTONUP ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onMiddleMouseUp( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_MBUTTONUP ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onLeftMouseDown( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_LBUTTONDOWN ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onLeftMouseDown( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_LBUTTONDOWN ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}

// Right Mouse button down
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onRightMouseDown( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_RBUTTONDOWN ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onRightMouseDown( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_RBUTTONDOWN ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onMiddleMouseDown( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_MBUTTONDOWN ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onMiddleMouseDown( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_MBUTTONDOWN ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}




template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onLeftMouseDblClick( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_LBUTTONDBLCLK ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onLeftMouseDblClick( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_LBUTTONDBLCLK ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}


template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onRightMouseDblClick( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_RBUTTONDBLCLK ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onRightMouseDblClick( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_RBUTTONDBLCLK ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}



// Mouse Movement
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onMouseMove( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_MOUSEMOVE ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectMouseClicks< EventHandlerClass, WidgetType, MessageMapType >::onMouseMove(
			typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_MOUSEMOVE ),
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
