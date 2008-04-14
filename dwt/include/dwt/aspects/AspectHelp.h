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

#ifndef ASPECTHELP_H_
#define ASPECTHELP_H_

#include "../Message.h"
#include <functional>

namespace dwt {

template<typename WidgetType>
class AspectHelp {
	struct Dispatcher {
		typedef std::tr1::function<void (HWND, unsigned)> F;

		Dispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(msg.lParam);
			if(lphi->iContextType != HELPINFO_WINDOW)
				return false;
			f(reinterpret_cast<HWND>(lphi->hItemHandle), lphi->dwContextId);
			ret = TRUE;
			return true;
		}

		F f;
	};

public:
	unsigned getHelpId() {
		return ::GetWindowContextHelpId(static_cast<WidgetType*>(this)->handle());
	}

	void setHelpId(unsigned id) {
		::SetWindowContextHelpId(static_cast<WidgetType*>(this)->handle(), id);
	}

	void onHelp(const typename Dispatcher::F& f) {
		static_cast<WidgetType*>(this)->addCallback(
			Message( WM_HELP ), Dispatcher(f)
		);
	}
};

}

#endif /*ASPECTHELP_H_*/
