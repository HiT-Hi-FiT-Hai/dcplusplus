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

#ifdef WINCE
#ifndef DWT_ApplicationPlatformWinCE_h
#define DWT_ApplicationPlatformWinCE_h

#include "Widget.h"

namespace dwt {

/// Platform specific code for the application class for Windows CE version of
/// Windows API
/** Contains WinCE Windows API specific functions for the application class.
*/
template< >
class ApplicationPlatform< SmartWinCE >
{
public:
	/// Shows the SIP on Windows CE devices
	/** Call this function to show the SIP (keyboard) on Windows CE devices,
	  * call showSip with visible = false to hide it again.
	  */
	void showSip( Widget * widget, bool visible = true )
	{
		SipShowIM( visible ? SIPF_ON : SIPF_OFF );
	}

	/// Returns true if SIP is visible
	/** Call this function to determine if SIP is visible
	*/
	bool getSipVisible()
	{
		SIPINFO info;
		info.cbSize = sizeof( SIPINFO );
		SipGetInfo( & info );
		return ( info.fdwFlags & SIPF_ON ) == SIPF_ON;
	}
};

}

#endif //! ApplicationPlatformWinCE_h
#endif //! WINCE
