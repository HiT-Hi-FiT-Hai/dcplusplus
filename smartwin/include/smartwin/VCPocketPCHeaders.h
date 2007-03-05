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
#ifndef VCDesktopHeaders_h
#define VCDesktopHeaders_h

#ifndef __GNUC__
#ifdef WINCE

	static const SmartWin::Platform CurrentPlatform = SmartWin::SmartWinCE;

	#pragma comment(linker, "/nodefaultlib:libc.lib")
	#pragma comment(linker, "/nodefaultlib:libcd.lib")
	#pragma comment(linker, "/nodefaultlib:oldnames.lib")

	#ifndef WINVER
		#define WINVER _WIN32_WCE
	#endif

	#include <ceconfig.h>
	#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
		#define SHELL_AYGSHELL
	#endif

	#ifdef _CE_DCOM
		#define _ATL_APARTMENT_THREADED
	#endif

	#include <aygshell.h>
	#pragma comment(lib, "aygshell.lib")

	#include <windows.h>
	#include <stdlib.h>
	#include <malloc.h>
	#include <memory.h>
	#include <tchar.h>

	#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
		#ifndef _DEVICE_RESOLUTION_AWARE
			#define _DEVICE_RESOLUTION_AWARE
		#endif
	#endif

	#ifdef _DEVICE_RESOLUTION_AWARE
		#include "DeviceResolutionAware.h"
	#endif

	#if _WIN32_WCE < 0x500 && ( defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP) )
		#pragma comment(lib, "ccrtrtti.lib")
		#ifdef _X86_
			#if defined(_DEBUG)
				#pragma comment(lib, "libcmtx86d.lib")
			#else
				#pragma comment(lib, "libcmtx86.lib")
			#endif
		#endif
	#endif

	#include <windowsx.h>
	#include <commctrl.h>
	#include <assert.h>

	#include <altcecrt.h>

	#define SMARTWIN_WNDCLASSEX WNDCLASS
	#define SmartWinRegisterClass RegisterClass

	// Windows CE doesn't define these ones
	#ifndef SC_SIZE
		#define SC_SIZE         0xF000
	#endif

	#ifndef WC_COMBOBOX
		#define WC_COMBOBOX         _T("COMBOBOX")
	#endif

	#ifndef TVITEMEX
		#define TVITEMEX         TVITEM
	#endif

	#ifndef WM_NCCREATE
		#define WM_NCCREATE WM_CREATE
	#endif //!WM_NCCREATE

	#ifndef SBARS_SIZEGRIP
		#define SBARS_SIZEGRIP 0
	#endif //!SBARS_SIZEGRIP

	#ifndef WS_OVERLAPPEDWINDOW
		// On WinCE we need all the space we can get...!!
		// therefore we're defaulting to NO CAPTION and NO SYSMENU...
		// To get SYSMENU please add a menu which will add a "cross" up right...
		#define WS_OVERLAPPEDWINDOW WS_BORDER
	#endif //!WS_OVERLAPPEDWINDOW

	#ifndef SetWindowLongPtr
		#define SetWindowLongPtr SetWindowLong
	#endif

	#ifndef GetWindowLongPtr
		#ifdef _WIN64
			ULONG_PTR WINAPI GetWindowLongPtrA( HWND, LONG_PTR );
			ULONG_PTR WINAPI GetWindowLongPtrW( HWND, LONG_PTR );
			#ifdef UNICODE
				#define GetWindowLongPtr  GetWindowLongPtrW
			#else //! UNICODE
				#define GetWindowLongPtr  GetWindowLongPtrA
			#endif // !UNICODE
		#else //! _WIN64
			#define GetWindowLongPtr  GetWindowLong
		#endif // !_WIN64
	#endif // !GetWindowLongPtr

	#ifdef max
		#undef max
	#endif

	#ifdef min
		#undef min
	#endif

	#pragma comment( lib, "Commdlg.lib" )
	#ifdef _DEBUG
		#pragma comment( lib, "SmartWinDevicesD.lib" )
	#else
		#pragma comment( lib, "SmartWinDevices.lib" )
	#endif

#endif
#endif
#endif
