// $Revision: 1.8 $
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
#ifndef AspectThreads_h
#define AspectThreads_h

namespace SmartWin
{
// begin namespace SmartWin

// TODO: Outfactor the rest of the implementation
/// Aspect class used by Widgets that have the possibility of creating a fork or a
/// new thread of execution
/** \ingroup AspectClasses
  * E.g. the WidgetWindow have a Threads Aspect to it therefore WidgetWindow realize
  * the AspectThreads through inheritance. Most Widgets realize this Aspect since
  * they can fork the current thread of execution into two threads.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectThreads
{
	template< class Param >
	class ThreadParam
	{
	public:
		typedef unsigned long ( EventHandlerClass::* ThreadProc )( Param & );
		typedef unsigned long ( EventHandlerClass::* ThreadProcVoid )();

		Param itsPar;
		ThreadProc itsThreadProc;
		ThreadProcVoid itsThreadProcVoid;
		EventHandlerClass * itsThis;

		ThreadParam( const Param & par, ThreadProc threadProc, EventHandlerClass * This )
			: itsPar( par ), itsThreadProc( threadProc ), itsThis( This )
		{}
		ThreadParam( const Param & par, ThreadProcVoid threadProc, EventHandlerClass * This )
			: itsPar( par ), itsThreadProcVoid( threadProc ), itsThis( This )
		{}
	};

	template< class Param >
	class ThreadParamGlobal
	{
	public:
		typedef unsigned long ( * ThreadProc )( EventHandlerClass *, Param & );
		typedef unsigned long ( * ThreadProcVoid )( EventHandlerClass * );

		Param itsPar;
		ThreadProc itsThreadProc;
		ThreadProcVoid itsThreadProcVoid;
		EventHandlerClass * itsThis;

		ThreadParamGlobal( const Param & par, ThreadProc threadProc, EventHandlerClass * This )
			: itsPar( par ), itsThreadProc( threadProc ), itsThis( This )
		{}
		ThreadParamGlobal( const Param & par, ThreadProcVoid threadProc, EventHandlerClass * This )
			: itsPar( par ), itsThreadProcVoid( threadProc ), itsThis( This )
		{}
	};

	template< class Param >
	static unsigned long WINAPI forkDelegator( void * params )
	{
		unsigned long retVal = - 1;
		ThreadParam< Param > * parameter = reinterpret_cast< ThreadParam< Param > * >( params );
		try
		{
			retVal =
				( ( * static_cast< EventHandlerClass * >( parameter->itsThis ) ).*parameter->itsThreadProc )
				( parameter->itsPar );
			delete parameter;
			parameter = 0;
		}
		catch ( ... )
		{
			delete parameter;
			throw; // TODO: Dangerous probably, but then again, what should we do...?
		}
		return retVal;
	}

	template< class Param >
	static unsigned long WINAPI forkDelegatorGlobal( void * params )
	{
		unsigned long retVal = - 1;
		ThreadParamGlobal< Param > * parameter = reinterpret_cast< ThreadParamGlobal< Param > * >( params );
		try
		{
			retVal =
				parameter->itsThreadProc
				( parameter->itsThis, parameter->itsPar );
			delete parameter;
			parameter = 0;
		}
		catch ( ... )
		{
			delete parameter;
			throw; // TODO: Dangerous probably, but then again, what should we do...?
		}
		return retVal;
	}

	template< class Param >
	static unsigned long WINAPI forkDelegatorVoid( void * params )
	{
		unsigned long retVal = - 1;
		ThreadParam< Param > * parameter = reinterpret_cast< ThreadParam< Param > * >( params );
		try
		{
			retVal =
				( ( * static_cast< EventHandlerClass * >( parameter->itsThis ) ).*parameter->itsThreadProcVoid )();
			delete parameter;
			parameter = 0;
		}
		catch ( ... )
		{
			delete parameter;
			throw; // TODO: Dangerous probably, but then again, what should we do...?
		}
		return retVal;
	}

	template< class Param >
	static unsigned long WINAPI forkDelegatorGlobalVoid( void * params )
	{
		unsigned long retVal = - 1;
		ThreadParamGlobal< Param > * parameter = reinterpret_cast< ThreadParamGlobal< Param > * >( params );
		try
		{
			retVal =
				parameter->itsThreadProcVoid
				( parameter->itsThis );
			delete parameter;
			parameter = 0;
		}
		catch ( ... )
		{
			delete parameter;
			throw; // TODO: Dangerous probably, but then again, what should we do...?
		}
		return retVal;
	}

public:
	/// Creates a new thread and passes the given parameter
	/** Forks execution into a member function with the given parameter of the given
	  * type. If you wish to later manipulate the thread in some way use the
	  * Utilities::Thread class to store the  return value of this function.
	  */
	template< class Param >
		Utilities::Thread fork( const Param & par, unsigned long ( EventHandlerClass::* threadProc )( Param & ) )
	{
		ThreadParam< Param > * parameter = new ThreadParam< Param >( par, threadProc, static_cast< EventHandlerClass * >( this ) );
		return Utilities::Thread( ::CreateThread(
			NULL,
			0,
			forkDelegator< Param >,
			reinterpret_cast< void * >( parameter ),
			0,
			NULL ) );
	}

	template< class Param >
		Utilities::Thread fork( const Param & par, unsigned long ( * threadProc )( EventHandlerClass *, Param & ) )
	{
		ThreadParamGlobal< Param > * parameter = new ThreadParamGlobal< Param >( par, threadProc, static_cast< EventHandlerClass * >( this ) );
		return Utilities::Thread( ::CreateThread(
			NULL,
			0,
			forkDelegatorGlobal< Param >,
			reinterpret_cast< void * >( parameter ),
			0,
			NULL ) );
	}

	/// Creates a new thread on the given function
	/** Forks execution into a member function. If you wish to later manipulate the
	  * thread in some way use the Utilities::Thread class to store the return value
	  * of this function.
	  */
	Utilities::Thread fork( unsigned long ( EventHandlerClass::* threadProc )() )
	{
		ThreadParam< int > * parameter = new ThreadParam< int >( 0, threadProc, static_cast< EventHandlerClass * >( this ) );
		return Utilities::Thread( ::CreateThread(
			NULL,
			0,
			forkDelegatorVoid< int >,
			reinterpret_cast< void * >( parameter ),
			0,
			NULL ) );
	}

		Utilities::Thread fork( unsigned long ( * threadProc )( EventHandlerClass * ) )
	{
		ThreadParamGlobal< int > * parameter = new ThreadParamGlobal< int >( 0, threadProc, static_cast< EventHandlerClass * >( this ) );
		return Utilities::Thread( ::CreateThread(
			NULL,
			0,
			forkDelegatorGlobalVoid< int >,
			reinterpret_cast< void * >( parameter ),
			0,
			NULL ) );
	}

protected:
	virtual ~AspectThreads()
	{}
};

// end namespace SmartWin
}

#endif
