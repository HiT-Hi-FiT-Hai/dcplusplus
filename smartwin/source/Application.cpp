// $Revision: 1.45 $
/*
  Copyright ( c ) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met :

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
  ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "../include/smartwin/Application.h"
#include "../SmartUtil/tstring.h"
#include "../include/smartwin/BasicTypes.h"
#include "../include/smartwin/aspects/AspectMouseClicks.h"
#include "../include/smartwin/aspects/AspectSizable.h"

#include <boost/lexical_cast.hpp>

using namespace SmartWin;

// link to Common Controls to relieve user of explicitly doing so
#ifdef _MSC_VER
#ifdef WINCE
#pragma comment( lib, "commctrl.lib" )
#else
#pragma comment( lib, "comctl32.lib" )
#endif //! WINCE
#endif //! _MSC_VER

// Friend functions to Application 

Application * Application::itsInstance = 0;
HANDLE Application::itsMutex = 0;

// Declaring (externally) the function needed to be supplied as entry point by Client Applications utilizing the library
// This function is being called by the internal entry point (WinMain)
extern int SmartWinMain( Application & app );
#ifdef WINCE
int WINAPI ::WinMain
	( HINSTANCE hInstance
	, HINSTANCE hPrevInstance
	, LPTSTR lpCmdLine
	, int nCmdShow
	)
#else
int PASCAL WinMain
	( HINSTANCE hInstance
	, HINSTANCE hPrevInstance
	, LPSTR lpCmdLine
	, int nCmdShow
	)
#endif
{
	unsigned int retVal = 0;
	bool corruptMemMemLeak = false;
	
	Application::init( hInstance, nCmdShow );
	try
	{
		retVal = SmartWinMain( Application::instance() ); // Call library user's startup function.
		Application::uninit();

		Application::checkCorruptOrMemleak( corruptMemMemLeak );
	}
	catch ( xCeption & err )
	{
		retVal = Application::reportErr( err, corruptMemMemLeak );
	}
	return retVal;
}


// Application implementation

/** Initializes the runtime for SmartWin++
	Typically only called by WinMain or DllMain.
  */
void Application::init( HINSTANCE hInstance, int nCmdShow )
{
	itsInstance = new Application( hInstance, nCmdShow );

#ifndef WINCE
	BOOL enable;
	::SystemParametersInfo( SPI_GETUIEFFECTS, 0, & enable, 0 );
	if ( ! enable ) {
		enable = TRUE;
		::SystemParametersInfo( SPI_SETUIEFFECTS, 0, & enable, 0 );
	}
#endif

	// Initializing Common Controls...
	INITCOMMONCONTROLSEX init;
	init.dwSize = sizeof( INITCOMMONCONTROLSEX );
	init.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_DATE_CLASSES | ICC_PROGRESS_CLASS;
	::InitCommonControlsEx( & init );

#ifdef _MSC_VER
#ifndef WINCE
#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF ); // Show heap leaks at exit, to debug window.
#endif
#endif
#endif
}

void Application::checkCorruptOrMemleak( bool & corruptMemMemLeak )
{
	corruptMemMemLeak = false;
#ifdef _MSC_VER
#ifndef WINCE
#ifdef _DEBUG
	corruptMemMemLeak = _CrtCheckMemory() != TRUE; // Check for corruption right now.
	xAssert( ! corruptMemMemLeak, _T( "The application has corrupted its heap memory." ) );
#endif
#endif
#endif
}

unsigned int Application::reportErr( xCeption & err, bool corruptMemMemLeak )
{
	unsigned int retVal;

	if ( corruptMemMemLeak )
		retVal = 0xdeadbeef;
	else
		retVal = err.getErrorCode();

#ifdef _MSC_VER
#ifndef WINCE
#ifdef _DEBUG
	_RPT0( _CRT_WARN, "\r\n" );
	_RPT0( _CRT_WARN, "*** ERROR ***\r\n" );
	_RPT0( _CRT_WARN, err.what() );
	_RPT0( _CRT_WARN, "\r\n" );
	if ( ! corruptMemMemLeak )
	{
		_RPT0( _CRT_WARN, err.whatWndMsg() );
		_RPT0( _CRT_WARN, "\r\n" );
	}
	_RPT0( _CRT_WARN, "*** END OF ERROR ***" );
	_RPT0( _CRT_WARN, "\r\n" );
#endif
#endif
#endif
	return retVal;
}

