// $Revision: 1.6 $
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
#ifndef SignalParams_H
#define SignalParams_H

#include "Message.h"
#include "Widget.h"

namespace SmartWin
{
// begin namespace SmartWin

namespace private_
{
// begin namespace private_

// POD Helper for passing params to dispatcher function...
class SignalParams
{
public:
	SignalParams( Message & msg, Widget * ptr )
		: Msg( msg ), This( ptr )
	{}

	Message Msg;
	Widget * This;
};

// POD Helper for passing the function pointer to the dispatcher
class SignalContent
{
public:
	typedef void ( * voidFunctionTakingVoid )();
	SignalContent( const Message & msg, voidFunctionTakingVoid function, Widget * ptrThis, bool isControl )
		: Msg( msg ), Function( function ), FunctionThis( 0 ), This( ptrThis ), IsControl( isControl ), RunDefaultHandling( false )
	{}

	SignalContent( const Message & msg, voidFunctionTakingVoid function, Widget * ptrThis )
		: Msg( msg ), Function( function ), FunctionThis( 0 ), This( ptrThis ), IsControl( false ), RunDefaultHandling( false )
	{}

	SignalContent( const Message & msg, itsVoidFunction function, Widget * ptrThis, bool isControl )
		: Msg( msg ), Function( 0 ), FunctionThis( function ), This( ptrThis ), IsControl( isControl ), RunDefaultHandling( false )
	{
	}

	SignalContent( const Message & msg, itsVoidFunction function, Widget * ptrThis )
		: Msg( msg ), Function( 0 ), FunctionThis( function ), This( ptrThis ), IsControl( false ), RunDefaultHandling( false )
	{
	}

	SignalContent( const Message & msg, voidFunctionTakingVoid function, itsVoidFunction functionThis, Widget * ptrThis, bool isControl )
		: Msg( msg ), Function( function ), FunctionThis( functionThis ), This( ptrThis ), IsControl( isControl ), RunDefaultHandling( false )
	{
	}

	Message Msg;
	voidFunctionTakingVoid Function;
	itsVoidFunction FunctionThis;
	Widget * This;
	bool IsControl;
	bool RunDefaultHandling;
};

// end namespace private_
}

// end namespace SmartWin
}

#endif
