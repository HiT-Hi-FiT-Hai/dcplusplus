// $Revision: 1.20 $
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

#include "boost.h"
#include "SmartUtil.h"
#include "CommandLine.h"
#include "WindowsHeaders.h"
#include "BasicTypes.h"
#include "ApplicationPlatform.h"
#include "Bitmap.h"
#include "Font.h"
#include "xCeption.h"
#include <vector>
#include <list>

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

namespace private_
{
	std::list < Widget * > & getApplicationWidgets();
}

/// HeartBeat class
/** Abstract class which classes which are supposed to be the "Heart Beat" class must
  * implement.
  */
class HeartBeat
{
public:
	/// The tick function which does the rendering and "heart beat" logic when in
	/// game mode.
	/** If you're gonna use SmartWin++ for game development you must have one class
	  * which inherits from the abstract HeartBeat class. This class must implement
	  * the tick function which is the function called by the library every "one
	  * tick". This is the place where one normally would put rendering and other
	  * "game logic" stuff. See the Tic Tac Toe application for an example
	  * implementation of this.
	  */
	virtual void tick() = 0;
	virtual ~HeartBeat()
	{}
};

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
	friend class MessageMapPolicyModalDialogWidget;
	friend std::list < Widget * > & private_::getApplicationWidgets();
public:
	// Registers a Widget meaning that the Widget is added to the applications list
	// of Widget objects
	void registerWidget( Widget * widget );

	/// Returns true if we have only one registered Widget left in the application.
	/** Used e.g. in the handling of WM_DESTROY to verify if we're supposed to return
	  * from the process or not. <br>
	  * Probably not of very much interest for the final user of the library.
	  */
		bool lastWidget( const Widget * This ) const;

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
	static void neededSmartWinInit( HINSTANCE hInstance, int nCmdShow, const char * cmdLine );

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
	const CommandLine & getCommandLine();

	/// Sets the Heart Beat object
	/** If you set this function before calling Application::run the library will go
	  * into "Game Mode" meaning that instead of waiting for next message it will
	  * peek for messages and after returning if it did find a message it will
	  * dispatch that message but anyway call your HeartBeat::tick() function after
	  * dispatching (or not!) your message. Normally the HeartBeat object would be
	  * the main Widget or something in which would be your primary rendering
	  * surface.
	  */
	void setHeartBeatFunction( HeartBeat * mainHeartBeatObject );

	/// Determine if is an application is already running or not
	/** Returns true if this application have another instance running!
	  */
	bool isAppAlreadyRunning();

	/// Since boost::signal is noncopyable we use smart pointers of signals
	typedef boost::shared_ptr < boost::signal < void () > > SignalPtr;

	/// Adds a waitable event HANDLE and the according signal
	/** You can feed in here HANDLEs of thread handles, console inputs, mutexes,
	  * processes, semaphores etc. (see Win32-API on MsgWaitForMultipleObjects) you
	  * acquired in your program, together with an according boost::signal. The
	  * signal contains the callback functions in your code which are invoked when
	  * Windows signals the HANDLE. (Since boost::signal is noncopyable we actually
	  * need here a smart pointer to the signal.)
	  */
	bool addWaitEvent( HANDLE hEvent, SignalPtr );

	/// Removes the waitable event HANDLE and the according signal
	/** Remove the event HANDLE in case we are not longer interested in being
	  * notified. (more info see addWaitEvent)
	  */
	void removeWaitEvent( HANDLE hEvent );

	/// Generates a new class name
	/** Use this function to generate a class name guaranteed to be unique within the
	  * application. The result is written back in the Seed passed. This should be
	  * the preferred method to generate class names for local classes. Global
	  * classes, on the other hand, require unique names across the operative system.
	  */
	void generateLocalClassName( Seed & );

	/// Set the class name
	/** It sets the class name to the string passed. It is useful for systemwide
	  * window classes (as opposed to local window classes). A unique name is part of
	  * the requirements to write systemwide window classes (e.g. a library of
	  * widgets to be used in other languages). The registration of a systemwide
	  * class in the operative system must be done with the appropriate style.
	  * SmartWin uses this function for those classes that do not need registration
	  * (e.g. STATIC).
	  */
	void setSystemClassName( Seed &, const SmartUtil::tstring & );

	/// Adds a class name to be unregistered
	/** Use this function to add a window class that will be unregistered when the
	  * application finishes. The class name is taken from the Seed passed.
	  */
	void addLocalWindowClassToUnregister( const Seed & );
private:
	// Unregister this classes when the application finishes
	static std::list< SmartUtil::tstring > itsClassesToUnregister;

	// To determine if a copy of an application is already running
	static HANDLE itsMutex;

	// Contains the registered Widgets of the application object
	std::list < Widget * > itsWidgets;

	// The "one and only" object of type Application...
	static Application * itsInstance;

	// The global HINSTANCE given in the WinMain function
	const HINSTANCE itsHInstance;

	// Its raw command line parameters
	const char * itsCmdLine;

	// If this one is none null we're in "Game Mode" meaning we're supposed to use
	// PeekMessage instead of GetMessage and call this callback function every
	// iteration etc...
	HeartBeat * itsHeartBeatObject;

	// We want to be notified when certain event HANDLEs become signalled by Windows.
	// Those handles go in this vector.
	std::vector< HANDLE > itsVHEvents;

	// The according signals we must raise, go in this vector.
	std::vector< SignalPtr > itsVSignals;

	// Private Constructor to ensure Singleton Implementation
	Application( HINSTANCE hInst, int nCmdShow, const char * cmdLine );

	// Since the Constructor needs parameters we need to have a static Constructor
	// which takes those parameters (Module Handle and show params)
	static void Instantiate( HINSTANCE hInst, int nCmdShow, const char * cmdLine = 0 );

	// Cleaning up...
	static void UnInstantiate();
};

// end namespace SmartWin
}

#endif
