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

#include "../include/smartwin/Threads.h"
#include "../include/smartwin/Widget.h"
#include "../include/smartwin/Application.h"
#include "../include/smartwin/BasicTypes.h"
#include "../include/smartwin/xCeption.h"
#include "../include/smartwin/WindowsHeaders.h"

namespace SmartWin
{
// begin namespace SmartWin

Widget::Widget( Widget * parent, HWND hWnd ) : 
	itsHandle( hWnd ),
	itsParent( parent )
{
}

Widget::~Widget()
{
}

// Subclasses a dialog item inside a dialog, usually used in combination with Dialog resources.
void Widget::attach( unsigned id )
{
	if ( !itsParent )
		throw xCeption( _T( "Can't attach a Widget without a parent..." ) );
	itsHandle = ::GetDlgItem( itsParent->handle(), id );
	if ( !itsHandle )
		throw xCeption( _T( "GetDlgItem failed." ) );

}

void Widget::updateWidget()
{
	::InvalidateRect( itsHandle, 0, TRUE );
	::UpdateWindow( itsHandle );
}

void Widget::invalidateWidget()
{
	::InvalidateRect( itsHandle, 0, TRUE );
}

void Widget::kill()
{
	delete this;
}

void Widget::create( const Seed & cs )
{
	itsHandle = ::CreateWindowEx( cs.exStyle,
		cs.className,
		cs.caption.c_str(),
		cs.style,
		cs.location.pos.x, cs.location.pos.y, cs.location.size.x, cs.location.size.y,
		itsParent ? itsParent->handle() : 0,
		cs.menuHandle,
		Application::instance().getAppHandle(),
		reinterpret_cast< LPVOID >( this ) 
	);
	if ( !itsHandle )
	{
		// The most common error is to forget WS_CHILD in the styles
		throw xCeption( _T( "CreateWindowEx in Widget::create fizzled ..." ) );
	}
}

void Widget::attach(HWND hwnd) {
	itsHandle = hwnd;
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

GlobalAtom Widget::propAtom(_T("SmartWin::Widget*"));

void Widget::setCallback( const Message& msg, const CallbackType& callback )
{
	CallbackCollectionType::iterator i = itsCallbacks.find(msg);
	if(i == itsCallbacks.end()) {
		itsCallbacks.insert(std::make_pair(msg, callback));
	} else {
		i->second = callback;
	}
}

bool Widget::tryFire( const MSG & msg, LRESULT & retVal ) {
	// First we must create a "comparable" message...
	Message msgComparer( msg );
	CallbackCollectionType::iterator i = itsCallbacks.find(msgComparer);
	if(i != itsCallbacks.end()) {
		return i->second( msg, retVal );
	}
	return false;
}


// end namespace SmartWin
}
