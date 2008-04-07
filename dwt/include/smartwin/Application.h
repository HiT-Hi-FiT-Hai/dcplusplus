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
#ifndef Application_h
#define Application_h

#include "WindowsHeaders.h"
#include "../SmartUtil.h"
#include "CommandLine.h"
#include "ApplicationPlatform.h"
#include "xCeption.h"
#include <vector>
#include <list>
#include <map>
#include <boost/noncopyable.hpp>
#include "Message.h"

#ifdef _MSC_VER
#ifndef WINCE
#ifdef _DEBUG
// Enable memory leak detection with file/line tracing.
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif
#endif

/// Contains all classes used in SmartWin
namespace SmartWin
{
// begin namespace SmartWin

// Forward declaration of friends
class Widget;

namespace Policies {
class ModelessDialog;
}

/// Class declaration for the application class
/** Every application using SmartWin will get ONE and ONLY one Application object
  * automatically. <br>
  * Class is a "natural" Singleton (see GoF "Design Patterns") which means that you
  * must access <br>
  * that one object through a static function. <br>
  * To get to the Application object use the static member function
  * Application::instance() <br>
  * The Application object contains several useful functions, e.g. the getModulePath
  * and the getCommandLine functions which in turn loads a bitmap and returns the
  * handle to it, retrieves the path to the physical directory of the exe file and
  * retrieves a vector of command line parameters. <br>
  * The Application class inherits from boost::noncopyable to indicate it's not to be
  * copied
  */
class Application
	: public ApplicationPlatform< CurrentPlatform >,
	public boost::noncopyable
{
#ifdef WINCE
	friend int WINAPI ::WinMain( HINSTANCE hInstance,
				HINSTANCE hPrevInstance,
				LPTSTR lpCmdLine,
				int nCmdShow );
#else
	friend int PASCAL ::WinMain( HINSTANCE hInstance,
				HINSTANCE hPrevInstance,
				LPSTR lpCmdLine,
				int nCmdShow );
#endif

	friend class Widget;
	friend class Policies::ModelessDialog;
public:
	/// Returns the Application object
	/** Use this static function to access the Application object.
	  */
	static Application & instance();

	/// Returns the HINSTANCE to the process
	/** Returns the handle to the process.
	  */
	HINSTANCE getAppHandle();

	/// Returns the path to the process
	/** NOTE! <br>
	  * This function returns the PATH to the application WITHOUT the process image
	  * name. <br>
	  * If you want to have the path AND the FILENAME of the image process you must
	  * use Application::getModuleFileName.
	  */
	SmartUtil::tstring getModulePath() const;

	/// Returns the full filename to the process
	/** NOTE! <br>
	  * This function returns the full filename to the application WITH the process
	  * image name. <br>
	  * If you want to have only the path you must use Application::getModulePath.
	  */
	SmartUtil::tstring getModuleFileName() const;

	typedef std::tr1::function<bool (MSG&)> FilterFunction;
	// List becuse its iterators aren't invalidated on add/delete...
	typedef std::list<FilterFunction> FilterList;
	typedef FilterList::iterator FilterIter;
	
	FilterIter addFilter(const FilterFunction& f);
	void removeFilter(const FilterIter& i);
	
	/// Starts the application
	/** Normally this function will be called from your
	  * "SmartWinMain(SmartWin::Application & app )" function as the last function.
	  * <br>
	  * E.g. return myApp.run();
	  */
	int run();

	/// The initialization that must be done first.
	/** Used internally by the WinMain function, and externally for DLL initialization.
	  */
	static void init( HINSTANCE hInstance, int nCmdShow );
	
	/// Shut down operations
	static void uninit();

	/// Calls various leak and memory corruption routines.
	/** Used after the message loop completes in WinMain.
	  */
	static void checkCorruptOrMemleak( bool & corruptMemMemLeak );

	/// Shows the xCeption.
	/** Used if an exception occurs in the message loop in WinMain.
	  */
	static unsigned int reportErr( xCeption & err, bool corruptMemMemLeak );

	/// Returns the command line object
	/** The command line object contains the parameters given to the process during
	  * startup. <br>
	  * Use this function to retrieve the command line object
	  */
	const CommandLine & getCommandLine() const;
	
	int getCmdShow() const;

	/// Determine if is an application is already running or not
	/** Returns true if this application have another instance running!
	  */
	bool isAppAlreadyRunning();

	typedef std::tr1::function<void()> Callback;
	
	/// Adds a waitable event HANDLE and the according signal
	/** You can feed in here HANDLEs of thread handles, console inputs, mutexes,
	  * processes, semaphores etc. (see Win32-API on MsgWaitForMultipleObjects) you
	  * acquired in your program, together with an according boost::signal. The
	  * signal contains the callback functions in your code which are invoked when
	  * Windows signals the HANDLE. (Since boost::signal is noncopyable we actually
	  * need here a smart pointer to the signal.)
	  */
	bool addWaitEvent( HANDLE hEvent, const Callback& );

	/// Removes the waitable event HANDLE and the according signal
	/** Remove the event HANDLE in case we are not longer interested in being
	  * notified. (more info see addWaitEvent)
	  */
	void removeWaitEvent( HANDLE hEvent );

private:
	// To determine if a copy of an application is already running
	static HANDLE itsMutex;

	// The "one and only" object of type Application...
	static Application * itsInstance;

	// The global HINSTANCE given in the WinMain function
	const HINSTANCE itsHInstance;

	int itsCmdShow;
	
	// Command line parameters
	CommandLine itsCmdLine;

	// We want to be notified when certain event HANDLEs become signalled by Windows.
	// Those handles go in this vector.
	std::vector< HANDLE > itsVHEvents;

	// The according signals we must raise, go in this vector.
	std::vector< Callback > itsVSignals;
	
	FilterList filters;
	
	// Private Constructor to ensure Singleton Implementation
	Application( HINSTANCE hInst, int nCmdShow );
};

// end namespace SmartWin
}

#endif
