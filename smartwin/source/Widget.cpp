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

int Widget::itsInstanceNo = 0;

Widget::~Widget()
{
	// We have to unregister our Widget here...
	for ( std::list < Widget * >::iterator idx = Application::instance().itsWidgets.begin();
		idx != Application::instance().itsWidgets.end();
		++idx )
	{
		if ( * idx == this )
		{
			Application::instance().itsWidgets.erase( idx );
			break;
		}
	}
}

// Subclasses a dialog item inside a dialog, usually used in combination with Dialog resources.
void Widget::subclass( unsigned id )
{
	if ( !itsParent )
		throw xCeption( _T( "Can't subclass a Widget without a parent..." ) );
	itsHandle = ::GetDlgItem( itsParent->handle(), id );
	if ( !itsHandle )
		throw xCeption( _T( "GetDlgItem failed." ) );

	Widget::registerWidget();
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
	// Checking to see if this is our HeartBeat object...
	HeartBeat * tmp = dynamic_cast< HeartBeat * >( this );
	if ( 0 == tmp )
		delete this;
}

void Widget::registerWidget()
{
	Application::instance().registerWidget( this );
}

void Widget::create( const SmartWin::Seed & cs )
{
	itsHandle = ::CreateWindowEx( cs.exStyle,
		cs.getClassName().c_str(),
		cs.caption.c_str(),
		cs.style,
		cs.location.pos.x, cs.location.pos.y, cs.location.size.x, cs.location.size.y,
		itsParent ? itsParent->handle() : 0,
		cs.menuHandle == - 1
			? reinterpret_cast< HMENU >( getCtrlId() )  // This value will become the controls ID when e.g. getting messages in the parent,
			: reinterpret_cast< HMENU >( cs.menuHandle ),
		Application::instance().getAppHandle(),
		reinterpret_cast< LPVOID >( this ) );
	if ( !itsHandle )
	{
		// The most common error is to forget WS_CHILD in the styles
		xCeption x( _T( "CreateWindowEx in Widget::create fizzled ..." ) );
		throw x;
	}
	isChild = ( ( cs.style & WS_CHILD ) == WS_CHILD );
	Application::instance().registerWidget( this );

}

Widget::Widget( Widget * parent, HWND hWnd, bool doReg )
	: 
	isChild( false ),
	itsHandle( hWnd ),
	itsParent( parent ),
	itsCriticalSection( 0 )
{
	if ( doReg )
	{
		if ( itsParent )
			itsParent->itsChildren.push_back( this );
	}
}

Utilities::CriticalSection & Widget::getCriticalSection()
{
	if ( !itsCriticalSection.get() )
	{
		itsCriticalSection.reset( new Utilities::CriticalSection );
	}
	return * itsCriticalSection;
}

void Widget::killMe()
{
	eraseMeFromParentsChildren();
	Widget::kill();
}

void Widget::killChildren()
{
	// A dialog doesn't clean up after itself when destroyed... Don't know why...
	for ( std::vector < Widget * >::iterator idx = itsChildren.begin();
		idx != itsChildren.end();
		++idx )
	{
		( * idx )->kill();
	}
}

void Widget::eraseMeFromParentsChildren()
{
	if ( ! itsParent ) return;

	for ( std::vector < Widget * >::iterator idx = itsParent->itsChildren.begin();
		idx != itsParent->itsChildren.end();
		++idx )
	{
		if ( * idx == this )
		{
			itsParent->itsChildren.erase( idx );
			break;
		}
	}
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

void private_::setHandle( SmartWin::Widget * widget, HWND handle )
{
	widget->itsHandle = handle;
}

// end namespace SmartWin
}
