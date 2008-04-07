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
#ifndef AspectRaw_h
#define AspectRaw_h

#include "../Message.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Aspect class used by Widgets that can handle "raw" events.
/** \ingroup AspectClasses
  * If any of the predefined Event Handlers aree not powerful enough you can use this
  * one to handle a "generic" event and parse the message yourself. <br>
  * Note! <br>
  * This is an UNTYPED Event! <br>
  * If there exists other events which handles the message you're interested in
  * handling then USE THOSE instead of this one!!! <br>
  * This is a "last resort" event type.
  */
template< class WidgetType >
class AspectRaw
{
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	struct Dispatcher {
		typedef std::tr1::function<LRESULT (WPARAM, LPARAM)> F;

		Dispatcher(const F& f_) : f(f_) { }
		
		bool operator()(const MSG& msg, LRESULT& ret) {
			ret = f(msg.wParam, msg.lParam);
			return true;
		}
		
		F f;
	};
public:
	/// \ingroup EventHandlersAspectRaw
	/// Setting the member event handler for a "raw" event
	/** Sets the event handler for the "raw" event handler. Use this if any of the
	  * predefined Event Handlers are not powerful enough or if you can't find the
	  * specific Event Handler you need. <br>
	  * Note! <br>
	  * This works in a special way. Unless you add the same Message twice, it will
	  * add that Event Handler and not remove the previous one. You can have
	  * "multiple" raw Event Handlers as long as the Message is not the same! <br>
	  * Note also! <br>
	  * This is an UNTYPED Event! <br>
	  * If there are other events which will handle the message, USE THOSE instead of
	  * this one!!! <br>
	  * <b>This is a "last resort" event type.</b> <br>
	  * Two parameters are passed: LPARAM and WPARAM <br>
	  * Return value is HRESULT which will be passed on to the System
	  */
	/// WARNING, this function uses the natural wparam/lparam order, not the inverted that previous
	/// smartwin versions did. The two functions above emulate the old behaviour though...
	void onRaw(const typename Dispatcher::F& f, const Message & msg) {
		W().addCallback(msg, Dispatcher(f));
	}

protected:
	virtual ~AspectRaw()
	{}
};

// end namespace SmartWin
}

#endif
