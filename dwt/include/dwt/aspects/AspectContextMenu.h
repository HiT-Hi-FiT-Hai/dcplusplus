/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors 
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

#ifndef ASPECTCONTEXTMENU_H_
#define ASPECTCONTEXTMENU_H_

#include "../Point.h"
#include "../Dispatchers.h"
#include "../Message.h"

namespace dwt {

template<typename WidgetType>
class AspectContextMenu {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }

	struct Dispatcher : Dispatchers::Base<bool (const ScreenCoordinate&)> {
		typedef Dispatchers::Base<bool(const ScreenCoordinate&)> BaseType;
		Dispatcher(const F& f_) : BaseType(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			bool shown = f(ScreenCoordinate(Point::fromLParam(msg.lParam)));
			ret = shown;
			return shown;
		}
	};
	
public:
	void onContextMenu(const typename Dispatcher::F& f) {
		W().addCallback(Message( WM_CONTEXTMENU ), Dispatcher(f));
	}
};

}

#endif /*ASPECTCONTEXTMENU_H_*/
