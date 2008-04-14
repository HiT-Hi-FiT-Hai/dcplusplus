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

#include <dwt/WindowClass.h>
#include <dwt/Application.h>
#include <dwt/xCeption.h>
#include <dwt/Widget.h>

#include <typeinfo>
#include <sstream>

namespace dwt {

int WindowClass::itsInstanceNo;

WindowClass::WindowClass(const tstring& className, WNDPROC wndProc, LPCTSTR menu, HBRUSH background, IconPtr icon_, IconPtr smallIcon_, HCURSOR cursor) : atom(0), icon(icon_), smallIcon(smallIcon_) {
	WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
	wc.lpfnWndProc = wndProc;
	wc.lpszMenuName = menu;
	wc.hbrBackground = background;
	wc.hIcon = icon ? icon->handle() : NULL;
	wc.hIconSm = smallIcon ? icon->handle() : NULL;
	wc.hCursor = cursor;
	wc.hInstance = Application::instance().getAppHandle();
	wc.lpszClassName = className.c_str();
	
	atom = ::RegisterClassEx(&wc);
	if ( 0 == atom )
	{
		xCeption x( _T( "Could not register class " ) + className );
		throw x;
	}
}

WindowClass::~WindowClass() {
	if(atom != 0) {
		::UnregisterClass(getClassName(), Application::instance().getAppHandle());
	}
}

tstring WindowClass::getNewClassName(Widget* widget)
{
	std::basic_stringstream< TCHAR > className;
	className << typeid(*widget).name() << ++itsInstanceNo;
	return className.str();
}

}
