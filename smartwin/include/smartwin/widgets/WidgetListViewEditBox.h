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
#ifndef WidgetListViewViewEditBox_h
#define WidgetListViewViewEditBox_h

#include "WidgetTextBox.h"

namespace SmartWin
{
// begin namespace SmartWin

namespace private_
{
// begin namespace private_

// Class is only to make attaching of Edit Control in List View possible
// TODO: Make window NOT hide the leftmost cell of row when entering "edit modus"..
class ListViewEditBox : WidgetTextBox
{
public:
	// Class type
	typedef ListViewEditBox ThisType;

	// Object type
	typedef ThisType * ObjectType;

	explicit ListViewEditBox( SmartWin::Widget * parent );

	virtual ~ListViewEditBox()
	{}

	Rectangle itsRect;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline ListViewEditBox::ListViewEditBox( SmartWin::Widget * parent )
	: WidgetTextBox( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Cant have a TextBox without a parent..." ) );
}

#ifdef PORT_ME
template< class EventHandlerClass >
LRESULT ListViewEditBox< EventHandlerClass >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
#ifdef _MSC_VER
#pragma warning( disable : 4060 )
#endif
	switch ( msg )
	{
			// Windows CE doesn't support WM_WINDOWPOSCHANGING message
			// therefore we can't reposition the EditControl in the listview
			// edit modus...!
#ifndef WINCE
		case WM_WINDOWPOSCHANGING :
		{
			// Ensuring position is "locked" in the rectangle initially set in
			// WidgetListView's create func And then letting message "pass
			// through" the rest of the hierarchy...
			WINDOWPOS * pos = ( WINDOWPOS * ) lPar;
			pos->x = itsRect.pos.x;
			pos->y = itsRect.pos.y;
			pos->cx = itsRect.width() - itsRect.pos.x;
			pos->cy = itsRect.size.y - itsRect.pos.y;
		} break;
#endif
	}
	return WidgetTextBox< EventHandlerClass >::sendWidgetMessage( hWnd, msg, wPar, lPar );
}
#endif
// end namespace private_
}

// end namespace SmartWin
}

#endif
