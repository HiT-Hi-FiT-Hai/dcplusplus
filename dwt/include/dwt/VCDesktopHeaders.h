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

#ifndef DWT_VCDesktopHeaders_h
#define DWT_VCDesktopHeaders_h

#ifndef __GNUC__
#ifndef WINCE

	// Need to tell msvc which version of Windows we're targeting!
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501
#endif
#ifndef _WIN32_IE
	#define _WIN32_IE 0x0501
#endif
#ifndef WINVER
	#define WINVER 0x501
#endif

	static const dwt::Platform CurrentPlatform = dwt::dwtDesktop;

	#define SMARTWIN_WNDCLASSEX WNDCLASSEX
	#define SmartWinRegisterClass RegisterClassEx

	// Windows API files...
	#include <windows.h>
	#include <tchar.h>
	#include <winuser.h>
	#include <windowsx.h>
	#include <Shellapi.h>
	#include <shlwapi.h>
	#include <commctrl.h>
	#include <commdlg.h>
	#include <assert.h>

#pragma comment( lib, "Comdlg32.lib" )
	#pragma comment( lib, "comctl32.lib" )
#ifdef DLL
	#ifdef _DEBUG
		#ifdef _UNICODE
			#pragma comment( lib, "SmartWinDU.lib" )
		#else
			#pragma comment( lib, "SmartWinD.lib" )
		#endif
	#else
		#ifdef _UNICODE
			#pragma comment( lib, "SmartWinU.lib" )
		#else
			#pragma comment( lib, "SmartWin.lib" )
		#endif
	#endif
#endif
#endif
#endif

#endif
