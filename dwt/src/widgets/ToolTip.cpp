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

#include "../../include/smartwin/widgets/ToolTip.h"

namespace SmartWin {

ToolTip::Seed::Seed() : 
	BaseType::Seed(TOOLTIPS_CLASS, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX, WS_EX_TRANSPARENT)
{
}

void ToolTip::create( const Seed & cs )
{
	xAssert((cs.style & WS_POPUP) == WS_POPUP, _T("Widget must have WS_POPUP style"));

	BaseType::create(cs);
}

void ToolTip::relayEvent(const MSG& msg) {
	if(msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
		sendMessage(TTM_RELAYEVENT, 0, reinterpret_cast<LPARAM>(&msg));
}

void ToolTip::setTool(Widget* widget, const Dispatcher::F& f) {
	addCallback(
		Message(WM_NOTIFY, TTN_GETDISPINFO), Dispatcher(f)
	);
	
	TOOLINFO ti = { sizeof(TOOLINFO) };
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.hwnd = getParent()->handle();
	ti.uId = reinterpret_cast<UINT_PTR>(widget->handle());
	ti.lpszText = LPSTR_TEXTCALLBACK;
	sendMessage(TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
	
}

}
