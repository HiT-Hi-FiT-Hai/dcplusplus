/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  SmartWin++

  Copyright (c) 2005 Thomas Hansen

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor SmartWin++ nor the names of its contributors 
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

#ifdef _MSC_VER
// We don't want the stupid "pointer trunctation" to 64 bit architecture warning.
// The warnings aren't justified anyway since they are basically a bug in 7.1 release...
// E.g. the SetWindowLongPtr is defined as SetWindowLong in 32 bits mode but will
// in 64 bits mode be defined as the 64 bits equivalent version, therefore it will
// give you a 64 bit compile warning when this file is compiled with warning level
// 4 (MSVC)
#pragma warning( disable : 4244 )
#pragma warning( disable : 4312 )
#pragma warning( disable : 4311 )
#endif

#include <dwt/Widget.h>

#include <dwt/Threads.h>
#include <dwt/Application.h>
#include <dwt/xCeption.h>

namespace dwt {

Widget::~Widget() {
	
}

// Subclasses a dialog item inside a dialog, usually used in combination with Dialog resources.
void Widget::attach( unsigned id ) {
	if ( !itsParent )
		throw xCeption( _T( "Can't attach a Widget without a parent..." ) );
	HWND hWnd = ::GetDlgItem( itsParent->handle(), id );
	if ( !hWnd )
		throw xCeption( _T( "GetDlgItem failed." ) );
	setHandle(hWnd);
}

void Widget::kill() {
	delete this;
}

HWND Widget::create( const Seed & cs ) {
	HWND hWnd = ::CreateWindowEx( cs.exStyle,
		cs.className,
		cs.caption.c_str(),
		cs.style,
		cs.location.x(), cs.location.y(), cs.location.width(), cs.location.height(),
		itsParent ? itsParent->handle() : 0,
		cs.menuHandle,
		Application::instance().getAppHandle(),
		reinterpret_cast< LPVOID >( this ) 
	);
	if (!hWnd) {
		// The most common error is to forget WS_CHILD in the styles
		throw xCeption( _T( "CreateWindowEx in Widget::create fizzled ..." ) );
	}
	return hWnd;
}

void Widget::setHandle(HWND hwnd) {
	if(itsHandle) {
		throw xCeption(_T("You may not attach to a widget that's already attached"));
	}
	itsHandle = hwnd;
	::SetProp(hwnd, propAtom, reinterpret_cast<HANDLE>(this) );
}

void Widget::addRemoveStyle( DWORD addStyle, bool add )
{
	DWORD newStyle = GetWindowStyle( itsHandle );
	bool mustUpdate = false;
	if ( add && ( newStyle & addStyle ) != addStyle )
	{
		mustUpdate = true;
		newStyle |= addStyle;
	}
	else if ( !add && ( newStyle & addStyle ) == addStyle )
	{
		mustUpdate = true;
		newStyle ^= addStyle;
	}
	if ( mustUpdate )
	{
		::SetWindowLong( itsHandle, GWL_STYLE, newStyle );

		// Faking a recheck in the window to read new style... (hack)
		::SetWindowPos( itsHandle, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
			SWP_NOZORDER | SWP_FRAMECHANGED );
	}
}

void Widget::addRemoveExStyle( DWORD addStyle, bool add )
{
	DWORD newStyle = ::GetWindowLong( itsHandle, GWL_EXSTYLE );
	bool mustUpdate = false;
	if ( add && ( newStyle & addStyle ) != addStyle )
	{
		mustUpdate = true;
		newStyle |= addStyle;
	}
	else if ( !add && ( newStyle & addStyle ) == addStyle )
	{
		mustUpdate = true;
		newStyle ^= addStyle;
	}
	if ( mustUpdate )
	{
		::SetWindowLong( itsHandle, GWL_EXSTYLE, newStyle );

		// Faking a recheck in the window to read new style... (hack)
		::SetWindowPos( itsHandle, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
			SWP_NOZORDER | SWP_FRAMECHANGED );
	}
}

GlobalAtom Widget::propAtom(_T("dwt::Widget*"));

void Widget::addCallback( const Message& msg, const CallbackType& callback ) {
	itsCallbacks[msg].push_back(callback);
}

void Widget::setCallback( const Message& msg, const CallbackType& callback ) {
	CallbackList& callbacks = itsCallbacks[msg];
	callbacks.clear();
	callbacks.push_back(callback);
}

bool Widget::tryFire( const MSG & msg, LRESULT & retVal ) {
	// First we must create a "comparable" message...
	Message msgComparer( msg );
	CallbackCollectionType::iterator i = itsCallbacks.find(msgComparer);
	bool handled = false;
	if(i != itsCallbacks.end()) {
		CallbackList& list = i->second;
		for(CallbackList::iterator j = list.begin(); j != list.end(); ++j) {
			handled |= (*j)(msg, retVal);
		}
	}
	return handled;
}

}
