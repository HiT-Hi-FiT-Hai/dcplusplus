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

Message::Message( UINT msg_ )
	: msg( msg_ ),
	param( 0 )
{
	
}

Message::Message( UINT msg_, LPARAM extraCode )
	: msg(msg_),
	param( 0 )
{
	switch ( msg )
	{
	case WM_SYSCOMMAND :
	case WM_COMMAND :
	case WM_TIMER:
	case WM_NOTIFY :
		param = extraCode;
		break;
	}
}

Message::Message(const MSG& msg_ )
	: msg(msg_.message),
	param( 0 )
{
	switch ( msg )
	{
	case WM_NOTIFY :
		{
			NMHDR * ptrOriginal = reinterpret_cast< NMHDR * >( msg_.lParam );
			param = ptrOriginal->code;
		} break;
	case WM_SYSCOMMAND : {
		param = msg_.wParam & 0xfff0;
		break;
	}
	case WM_TIMER:
		{
			param = msg_.wParam;
		} break;
	case WM_COMMAND :
		{
			if(msg_.lParam == 0) {
				param = LOWORD( msg_.wParam );
			} else {
				param = msg_.wParam;
			}
		} break;
	}
}

bool Message::operator <( const Message & right ) const
{
	if ( msg < right.msg )
		return true;

	if(msg == right.msg && param < right.param) {
		return true;
	}
	return false;
}

bool Message::operator == ( const Message & right ) const {
	return msg == right.msg && param == right.param;
}

// end namespace SmartWin
}
