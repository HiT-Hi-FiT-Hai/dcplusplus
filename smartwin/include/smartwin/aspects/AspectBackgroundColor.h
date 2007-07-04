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
#ifndef AspectBackgroundColor_h
#define AspectBackgroundColor_h

#include "../SignalParams.h"
#include "AspectAdapter.h"
#include <boost/cast.hpp>

namespace SmartWin
{
// begin namespace SmartWin

struct AspectBackgroundColorDispatcher {
	typedef std::tr1::function<BrushPtr (Canvas&)> F;
	
	AspectBackgroundColorDispatcher(const F& f_, SmartWin::Widget* widget_) : f(f_), widget(widget_) { }

	HRESULT operator()(private_::SignalContent& params) {
		FreeCanvas canvas( widget->handle(), reinterpret_cast< HDC >( params.Msg.WParam ) );

		BrushPtr retBrush = f(canvas);
		return retBrush ? reinterpret_cast< HRESULT >( retBrush->getBrushHandle() ) : 0;
	}

	F f;
	Widget* widget;
};

/// Aspect class used by Widgets that have the possibility of handling the
/// erase background property
/** \ingroup AspectClasses
  * E.g. the WidgetWindow has a background Aspect to it, therefore WidgetDataGrid
  * realizes the AspectEnabled through inheritance.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectBackgroundColor
{
	typedef AspectBackgroundColorDispatcher Dispatcher;
	typedef AspectAdapter<Dispatcher::F, EventHandlerClass, MessageMapType::IsControl> Adapter;
public:
	/// \ingroup EventHandlersAspectBackgroundColor
	/// Setting the event handler for the "erase background" event
	/** The erase background event is raised when the Widget needs to redraw its
	  * background. <br>
	  * Since the Brush object needs to live past the function call we use a BrushPtr
	  * to wrap the Brush object into, you can for instance add a BrushPtr object as
	  * a member of your Widget class and return that BrushPtr from your event
	  * handler. <br>
	  * The parameter passed is Canvas & and return value is BrushPtr <br>
	  * Note! <br>
	  * It is imperative that you keep a reference to the BrushPtr yourself somewhere
	  * e.g. as member of class since otherwise the brush will be released before it
	  * is returned to the system and cannot be used!
	  */
	void onBackgroundColor( typename MessageMapType::itsBrushFunctionTakingCanvas eventHandler ) {
		onBackgroundColor(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	
	void onBackgroundColor( typename MessageMapType::brushFunctionTakingCanvas eventHandler ) {
		onBackgroundColor(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}

	void onBackgroundColor(const typename Dispatcher::F& f) {
		MessageMapBase * ptrThis = boost::polymorphic_cast< MessageMapBase * >( this );
		WidgetType* widget = boost::polymorphic_cast< WidgetType * >( this );
		ptrThis->setCallback(
			widget->getBackgroundColorMessage(), Dispatcher(f, boost::polymorphic_cast<Widget*>(this) )
		);
	}

protected:
	virtual ~AspectBackgroundColor()
	{}
};

// end namespace SmartWin
}

#endif