HINSTANCE Application::getAppHandle()
{
	return itsHInstance;
}

Application::Application( HINSTANCE hInst, int nCmdShow )
	: itsHInstance( hInst ), itsCmdShow(nCmdShow)
{
}

const CommandLine & Application::getCommandLine() const
{
	return itsCmdLine;
}

bool Application::isAppAlreadyRunning()
{
	SmartUtil::tstring appPath = getModulePath() + getModuleFileName();
	itsMutex = ::CreateMutex( NULL, FALSE, appPath.c_str() );
	if ( !itsMutex )
		return false;
	if ( ::GetLastError() == ERROR_ALREADY_EXISTS ) {
		::CloseHandle( itsMutex );
		itsMutex = 0;
		return true;
	} else {
		// We were the first one to create the mutex
		// so that makes us the main instance.
		return false;
	}
}

bool Application::addWaitEvent( HANDLE hWaitEvent, Application::SignalPtr pSignal )
{
	// in case the maximum number of objects is already achieved return false
	if ( itsVHEvents.size() >= MAXIMUM_WAIT_OBJECTS - 1 )
		return false;

	if ( hWaitEvent != INVALID_HANDLE_VALUE )
	{
		itsVSignals.push_back( pSignal );
		itsVHEvents.push_back( hWaitEvent );
	}
	return true;
}

void Application::removeWaitEvent( HANDLE hWaitEvent )
{
	if ( hWaitEvent != INVALID_HANDLE_VALUE )
	{
		std::vector< Application::SignalPtr >::iterator pSig;
		std::vector< HANDLE >::iterator pH;
		for ( pSig = itsVSignals.begin(), pH = itsVHEvents.begin();
			pSig != itsVSignals.end(); pSig++, pH++ )
		{
			if ( * pH == hWaitEvent )
			{
				itsVSignals.erase( pSig );
				itsVHEvents.erase( pH );
				break;
			}
		}
	}
}

void Application::uninit()
{
	delete itsInstance;
	itsInstance = 0;
	if ( itsMutex )
	{
		::CloseHandle( itsMutex );
		itsMutex = 0;
	}
}

Application & Application::instance()
{
	if ( 0 == itsInstance )
	{
		init( ::GetModuleHandle( NULL ), 0 );
	}
	return * itsInstance;
}

SmartUtil::tstring Application::getModulePath() const
{
	TCHAR retVal[2049];
	GetModuleFileName( 0, retVal, 2048 );
	SmartUtil::tstring retStr = retVal;
	retStr = retStr.substr( 0, retStr.find_last_of( '\\' ) + 1 );
	return retStr;
}

SmartUtil::tstring Application::getModuleFileName() const
{
	TCHAR retVal[2049];
	return SmartUtil::tstring(retVal, GetModuleFileName(0, retVal, 2048));
}

#ifdef __WINE__
// Because linux applications became stuck in the MsgWaitForMultipleObjectsEx call
// when the WM_CLOSE was sent, we restore the simpler message loops for __WINE__.
//
int Application::run()
{
	MSG msg;
	while ( ::GetMessage( & msg, 0, 0, 0 ) > 0 )
	{
		if ( ::IsDialogMessage( GetParent( msg.hwnd ), & msg ) ||
				TranslateMDISysAccel( msg.hwnd, & msg ) )
		{
			continue; // Allow Tab order
		}

		::TranslateMessage ( & msg );
		::DispatchMessage ( & msg );
	}
	return static_cast< int >( msg.wParam );
}
#else

