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
#ifndef AspectScrollable_h
#define AspectScrollable_h

#include "boost.h"
#include "AspectVoidVoidDispatcher.h"
#include "../SignalParams.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Aspect class used by Widgets that have the possibility of scrolling
/** \ingroup AspectClasses
  * E.g. the WidgetSlider have a scroll Aspect to it therefore WidgetSlider realize
  * the AspectScrollable through inheritance.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectScrollable
{
	typedef AspectVoidVoidDispatcher< EventHandlerClass, WidgetType, MessageMapType, MessageMapType::IsControl > Dispatcher;
public:
	/// \ingroup EventHandlersAspectScrollable
	/// Setting the event handler for the "scrolling horizontally" event
	/** A scrolling event occurs when for instance a WidgetSliders value is being
	  * manipulated through user interaction. Such an action would raise this event.
	  * <br>
	  * No parameters are passed.
	  */
	void onScrollHorz( typename MessageMapType::itsVoidFunctionTakingVoid eventHandler );
	void onScrollHorz( typename MessageMapType::voidFunctionTakingVoid eventHandler );

	/// \ingroup EventHandlersAspectScrollable
	/// Setting the event handler for the "scrolling vertically" event
	/** A scrolling event occurs when for instance a WidgetSliders value is being
	  * manipulated through user interaction. Such an action would raise this event.
	  * <br>
	  * No parameters are passed.
	  */
	void onScrollVert( typename MessageMapType::itsVoidFunctionTakingVoid eventHandler );
	void onScrollVert( typename MessageMapType::voidFunctionTakingVoid eventHandler );

protected:
	virtual ~AspectScrollable()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectScrollable< EventHandlerClass, WidgetType, MessageMapType >::onScrollHorz( typename MessageMapType::itsVoidFunctionTakingVoid eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_HSCROLL ),
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
void AspectScrollable< EventHandlerClass, WidgetType, MessageMapType >::onScrollHorz( typename MessageMapType::voidFunctionTakingVoid eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_HSCROLL ),
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
void AspectScrollable< EventHandlerClass, WidgetType, MessageMapType >::onScrollVert( typename MessageMapType::itsVoidFunctionTakingVoid eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_VSCROLL ),
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
void AspectScrollable< EventHandlerClass, WidgetType, MessageMapType >::onScrollVert( typename MessageMapType::voidFunctionTakingVoid eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_VSCROLL ),
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
