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
#ifndef GCCHeaders_h
#define GCCHeaders_h

#ifdef __GNUC__

	// Need to tell gcc which version of Windows we're targeting!
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501
#endif
#ifndef _WIN32_IE
	#define _WIN32_IE 0x0501
#endif
#ifndef WINVER
	#define WINVER 0x501
#endif
	// Removing windows.h max and min macro
	#undef NOMINMAX
	#define NOMINMAX

	static const SmartWin::Platform CurrentPlatform = SmartWin::SmartWinDesktop;

	#define SMARTWIN_WNDCLASSEX WNDCLASSEX
	#define SmartWinRegisterClass RegisterClassEx

	// Windows API files...
	#include <errno.h>
	#include <windows.h>
	#include <tchar.h>
	#include <winuser.h>
	#include <windowsx.h>
	#include <shellapi.h>
	#include <shlwapi.h>
	#include <commctrl.h>
	#include <commdlg.h>
	#include <assert.h>

	// Need to forward declare these since GCC does syntax checking of
	// non-instantiated templates!
	BOOL CommandBar_InsertMenubarEx( HWND hwndCB, HINSTANCE hInst, LPTSTR pszMenu, WORD iButton );
	BOOL CommandBar_AddAdornments( HWND hwndCB, DWORD dwFlags, DWORD dwReserved );
	void CommandBar_Destroy( HWND hwndCB );

	// mingw misses this functions
	typedef struct
	{
		HIMAGELIST himl; // Index: Normal, hot pushed, disabled. If count is less than 4, we use index 1
		RECT margin; // Margin around icon.
		UINT uAlign;
	} BUTTON_IMAGELIST, * PBUTTON_IMAGELIST;

	#define BUTTON_IMAGELIST_ALIGN_CENTER   4       // Doesn't draw text

	#define COLOR_MENUHILIGHT       29
	#define COLOR_MENUBAR           30
	#define ODS_HOTLIGHT        0x0040
	#define ODS_INACTIVE        0x0080

	// Additional (gcc, normally) stuff

	#ifndef SPI_GETUIEFFECTS
		#define SPI_GETUIEFFECTS 0x103E
	#endif //! SPI_GETUIEFFECTS

	#ifndef SPI_SETUIEFFECTS
		#define SPI_SETUIEFFECTS 0x103F
	#endif //! SPI_SETUIEFFECTS


	#ifndef GCLP_HCURSOR
		#define GCLP_HCURSOR (-12)
	#endif //! GCLP_HCURSOR

	#ifndef SetClassLong
		DWORD WINAPI SetClassLongA( HWND, INT, LONG );
		DWORD WINAPI SetClassLongW( HWND, INT, LONG );
		#ifdef UNICODE
			#define SetClassLong  SetClassLongW
		#else //! UNICODE
			#define SetClassLong  SetClassLongA
		#endif // !UNICODE
	#endif // !SetClassLong

	#ifndef SetClassLongPtr
		#ifdef _WIN64
			ULONG_PTR WINAPI SetClassLongPtrA( HWND, INT, LONG_PTR );
			ULONG_PTR WINAPI SetClassLongPtrW( HWND, INT, LONG_PTR );
			#ifdef UNICODE
				#define SetClassLongPtr  SetClassLongPtrW
			#else //! UNICODE
				#define SetClassLongPtr  SetClassLongPtrA
			#endif // !UNICODE
		#else // !_WIN64
			#define SetClassLongPtr  SetClassLong
		#endif // !_WIN64
	#endif // !SetClassLongPtr

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

	// these should be defined in CommCtrl.h, but the one in MinGW doesn't define them... (2007-11-06)
	#if (_WIN32_WINNT >= 0x0501)
	#ifndef HDF_SORTUP
	#define HDF_SORTUP              0x0400
	#endif
	#ifndef HDF_SORTDOWN
	#define HDF_SORTDOWN            0x0200
	#endif
	#ifndef LVS_EX_DOUBLEBUFFER
	#define LVS_EX_DOUBLEBUFFER     0x00010000
	#endif
	#endif

	#ifdef max
	#undef max
	#endif

	#ifdef min
	#undef min
	#endif

#endif

#endif
