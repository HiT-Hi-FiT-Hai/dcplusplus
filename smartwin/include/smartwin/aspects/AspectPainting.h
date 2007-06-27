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

struct AspectPaintingDispatcher {
	typedef boost::function<void (Canvas&)> F;
	
	AspectPaintingDispatcher(const F& f_, SmartWin::Widget* widget_) : f(f_), widget(widget_) { }

	HRESULT operator()(private_::SignalContent& params) {
		PaintCanvas canvas( widget->handle() );

		f(canvas);
		return 0;
	}

	F f;
	Widget* widget;
};


/// Aspect class used by Widgets that can be custom painted.
/** \ingroup AspectClasses
  * When a Painting Event is raised the Widget needs to be repainted.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectPainting
{
	typedef AspectPaintingDispatcher Dispatcher;
	typedef AspectAdapter<Dispatcher::F, EventHandlerClass, MessageMapType::IsControl> Adapter;
public:
	/// \ingroup EventHandlersAspectPainting
	/// Painting event handler setter
	/** If supplied, event handler is called with a Canvas & which you can use to
	  * paint stuff onto the window with. <br>
	  * Parameters passed is Canvas &
	  */
	void onPainting( typename MessageMapType::itsVoidFunctionTakingCanvas eventHandler ) {
		onPainting(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onPainting( typename MessageMapType::voidFunctionTakingCanvas eventHandler ) {
		onPainting(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}

	void onPainting(const Dispatcher::F& f) {
		MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
		ptrThis->setCallback(
			Message( WM_PAINT ), Dispatcher(f, boost::polymorphic_cast<Widget*>(this) )
		);
	}

protected:
	virtual ~AspectPainting()
	{}
};

// end namespace SmartWin
}

#endif
