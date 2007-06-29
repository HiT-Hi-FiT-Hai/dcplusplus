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

#include "../SignalParams.h"
#include "AspectAdapter.h"

namespace SmartWin
{
// begin namespace SmartWin

struct AspectMouseDispatcher
{
	typedef std::tr1::function<void (const MouseEventResult &)> F;

	AspectMouseDispatcher(const F& f_) : f(f_) { }

	HRESULT operator()(private_::SignalContent& params) {
		f(private_::createMouseEventResultFromMessageParams( params.Msg.LParam, params.Msg.WParam ));
		return 0;
	}

	F f;
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
	typedef AspectMouseDispatcher Dispatcher;
	typedef AspectAdapter<Dispatcher::F, EventHandlerClass, MessageMapType::IsControl> Adapter;

public:
	/// \ingroup EventHandlersAspectMouseClicks
	/// Left mouse button pressed and released event handler setter
	/** If supplied, function will be called when user releases the Left Mouse button
	  * after clicking onto the client area of the Widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onLeftMouseUp( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler ) {
		onLeftMouseUp(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onLeftMouseUp( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler ) {
		onLeftMouseUp(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onLeftMouseUp(const Dispatcher::F& f) {
		onMouse(WM_LBUTTONUP, f);
	}
	
	/// \ingroup EventHandlersAspectMouseClicks
	/// Right mouse button pressed and released event handler setter
	/** If supplied, function will be called when user releases the Right Mouse
	  * button after clicking onto the client area of the Widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onRightMouseUp( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler ) {
		onRightMouseUp(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onRightMouseUp( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler ) {
		onRightMouseUp(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onRightMouseUp(const Dispatcher::F& f) {
		onRightMouseUp(WM_RBUTTONUP, f);
	}

	/// \ingroup EventHandlersAspectMouseClicks
	/// Middle mouse button pressed and released event handler setter
	/** If supplied, function will be called when user releases the middle Mouse
	  * button after clicking onto the client area of the Widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onMiddleMouseUp( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler ) {
		onMiddleMouseUp(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onMiddleMouseUp( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler ) {
		onMiddleMouseUp(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onMiddleMouseUp(const Dispatcher::F& f) {
		onMiddleMouseUp(WM_MBUTTONUP, f);
	}

	/// \ingroup EventHandlersAspectMouseClicks
	/// Left mouse button pressed event handler setter
	/** If supplied, function will be called when user press the Left Mouse button in
	  * the client area of the widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onLeftMouseDown( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler ) {
		onLeftMouseDown(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onLeftMouseDown( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler ) {
		onLeftMouseDown(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onLeftMouseDown(const Dispatcher::F& f) {
		onMouse(WM_LBUTTONDOWN, f);
	}

	/// \ingroup EventHandlersAspectMouseClicks
	/// Right mouse button pressed event handler setter
	/** If supplied, function will be called when user press the Right Mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onRightMouseDown( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler ) {
		onRightMouseDown(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onRightMouseDown( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler ) {
		onRightMouseDown(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onRightMouseDown(const Dispatcher::F& f) {
		onMouse(WM_RBUTTONDOWN, f);
	}

	/// Middle mouse button pressed event handler setter
	/** If supplied, function will be called when user press the Middle Mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onMiddleMouseDown( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler ) {
		onMiddleMouseDown(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onMiddleMouseDown( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler ) {
		onMiddleMouseDown(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onMiddleMouseDown(const Dispatcher::F& f) {
		onMouse(WM_MBUTTONDOWN, f);
	}



	/// Left mouse button double-clicked event handler setter
	/** If supplied, function will be called when user double clicks the Left mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onLeftMouseDblClick( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler ) {
		onLeftMouseDblClick(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onLeftMouseDblClick( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler ) {
		onLeftMouseDblClick(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onLeftMouseDblClick(const Dispatcher::F& f) {
		onMouse(WM_LBUTTONDBLCLK, f);
	}

	/// Right mouse button double-clicked event handler setter
	/** If supplied, function will be called when user  double clicks the Right mouse button
	  * in the client area of the widget. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onRightMouseDblClick( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler ) {
		onRightMouseDblClick(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onRightMouseDblClick( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler ) {
		onRightMouseDblClick(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onRightMouseDblClick(const Dispatcher::F& f) {
		onMouse(WM_RBUTTONDBLCLK, f);
	}



	/// \ingroup EventHandlersAspectMouseClicks
	/// Mouse moved event handler setter
	/** If supplied, function will be called when user moves the mouse. <br>
	  * The parameter passed is const MouseEventResult & which contains the state of
	  * the mouse.
	  */
	void onMouseMove( typename MessageMapType::itsVoidFunctionTakingMouseEventResult eventHandler ) {
		onMouseMove(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onMouseMove( typename MessageMapType::voidFunctionTakingMouseEventResult eventHandler ) {
		onMouseMove(Adapter::adapt1(boost::polymorphic_cast<WidgetType*>(this), eventHandler));
	}
	void onMouseMove(const Dispatcher::F& f) {
		onMouse(WM_MOUSEMOVE, f);
	}

protected:
	
	void onMouse(UINT msg, const Dispatcher::F& f) {
		MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
		ptrThis->setCallback(
			Message( msg ), Dispatcher(f)
		);
	}
	virtual ~AspectMouseClicks()
	{}
};

// end namespace SmartWin
}

#endif
