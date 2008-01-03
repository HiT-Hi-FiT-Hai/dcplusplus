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

#include "../BasicTypes.h"
#include "../WindowClass.h"
#include "WidgetWindowBase.h"

#include <boost/scoped_ptr.hpp>

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
class WidgetWindow
	: public WidgetWindowBase< Policies::Normal >
{
	typedef WidgetWindowBase< Policies::Normal > BaseType;
	struct CreateDispatcher
	{
		typedef std::tr1::function<void (const CREATESTRUCT&)> F;

		CreateDispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {

			CREATESTRUCT * cs = reinterpret_cast< CREATESTRUCT * >( msg.lParam );

			f(*cs);
			
			return false;
		}

		F f;
	};

public:
	/// Class type
	typedef WidgetWindow ThisType;

	/// Object type
	typedef ThisType* ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public Widget::Seed
	{
	public:
		typedef WidgetWindow::ThisType WidgetType;

		IconPtr icon;
		IconPtr smallIcon;
		HBRUSH background;
		LPCTSTR menuName;
		HCURSOR cursor;

		/// Fills with default parameters
		Seed();
	};

	/// Actually creates the window
	/** This one creates the window. <br>
	  * All WidgetWindows, and classes derived from them must create the Window
	  * before using it with functions such as setBounds() or setVisible( false ). <br>
      * The simple version "createWindow()" uses a default Seed for the window attributes.
	  * The seed is not taken a constant because the class name will be generated at registration.
	  */
	void createWindow( Seed cs = Seed() );

	/// Creates an invisible window, for quiet initialization.
	/** Same as createWindow, except that the window lacks WS_VISIBLE.
	  * Since you must create the window before you add other Widgets,
	  * and doing so causes a bit of screen flash before the final window
	  * is ready, createInvisibleWindow() lets you add Widgets while
	  * the main Widget is not visible.  Of course you could do code like <br>
	  *
	  *   Seed defInvisible = Seed(); <br>
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
	void createInvisibleWindow( Seed cs = Seed() );

	// TODO: Check up if the CREATESTRUCT * actualy IS modyfiable...!!
	/// Setting the event handler for the "create" event
	/** The event handler must have the signature "void foo( CREATESTRUCT * )" where
	  * the WidgetType is the type of Widget that realizes the Aspect. <br>
	  * If you supply an event handler for this event your handler will be called 
	  * when Widget is initially created. <br>
	  * The argument CREATESTRUCT sent into your handler is modifiable so that you 
	  * can add/remove styles and add remove EX styles etc.       
	  */
	void onCreate(const CreateDispatcher::F& f) {
		setCallback(
			Message( WM_CREATE ), CreateDispatcher(f)
		);
	}


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
	boost::scoped_ptr<WindowClass> windowClass;
};

class WidgetChildWindow
	: public WidgetWindow
{
public:
	typedef WidgetChildWindow ThisType;
	typedef ThisType* ObjectType;
	
	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public WidgetWindow::Seed
	{
	public:
		/// Fills with default parameters
		Seed();
	};

	//TODO: This could be specialized to take WNDPROC from MessageMapPolicy
	/// Actually creates the window
	/** This one creates the window. It is implemented in case somebody wants to use
	  * createWindow() without parameters. If it wasn't declared, the compiler would
	  * call WidgetWindow::create with WidgetWindow::Seed, which wouldn't
	  * create a child window.
	  */
	void createWindow( const Seed& cs = Seed() )
	{
		WidgetWindow::createWindow( cs );
	}

protected:
	// Unlike WidgetWindow, WidgetChildWindow must have a parent!!!
	explicit WidgetChildWindow( Widget * parent ) : WidgetWindow( parent ) 
	{};
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline WidgetWindow::WidgetWindow( Widget * parent )
	: BaseType( parent )
{}

inline WidgetWindow::~WidgetWindow()
{
}

inline void WidgetWindow::createInvisibleWindow( Seed cs )
{
	cs.style=  cs.style & ( ~ WS_VISIBLE );
	WidgetWindow::createWindow( cs );
}

inline void WidgetWindow::activatePreviousInstance()
{
#ifdef PORT_ME
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
#endif
}

inline WidgetChildWindow::Seed::Seed()
{
	this->style = WS_VISIBLE | WS_CHILD;
}

// end namespace SmartWin
}

#endif
