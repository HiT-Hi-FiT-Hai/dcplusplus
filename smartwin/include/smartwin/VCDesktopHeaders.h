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
#ifndef VCDesktopHeaders_h
#define VCDesktopHeaders_h

#ifndef __GNUC__
#ifndef WINCE

	static const SmartWin::Platform CurrentPlatform = SmartWin::SmartWinDesktop;

	#define SMARTWIN_WNDCLASSEX WNDCLASSEX
	#define SmartWinRegisterClass RegisterClassEx

	// Windows API files...
	#include <windows.h>
	#include <tchar.h>
	#include <winuser.h>
	#include <windowsx.h>
	#include <Shellapi.h>
	#include <commctrl.h>
	#include <assert.h>

/* No longer needed because we replaced std:min() with (std:min)()
	#ifdef max
	#undef max
	#endif

	#ifdef min
	#undef min
	#endif
*/
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
