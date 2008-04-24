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

#include <dwt/widgets/MDIParent.h>

namespace dwt {

MDIParent::Seed::Seed() : 
	BaseType::Seed(_T("MDICLIENT"), WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL, WS_EX_CLIENTEDGE),
	idFirstChild(0),
	windowMenu(NULL)
{
}

void MDIParent::create( const Seed & cs )
{
	CLIENTCREATESTRUCT ccs;
	ccs.hWindowMenu = cs.windowMenu;
	ccs.idFirstChild = cs.idFirstChild;
	
	HWND wnd = ::CreateWindowEx( cs.exStyle,
		cs.className,
		cs.caption.c_str(),
		cs.style,
		cs.location.x(), cs.location.y(), cs.location.width(), cs.location.height(),
		this->getParent() ? this->getParent()->handle() : 0,
		NULL,
		::GetModuleHandle(NULL),
		reinterpret_cast< LPVOID >( &ccs ) );
	if ( !wnd )
	{
		// The most common error is to forget WS_CHILD in the styles
		throw xCeption( _T( "CreateWindowEx in Widget::create fizzled ..." ) );
	}
}

}