//
// This routine works excellent in single threaded environments, but I suspect it will fail if client wants
// to create more than one thread(e.g. WaitMessage waits WITHIN thread), probably need to refactor to get multiple threads to work
int Application::run()
{
	// Checking if idx window was the window that had a message
	MSG msg;

	 /* This message loop is event aware and was inspired by an example
	  * in Jeffrey Richter's book "Advanced Windows" (from 1996?)
	  * (see Fig. 14-3 or file 'FileChng.c')
	  */
	bool bQuit = false;
	while ( !bQuit )
	{
		 /* MsgWaitForMultipleObjects behaves like GetMessage except that it is also
		  * sensitive to waitable event HANDLEs.
		  */
		DWORD dwWaitResult = MsgWaitForMultipleObjectsEx(
			static_cast< DWORD >( itsVHEvents.size() ),
			( itsVHEvents.size() > 0 ) ? & itsVHEvents[0] : 0,
			INFINITE,
			QS_ALLINPUT,
			0 );

		if ( dwWaitResult < WAIT_OBJECT_0 + itsVHEvents.size() )
		{
			// the wait event was signalled by Windows
			// signal its handlers
			( * itsVSignals[dwWaitResult - WAIT_OBJECT_0] )();
		}
		else if ( dwWaitResult == WAIT_OBJECT_0 + itsVHEvents.size() )
		{
			while ( PeekMessage( & msg, NULL, 0, 0, PM_REMOVE ) )
			{
				bool filtered = false;
				for(FilterIter i = filters.begin(); i != filters.end(); ++i) {
					if((*i)(msg)) {
						filtered = true;
						break;
					}
				}
				
				if(filtered) {
						continue;
				}

				if ( msg.message == WM_QUIT )
				{
					bQuit = true;
				}
				else
				{
					::TranslateMessage ( & msg );
					::DispatchMessage ( & msg );
				}
			}
		}
		else if ( dwWaitResult < WAIT_ABANDONED_0 + itsVHEvents.size() )
		{
			SmartUtil::tstring strX =
				_T( "Application::run : Encountered an abandoned wait mutex object (index " );
			strX += boost::lexical_cast< SmartUtil::tstring >( dwWaitResult - WAIT_ABANDONED_0 );
			strX += _T( " )." );

			throw xCeption( strX );
		}
		else if ( dwWaitResult != WAIT_TIMEOUT )
		{
			throw xCeption( _T( "Application::run : MsgWaitForMultipleObjects() failed!" ) );
		}
	}
	return static_cast< int >( msg.wParam );
}

Application::FilterIter Application::addFilter(const FilterFunction& f) {
	return filters.insert(filters.end(), f);
}

void Application::removeFilter(const FilterIter& i) {
	filters.erase(i);
}

#endif // not __WINE__

WidgetSizedEventResult::WidgetSizedEventResult( WPARAM wP, LPARAM lP )
{
	newSize = Point( GET_X_LPARAM( lP ), GET_Y_LPARAM( lP ) );
	isMaximized = ( wP == SIZE_MAXIMIZED );
	isMinimized = ( wP == SIZE_MINIMIZED );
	isRestored = ( wP == SIZE_RESTORED );
}

MouseEventResult::MouseEventResult( WPARAM wP, LPARAM lP )
{
	isAltPressed = ::GetKeyState( VK_MENU ) < 0;
	pos.x = GET_X_LPARAM( lP );
	pos.y = GET_Y_LPARAM( lP );
	isControlPressed = ( ( wP & MK_CONTROL ) == MK_CONTROL );
	isShiftPressed = ( ( wP & MK_SHIFT ) == MK_SHIFT );

	// These might be an issue when porting to Windows CE since CE does only support LEFT (or something...)
	ButtonPressed = (
		MK_LBUTTON & wP ? MouseEventResult::LEFT : (
			MK_RBUTTON & wP ? MouseEventResult::RIGHT : (
				MK_MBUTTON & wP ? MouseEventResult::MIDDLE : (
					MK_XBUTTON1 & wP ? MouseEventResult::X1 : (
						MK_XBUTTON2 & wP ? MouseEventResult::X2 : MouseEventResult::OTHER
					)
				)
			)
		)
	);
}
