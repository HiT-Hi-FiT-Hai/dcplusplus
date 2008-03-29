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
#ifndef AspectColor_h
#define AspectColor_h

#include "../resources/Brush.h"
#include "../CanvasClasses.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Aspect class used by Widgets that have the possibility of handling the
/// erase background property
/** \ingroup AspectClasses
  * E.g. the WidgetWindow has a background Aspect to it, therefore Table
  * realizes the AspectEnabled through inheritance.
  */
template<class WidgetType>
class AspectColor {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
public:
	void setColor(COLORREF text, COLORREF background) {
		W().setColorImpl(text, background);
	}
	
protected:
	virtual ~AspectColor() { }
};

template< class WidgetType >
class AspectColorCtlImpl {
	friend class AspectColor<WidgetType>;
	
	WidgetType& W() { return *static_cast<WidgetType*>(this); }

	struct Dispatcher {
		Dispatcher(COLORREF text_, COLORREF bg_) : brush(new Brush(bg_)), text(text_), bg(bg_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			HDC dc = (HDC) msg.wParam;
			::SetTextColor(dc, text);
			::SetBkColor(dc, bg);
			ret = brush ? reinterpret_cast< LRESULT >( brush->handle() ) : 0;
			return true;
		}

		BrushPtr brush;
		COLORREF text;
		COLORREF bg;
	};

	/// Set the background, text and text colors 
	void setColorImpl(COLORREF text, COLORREF background) {
		W().setCallback(Message(WM_CTLCOLOR), Dispatcher(text, background));
	}

protected:
	virtual ~AspectColorCtlImpl() { }
};


// end namespace SmartWin
}

#endif
