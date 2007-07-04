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
#ifndef WidgetMDIFrame_h
#define WidgetMDIFrame_h

#include "WidgetWindowBase.h"
#include "../aspects/AspectThreads.h"
#include "../SignalParams.h"
#include "../BasicTypes.h"

namespace SmartWin
{
// begin namespace SmartWin

/// "MDI Frame" class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html window.PNG
  * Class for creating a "normal" window. <br>
  * A normal window is what most applications would use as the basic main structure, 
  * in it you can add other Widgets like for instance buttons, comboboxes etc. <br>
  * A WidgetMDIFrame is basically a container Widget for other Widgets to reside in. 
  * <br>
  * Class is a public superclass of WidgetMDIFrameBase and therefore can use all 
  * features of WidgetMDIFrameBase.   
  */
template< class EventHandlerClass >
class WidgetMDIFrame
	: public WidgetWindowBase< EventHandlerClass, MessageMapPolicyMDIFrameWidget<WidgetMDIFrame<EventHandlerClass> > >
{
	typedef WidgetWindowBase< EventHandlerClass, MessageMapPolicyMDIFrameWidget<WidgetMDIFrame<EventHandlerClass> > > BaseType;
	typedef typename BaseType::MessageMapType MessageMapType;

public:
	typedef SmartWin::WidgetMDIParent<EventHandlerClass> WidgetMDIParent;
	typedef typename WidgetMDIParent::ObjectType WidgetMDIParentPtr;
	
	/// Class type
	typedef WidgetMDIFrame< EventHandlerClass > ThisType;

	/// Object type
	typedef ThisType* ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetMDIFrame::ThisType WidgetType;

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
	  * All WidgetMDIFrames, and classes derived from them must create the Window
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

	WidgetMDIParentPtr getMDIClient() { return mdi; }
protected:
	// Protected since this Widget we HAVE to inherit from
	explicit WidgetMDIFrame( Widget * parent = 0 );

	virtual ~WidgetMDIFrame();
private:
	SmartUtil::tstring itsRegisteredClassName;
	WidgetMDIParentPtr mdi;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass >
const typename WidgetMDIFrame< EventHandlerClass >::Seed & WidgetMDIFrame< EventHandlerClass >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
		d_DefaultValues.background = ( HBRUSH )( COLOR_APPWORKSPACE + 1 );
		d_DefaultValues.caption = _T( "" );
#ifndef WINCE
		d_DefaultValues.cursor = NULL;
		d_DefaultValues.icon = NULL;
#else
		d_DefaultValues.cursor = 0;
		d_DefaultValues.icon = 0;
#endif
		d_DefaultValues.menuName = _T( "" ); //TODO: does menu &"" work as good as menu NULL ?

		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass >
WidgetMDIFrame< EventHandlerClass >::Seed::Seed()
{
	* this = WidgetMDIFrame::getDefaultSeed();
}

template< class EventHandlerClass >
WidgetMDIFrame< EventHandlerClass >::WidgetMDIFrame( Widget * parent )
	: Widget(parent), BaseType( parent ), mdi(0)
{}

template< class EventHandlerClass >
WidgetMDIFrame< EventHandlerClass >::~WidgetMDIFrame()
{
	::UnregisterClass( itsRegisteredClassName.c_str(), Application::instance().getAppHandle() );
}


template< class EventHandlerClass >
void WidgetMDIFrame< EventHandlerClass >::createInvisibleWindow( Seed cs )
{
	cs.style=  cs.style & ( ~ WS_VISIBLE );
	WidgetMDIFrame< EventHandlerClass >::createWindow( cs );
}


template< class EventHandlerClass >
void WidgetMDIFrame< EventHandlerClass >::createWindow( Seed cs )
{
	Application::instance().generateLocalClassName( cs );
	itsRegisteredClassName = cs.getClassName();

	SMARTWIN_WNDCLASSEX ws;

#ifndef WINCE
	ws.cbSize = sizeof( SMARTWIN_WNDCLASSEX );
#endif //! WINCE
	// This are window class styles, not window styles ...
	ws.style = CS_DBLCLKS;	// Allow double click messages
	ws.lpfnWndProc = &MessageMapType::wndProc;
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
		xCeption x( _T( "WidgetMDIFrameBase.createWindow() SmartWinRegisterClass fizzled..." ) );
		throw x;
	}
	Application::instance().addLocalWindowClassToUnregister( cs );
	Widget::create( cs );
	
	mdi = WidgetCreator<WidgetMDIParent>::create(this);
}

// end namespace SmartWin
}

#endif
