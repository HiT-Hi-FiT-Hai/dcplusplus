// $Revision: 1.5 $
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
#include "../include/smartwin/Message.h"

namespace SmartWin
{
// begin namespace SmartWin

Message::Message( UINT msg )
	: Handle( 0 ),
	Msg( msg ),
	WParam( 0 ),
	LParam( 0 )
{
}
Message::Message( UINT msg, LPARAM extraCode )
	: Handle( 0 ),
	Msg( msg ),
	WParam( 0 ),
	LParam( 0 )
{
	switch ( msg )
	{
	case WM_NOTIFY :
		LParam = extraCode;
		break;
	case WM_SYSCOMMAND :
	case WM_COMMAND :
		WParam = extraCode;
	}
}

Message::Message( HANDLE handle, UINT msg, WPARAM wPar, LPARAM lPar, bool forceValues )
	: Handle( handle ),
	Msg( msg ),
	WParam( wPar ),
	LParam( lPar )
{
	if ( forceValues )
		return; // We don't manipulate to get the code or anything here...
	switch ( msg )
	{
	case WM_NOTIFY :
		{
			NMHDR * ptrOriginal = reinterpret_cast< NMHDR * >( lPar );
			LParam = ptrOriginal->code;
		} break;
	case WM_SYSCOMMAND :
		{
			// Checking to see if this is from a menu
			WParam = wPar;
		} break;
	case WM_COMMAND :
		{
			// Checking to see if this is from a menu
			if ( lPar == 0 )
				WParam = static_cast< WPARAM >( LOWORD( wPar ) );
			else
				WParam = static_cast< WPARAM >( HIWORD( wPar ) );
		} break;
	}
}

bool operator <( const Message & left, const Message & right )
{
	if ( left.Msg > right.Msg )
		return false;
	else if ( left.Msg < right.Msg )
		return true;

	// MESSAGES are EQUAL!

	switch ( left.Msg )
	{
	case WM_SYSCOMMAND :
	case WM_COMMAND :
		if ( left.WParam < right.WParam )
			return true;
		break;
	case WM_NOTIFY :
		if ( left.LParam < right.LParam )
			return true;
		break;
	}
	return false;
}

bool operator == ( const Message & left, const Message & right )
{
	if ( left.Msg == right.Msg )
	{
		switch ( left.Msg )
		{
		case WM_SYSCOMMAND :
		case WM_COMMAND :
			if ( left.WParam == right.WParam )
				return true;
			break;
		case WM_NOTIFY :
			if ( left.LParam == right.LParam )
				return true;
			break;
		default:
			return left.Msg == right.Msg;
		}
	}
	return false;
}

// end namespace SmartWin
}
