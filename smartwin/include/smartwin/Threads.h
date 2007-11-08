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
#ifndef Threads_h
#define Threads_h

#include "WindowsHeaders.h"
#include <vector>

namespace SmartWin
{
// begin namespace SmartWin

namespace Utilities
{
// begin namespace Utilities

/// Class to ensure serialized access to resources shared among several threads.
/** Use this class to ensure serialized access to objects shared among multiple
  * threads. To use instantiate and call lock which will deny other executing threads
  * to be able to return after calling lock before the first thread have called
  * unlock. <br>
  * Related classes and functions.
  * <ul>
  * <li>class CriticalSection</li>
  * <li>class Thread</li>
  * <li>function Widget::getCriticalSection()</li>
  * </ul>
  */
class CriticalSection
{
	CRITICAL_SECTION itsCs;
	bool itsIsLocked;

	// DENY copying
	CriticalSection( const CriticalSection & );

public:
	/// Initializes critical section thread semantics
	CriticalSection()
		: itsIsLocked( false )
	{
		::InitializeCriticalSection( & itsCs );
	}

	// Deletes the critical section object
	~CriticalSection()
	{
		::DeleteCriticalSection( & itsCs );
	}

	/// Lock the section
	/** Only the first thread calling lock will return immediately, all other threads
	  * calling lock will be HALTED until the first thread have called unlock on the
	  * object.
	  */
	void lock()
	{
		::EnterCriticalSection( & itsCs );
		itsIsLocked = true;
	}

	/// Unlocks the section
	/** No other thread will be able to return after calling lock until the thread
	  * who first called lock on the object have called unlock on the object.
	  */
	void unlock()
	{
		::LeaveCriticalSection( & itsCs );
		itsIsLocked = false;
	}

	/// Checks if object is locked
	/** Returns true if somebody have aquired a lock on the object
	  */
	bool isLocked()
	{
		return itsIsLocked;
	}
};

/// Helper class for aquiring a lock on a CriticalSection
/** Encapsulates lock in Constructor and unlock in DTOR to have RAII semantics on
  * CriticalSection locking Related classes and functions.
  * <ul>
  * <li>class CriticalSection</li>
  * <li>class Thread</li>
  * <li>function Widget::getCriticalSection()</li>
  * </ul>
  */
class ThreadLock
{
	CriticalSection & itsCs;

	// DENY copying
	ThreadLock( const ThreadLock & rhs );

public:
	/// Instantiate with a CriticalSection and it will automatically be locked and
	/// freed in DTOR
	ThreadLock( CriticalSection & cs )
		: itsCs( cs )
	{
		itsCs.lock();
	}

	~ThreadLock()
	{
		itsCs.unlock();
	}
};

/// Thread class, encapsulates a thread, its properties and its functions
/** This class encapsulates a thread and its functions. To actually create and start
  * a thread use the fork function of the Widget you wish to create the thread
  * within...
  * <ul>
  * <li>class CriticalSection</li>
  * <li>class ThreadLock</li>
  * <li>function Widget::getCriticalSection</li>
  * <li>function Widgetxxx::fork</li>
  * </ul>
  */
class Thread
{
	HANDLE itsThreadHandle;

public:
	/// Constructor taking a thread handle
	explicit Thread( HANDLE thread )
		: itsThreadHandle( thread )
	{}

	/// Empty Constructor, doesn't alloc any resources or anything...
	Thread()
		: itsThreadHandle( 0 )
	{}

	/// Wait for multiple threads to finish
	/** If you have for instance 3 different threads that must finish their current
	  * work before you can continue execution in another thread then call this
	  * function with the threads to wait for as the contents of the vector. Note!
	  * <br>
	  * If the thread you are waiting for is manipulating GUI in some way then you
	  * are likely to experience a deadlock here if you call this one on the main
	  * thread since the message handling will then be halted (since you called it on
	  * the main thread)
	  */
	static void waitForObjects( const std::vector< Thread > & threads )
	{
		HANDLE * handles = new HANDLE[threads.size()];
		try
		{
			int idx = 0;
			for ( std::vector< Thread >::const_iterator iter = threads.begin();
				iter != threads.end();
				++iter )
			{
				handles[idx++] = iter->itsThreadHandle;
			}
			::WaitForMultipleObjects( static_cast< DWORD >( threads.size() ), handles, TRUE, INFINITE );
			delete [] handles;
		}
		catch ( ... )
		{
			delete [] handles;
		}
	}

	static void waitForObject( const Thread & thread )
	{
		::WaitForSingleObject( thread.itsThreadHandle, INFINITE );
	}

	/// Suspends the thread until resume is called on the thread again.
	/** Basically pauses the thread until resume is called on the thread
	  */
	void suspend()
	{
		::SuspendThread( itsThreadHandle );
	}

	/// Resumes a thread again after suspend
	/** Call suspend to stop the thread from executing and call resume to get it
	  * running again
	  */
	void resume()
	{
		::ResumeThread( itsThreadHandle );
	}

	/// Terminates the thread !! (CAUTION)
	/** WARNING!!! <br>
	  * This function will IMMEDIATELY terminate the thread, this means that the
	  * thread will NOT get to do ANY house cleaning at ALL. The thread will
	  * IMMEDIATELY be terminated and it will not get to do anything at all, for
	  * instance it will not be able to delete its heaped memory or finish its
	  * current line of execution etc... This is a LAST RESORT function only meant in
	  * very critical situations where you really need to terminate the thread and
	  * kill its execution!
	  */
	void terminate( unsigned long exitCode )
	{
		::TerminateThread( itsThreadHandle, exitCode );
	}
};

// end namespace Utilities
}

// end namespace SmartWin
}

#endif
