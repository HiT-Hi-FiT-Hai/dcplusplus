// $Revision: 1.30 $
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
#ifndef WidgetWindow_h
#define WidgetWindow_h

#include "WidgetWindowBase.h"
#include "../aspects/AspectThreads.h"
#include <sstream>

namespace SmartWin
{
// begin namespace SmartWin

/// "Window" class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html window.PNG
  * Class for creating a "normal" window. <br>
  * A normal window is what most applications would use as the basic main structure, 
  * in it you can add other Widgets like for instance buttons, comboboxes etc. <br>
  * A WidgetWindow is basically a container Widget for other Widgets to reside in. 
  * <br>
  * Class is a public superclass of WidgetWindowBase and therefore can use all 
  * features of WidgetWindowBase.   
  */
template< class EventHandlerClass, class MessageMapPolicy = MessageMapPolicyNormalWidget >
class WidgetWindow
	: public WidgetWindowBase< EventHandlerClass, MessageMapPolicy >
{
	typedef typename WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::ThisMessageMap ThisMessageMap;
	typedef typename WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::ThisMessageMap MessageMapType;
public:
	/// Class type
	typedef WidgetWindow< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetWindow< EventHandlerClass, MessageMapPolicy > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetWindow::ThisType WidgetType;

		//TODO: put variables to be filled here
		HICON icon;
		HBRUSH background;
		SmartUtil::tstring menuName;
		HCURSOR cursor;

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	/// Actually creates the window
	/** This one creates the window. <br>
	  * All WidgetWindows, and classes derived from them must create the Window
	  * before using it with functions such as setBounds() or setVisible( false ). <br>
      * The simple version "createWindow()" uses a default Seed for the window attributes.
	  * The seed is not taken a constant because the class name will be generated at registration.
	  */
	virtual void createWindow( Seed = getDefaultSeed() );

	/// Creates an invisible window, for quiet initialization.
	/** Same as createWindow, except that the window lacks WS_VISIBLE.
	  * Since you must create the window before you add other Widgets,
	  * and doing so causes a bit of screen flash before the final window
	  * is ready, createInvisibleWindow() lets you add Widgets while
	  * the main Widget is not visible.  Of course you could do code like <br>
	  *
	  *   Seed defInvisible = getDefaultSeed(); <br>
	  *   defInvisible.style= defInvisible.style & ( ~ WS_VISIBLE ); <br>
	  *   createWindow( defInvisible ); <br>
	  *
	  * but this is cleaner: <br>
	  *
	  *   createInvisibleWindow(); <br>
	  *   do init <br>
	  *   setVisible( true ); <br>
	  *
	  * The other styles are either defaulted with createInvisibleWindow()
	  * or specified with createInvisibleWindow( Seed ).
	  */
	virtual void createInvisibleWindow( Seed = getDefaultSeed() );



	// TODO: Check up if the CREATESTRUCT * actualy IS modyfiable...!!
	/// Setting the event handler for the "create" event
	/** The event handler must have the signature "void foo( CREATESTRUCT * )" where
	  * the WidgetType is the type of Widget that realizes the Aspect. <br>
	  * If you supply an event handler for this event your handler will be called 
	  * when Widget is initially created. <br>
	  * The argument CREATESTRUCT sent into your handler is modifiable so that you 
	  * can add/remove styles and add remove EX styles etc.       
	  */
#ifdef _MSC_VER
	void onCreate( itsVoidFunctionTakingSeedPointer eventHandler );
	void onCreate( voidFunctionTakingSeedPointer eventHandler );
#endif

#ifdef __GNUC__
	void onCreate( typename ThisMessageMap::itsVoidFunctionTakingSeedPointer eventHandler );
	void onCreate( typename ThisMessageMap::voidFunctionTakingSeedPointer eventHandler );
#endif




	// TODO: Fix! This doesn't work if you have an application with multiple windows in addition to that it's a member method plus lots of other architectual issues!!
	// TODO: Add support in the Seed class to override "class name" and then we can do a lookup upon that name given...??
	// TODO: In addition make this a NON member function!!
	/// Activates already running instance of an application
	/** Note that this only works if you have only one SmartWin application running!
	  */
	void activatePreviousInstance();

protected:
	// Protected since this Widget we HAVE to inherit from
	explicit WidgetWindow( Widget * parent = 0 );

	virtual ~WidgetWindow();
private:
	SmartUtil::tstring itsRegisteredClassName;
};

template< class EventHandlerClass, class MessageMapPolicy = MessageMapPolicyNormalWidget >
class WidgetChildWindow
	: public WidgetWindow< EventHandlerClass, MessageMapPolicy >
{
public:
	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public WidgetWindow< EventHandlerClass, MessageMapPolicy >::Seed
	{
	public:
		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	//TODO: This could be specialized to take WNDPROC from MessageMapPolicy
	/// Actually creates the window
	/** This one creates the window. It is implemented in case somebody wants to use
	  * createWindow() without parameters. If it wasn't declared, the compiler would
	  * call WidgetWindow::create with WidgetWindow::getDefaultSeed, which wouldn't
	  * create a child window.
	  */
	virtual void createWindow( Seed cs = getDefaultSeed() )
	{
		WidgetWindow< EventHandlerClass, MessageMapPolicy >::createWindow( cs );
	}

protected:
	// Unlike WidgetWindow, WidgetChildWindow must have a parent!!!
	explicit WidgetChildWindow( Widget * parent ) : WidgetWindow< EventHandlerClass, MessageMapPolicy >( parent ) //Long name to satisfy devcpp
	{};
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetWindow< EventHandlerClass, MessageMapPolicy >::Seed & WidgetWindow< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
		d_DefaultValues.background = ( HBRUSH )( COLOR_BTNFACE + 1 );
		d_DefaultValues.caption = _T( "" );
#ifndef WINCE
		d_DefaultValues.cursor = LoadCursor( 0, IDC_ARROW );
		d_DefaultValues.icon = LoadIcon( 0, IDI_APPLICATION );
#else
		d_DefaultValues.cursor = 0;
		d_DefaultValues.icon = 0;
#endif
		d_DefaultValues.menuName = _T( "" ); //TODO: does menu &"" work as good as menu NULL ?

		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetWindow< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetWindow::getDefaultSeed();
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetWindow< EventHandlerClass, MessageMapPolicy >::WidgetWindow( Widget * parent )
	: WidgetWindowBase< EventHandlerClass, MessageMapPolicy >( parent )
{}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetWindow< EventHandlerClass, MessageMapPolicy >::~WidgetWindow()
{
	::UnregisterClass( itsRegisteredClassName.c_str(), Application::instance().getAppHandle() );
}


template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindow< EventHandlerClass, MessageMapPolicy >::createInvisibleWindow( Seed cs )
{
	cs.style=  cs.style & ( ~ WS_VISIBLE );
	WidgetWindow< EventHandlerClass, MessageMapPolicy >::createWindow( cs );
}


template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindow< EventHandlerClass, MessageMapPolicy >::createWindow( Seed cs )
{
	Application::instance().generateLocalClassName( cs );
	itsRegisteredClassName = cs.getClassName();

	SMARTWIN_WNDCLASSEX ws;

#ifndef WINCE
	ws.cbSize = sizeof( SMARTWIN_WNDCLASSEX );
#endif //! WINCE
	// This are window class styles, not window styles ...
	ws.style = CS_DBLCLKS;	// Allow double click messages
	ws.lpfnWndProc = MessageMapPolicy::mainWndProc_;
	ws.cbClsExtra = 0;
	ws.cbWndExtra = 0;
	ws.hInstance = Application::instance().getAppHandle();
#ifdef WINCE
	ws.hIcon = 0;
#else
	ws.hIcon = cs.icon;
#endif //! WINCE
	ws.hCursor = cs.cursor;
	ws.hbrBackground = cs.background;
	ws.lpszMenuName = cs.menuName.empty() ? 0 : cs.menuName.c_str();
	ws.lpszClassName = itsRegisteredClassName.c_str();
#ifndef WINCE
	//TODO: fix this
	ws.hIconSm = cs.icon;
#endif //! WINCE

	ATOM registeredClass = SmartWinRegisterClass( & ws );
	if ( 0 == registeredClass )
	{
		xCeption x( _T( "WidgetWindowBase.createWindow() SmartWinRegisterClass fizzled..." ) );
		throw x;
	}
	Application::instance().addLocalWindowClassToUnregister( cs );
	Widget::create( cs );
}


template< class EventHandlerClass, class MessageMapPolicy >
#ifdef _MSC_VER
void WidgetWindow< EventHandlerClass, MessageMapPolicy >::onCreate( itsVoidFunctionTakingSeedPointer eventHandler )
#endif

#ifdef __GNUC__
void WidgetWindow< EventHandlerClass, MessageMapPolicy >::onCreate( typename ThisMessageMap::itsVoidFunctionTakingSeedPointer eventHandler )
#endif
{
	this->addNewSignal
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_CREATE )
				, reinterpret_cast< itsVoidFunction >( eventHandler )
				, this
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::DispatcherWindowBase::dispatchCreateThis )
				)
			)
		);
}

template< class EventHandlerClass, class MessageMapPolicy >

#ifdef _MSC_VER
void WidgetWindow< EventHandlerClass, MessageMapPolicy >::onCreate( voidFunctionTakingSeedPointer eventHandler )
#endif

#ifdef __GNUC__
void WidgetWindow< EventHandlerClass, MessageMapPolicy >::onCreate( typename ThisMessageMap::voidFunctionTakingSeedPointer eventHandler )
#endif
{
	this->addNewSignal
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_CREATE )
				, reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler )
				, this
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::DispatcherWindowBase::dispatchCreate )
				)
			)
		);
}

template< class Parent, class WidgetMessageMapType >
void WidgetWindow< Parent, WidgetMessageMapType >::activatePreviousInstance()
{
	int iTries = 5;
	while ( iTries-- > 0 )
	{
		HWND hWnd = ::FindWindow( this->getNewClassName().c_str(), NULL );
		if ( hWnd )
		{
			//Make sure the window is not minimized or maximized
			::ShowWindow( hWnd, SW_SHOWNORMAL );
			::SetForegroundWindow( hWnd );
		}
		else
		{
			// It's possible that the other window is in the process of being created...
			::Sleep( 200 );
		}
	}
}

template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetChildWindow< EventHandlerClass, MessageMapPolicy >::Seed & WidgetChildWindow< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static Seed d_DefaultValues;

	// checking whether it needs initialization uses the same time as doing it
	d_DefaultValues.style |= WS_CHILD;
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetChildWindow< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	this->style = WS_VISIBLE | WS_CHILD;
}

// end namespace SmartWin
}

#endif
