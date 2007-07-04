// $Revision: 1.35 $
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
#ifndef WidgetWindowBase_h
#define WidgetWindowBase_h

#include "SmartUtil.h"
#include "../MessageMapPolicyClasses.h"
#include "../WindowsHeaders.h"
#include "../CallbackFuncPrototypes.h"
#include "../xCeption.h"
#include "../Command.h"
#include "WidgetStatusBar.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectText.h"
#include "../aspects/AspectMouseClicks.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectActivate.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectEraseBackground.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectBorder.h"
#include "../aspects/AspectDragDrop.h"
#include "../aspects/AspectAdapter.h"
#include "../MessageMap.h"
#include "../BasicTypes.h"
#include <sstream>
#include <map>
#include <memory>

namespace SmartWin
{
// begin namespace SmartWin

// TODO: Fix!
// Just to get destruction logic right, crappy solution, fix! An outer most widget
// is a container widget which is one of the "application windows". When there are
// NO MORE "application widgets" the application is terminated
class OuterMostWidget
{
};

struct WidgetWindowBaseCloseDispatcher
{
	typedef std::tr1::function<bool ()> F;
	
	WidgetWindowBaseCloseDispatcher(const F& f_, Widget* widget_) : f(f_), widget(widget_) { }

	HRESULT operator()(private_::SignalContent& params) {
		bool destroy = f();

		if ( destroy ) {
			::DestroyWindow( widget->handle() );
			return TRUE;
		}

		return FALSE;
	}

	F f;
	Widget* widget;
};

struct WidgetWindowBaseTimerDispatcher
{
	typedef std::tr1::function<bool ()> F;
	
	WidgetWindowBaseTimerDispatcher(const F& f_) : f(f_) { }

	HRESULT operator()(private_::SignalContent& params) {
		bool keep = f();
		
		if(!keep) {
			::KillTimer(reinterpret_cast<HWND>(params.Msg.Handle), params.Msg.WParam);
			// TODO remove from message map as well...
		}
		return FALSE;
	}

	F f;
};

/// Main Window class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html widgetwindow.png
  * This class defines a "normal" window or the most commonly used "container 
  * Widget", normally you would define your own class which (indirectly) derives from 
  * this one. <br>
  * You would normally derive directly from WidgetFactory and then supply this class 
  * as the first template parameter. <br>
  * The second parameter would then be YOUR CLASS. <br>
  * Example <br>
  * <b>class MyMainWindow : public SmartWin::WidgetFactory<SmartWin::WidgetWindow, 
  * MyMainWindow> { ... };</b> <br>
  * Note especially that the second template argument to the WidgetFactory template 
  * class would almost ALWAYS be the name of your class derived from WidgetFactory. 
  * <br>
  * You can also derive directly from WidgetWindow and skip around the WidgetFactory 
  * factory class, the inheritance string would then become: <br>
  * <b>class MyMainWindow : public SmartWin::WidgetWindow<MyMainWindow></b> <br>
  * But then you wouldn't have access to all the "createxxx" functions from class 
  * WidgetFactory which automatically gurantees that your Widgets get the right parent 
  * etc. <br>
  * Look at (almost) any of the example projects distributed with the main download of 
  * the library residing in the SmartWinUnitTests directory for an example of how to 
  * use  this class with the factory class WidgetFactory.   
  */
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetWindowBase :
	public MessageMap< EventHandlerClass, MessageMapPolicy >,

	// Aspects
	public AspectBorder< EventHandlerClass >,
	public AspectSizable< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectFont< EventHandlerClass >,
	public AspectText< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectMouseClicks< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectKeyboard< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectFocus< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectActivate< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectEraseBackground< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectPainting< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectEnabled< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectThreads< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,
	public AspectDragDrop< EventHandlerClass, WidgetWindowBase< EventHandlerClass, MessageMapPolicy >, MessageMap< EventHandlerClass, MessageMapPolicy > >,

	public OuterMostWidget
{
public:
	/// Class type
	typedef WidgetWindowBase< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	typedef MessageMap< EventHandlerClass, MessageMapPolicy > MessageMapType;
private:
	typedef WidgetWindowBaseCloseDispatcher CloseDispatcher;
	typedef AspectAdapter<CloseDispatcher::F, EventHandlerClass, MessageMapType::IsControl> CloseAdapter;
	typedef WidgetWindowBaseTimerDispatcher TimerDispatcher;
public:

	// The next line must be included in Widgets that are supposed to support being
	// Maximized, Minimized or Restored. It's a Magic Enum construct and can be
	// read about at
	// http://blog.notus.no/thomas/PermaLink.aspx?guid=c311fe7e-40d5-48a2-baa3-f69957d5f313
	// The enum is dereferenced in AspectSizable::restore/minimize/maximize
	enum MaxiMiniRestorable
	{};

	// TODO: Outfactor into WidgetClosable
	/// Event Handler setter for the Closing Event
	/** If supplied event handler is called before the window is closed. <br>
	  * Signature of event handler must be "bool foo()" <br>
	  * If you return true from your event handler the window is closed, otherwise 
	  * the window is NOT allowed to actually close!!       
	  */
	void onClosing( typename MessageMapType::itsBoolFunctionTakingVoid eventHandler ) {
		onClosing(CloseAdapter::adapt0(boost::polymorphic_cast<ThisType*>(this), eventHandler));		
	}
	void onClosing( typename MessageMapType::boolFunctionTakingVoid eventHandler ) {
		onClosing(CloseAdapter::adapt0(boost::polymorphic_cast<ThisType*>(this), eventHandler));		
	}
	void onClosing(const CloseDispatcher::F& f) {
		MessageMapBase * ptrThis = boost::polymorphic_cast< MessageMapBase * >( this );
		ptrThis->setCallback(
			Message( WM_CLOSE ), CloseDispatcher(f, boost::polymorphic_cast<Widget*>(this))
		);
	}
            
	// TODO: Outfactor into "time Aspect" class
	/// Creates a timer object.
	/** The supplied function must have the signature bool foo() <br>
	  * The event function will be called when at least milliSeconds seconds have elapsed.
	  * If your event handler returns true, it will keep getting called periodically, otherwise 
	  * it will be removed.
	  */
	void createTimer(const TimerDispatcher::F& f, unsigned int milliSeconds);

	/// Closes the window
	/** Call this function to raise the "Closing" event. <br>
	  * This will normally try to close the window. <br>
	  * Note! <br>
	  * If this event is trapped and we in that event handler state that we DON'T 
	  * want to close the window (by returning false) the window will not be close. 
	  * <br>
	  * Note! <br>
	  * If the asyncron argument is true the message will be posted to the message 
	  * que meaning that the close event will be done asyncronously and therefore the 
	  * function will return immediately and the close event will be handled when the 
	  * close event pops up in the event handler que.       
	  */
	void close( bool asyncron = false );

	// TODO: Outfactor to system implementation type, see e.g. WidgetFactory
#ifndef WINCE
	/// Animates a window
	/** Slides the window into view from either right or left depending on the
	  * parameter "left". If "left" is true, then from the left,  otherwise from the
	  * right. <br>
	  * Show defines if the window shall come INTO view or go OUT of view. <br>
	  * The "time" parameter is the total duration of the function in milliseconds. 
	  */
	void animateSlide( bool show, bool left, unsigned int msTime );

	/// Animates a window
	/** Blends the window INTO view or OUT of view. <br>
	  * Show defines if the window shall come INTO view or go OUT of view. <br>
	  * The "time" parameter is the total duration of the function in milliseconds. 
	  */
	void animateBlend( bool show, int msTime );

	/// Animates a window
	/** Collapses the window INTO view or OUT of view. The collapse can be thought of
	  * as either an "explosion" or an "implosion". <br>
	  * Show defines if the window shall come INTO view or go OUT of view. <br>
	  * The "time" parameter is the total duration of the function in milliseconds. 
	  */
	void animateCollapse( bool show, int msTime );
#endif

	/// Adds or removes the minimize box from the Widget
	void setMinimizeBox( bool value = true );

	/// Adds or removes the maximize box from the Widget
	void setMaximizeBox( bool value = true );

	/// Sets the small icon for the Widget (the small icon appears typically in the top left corner of the Widget)
	void setIconSmall( int resourceId );

	/// Sets the small icon for the Widget (the small icon appears typically in the top left corner of the Widget)
	void setIconSmall( const SmartUtil::tstring & filePathName );

	/// Sets the large icon for the Widget (the large icon appears e.g. when you press ALT+Tab)
	void setIconLarge( int resourceId );

	/// Sets the large icon for the Widget (the large icon appears e.g. when you press ALT+Tab)
	void setIconLarge( const SmartUtil::tstring & filePathName );

	/// Sets the cursor for the Widget
	void setCursor( int resourceId );

	/// Sets the cursor for the Widget
	void setCursor( const SmartUtil::tstring & filePathName );

protected:
	// Protected since this Widget we HAVE to inherit from
	explicit WidgetWindowBase( Widget * parent = 0 );

	// Protected to avoid direct instantiation, you can inherit but NOT instantiate
	// directly
	virtual ~WidgetWindowBase()
	{}


};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::createTimer( const TimerDispatcher::F& f,
	unsigned int milliSecond)
{

	UINT_PTR id = ::SetTimer( this->Widget::itsHandle, 0, static_cast< UINT >( milliSecond ), NULL);
	
	MessageMapBase * ptrThis = boost::polymorphic_cast< MessageMapBase * >( this );
	ptrThis->setCallback(
		Message( WM_TIMER, id ), TimerDispatcher(f)
	);

}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::close( bool asyncron )
{
	if ( asyncron )
		::PostMessage( this->Widget::itsHandle, WM_CLOSE, 0, 0 ); // Return now
	else
		::SendMessage( this->Widget::itsHandle, WM_CLOSE, 0, 0 ); // Return after close is done.
}

#ifndef WINCE
template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::animateSlide( bool show, bool left, unsigned int time )
{
	::AnimateWindow( this->Widget::itsHandle, static_cast< DWORD >( time ),
		show ?
			left ? AW_SLIDE | AW_HOR_NEGATIVE :
				AW_SLIDE | AW_HOR_POSITIVE
			:
			left ? AW_HIDE | AW_SLIDE | AW_HOR_NEGATIVE :
				AW_HIDE | AW_SLIDE | AW_HOR_POSITIVE
			);
}

//HC: This function gives problems with some non-Microsoft visual styles
template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::animateBlend( bool show, int msTime )
{
	::AnimateWindow( this->Widget::itsHandle, static_cast< DWORD >( msTime ), show ? AW_BLEND : AW_HIDE | AW_BLEND );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::animateCollapse( bool show, int msTime )
{
	::AnimateWindow( this->Widget::itsHandle, static_cast< DWORD >( msTime ), show ? AW_CENTER : AW_HIDE | AW_CENTER );
}
#endif

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::setMinimizeBox( bool value )
{
	Widget::addRemoveStyle( WS_MINIMIZEBOX, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::setMaximizeBox( bool value )
{
	Widget::addRemoveStyle( WS_MAXIMIZEBOX, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::setIconSmall( int resourceId )
{
	HICON hIcon = ( HICON )::LoadImage( Application::instance().getAppHandle(), MAKEINTRESOURCE( resourceId ), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR );
	::SendMessage( this->Widget::itsHandle, WM_SETICON, ICON_SMALL, reinterpret_cast< LPARAM >( hIcon ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::setIconLarge( int resourceId )
{
	HICON hIcon = ( HICON )::LoadImage( Application::instance().getAppHandle(), MAKEINTRESOURCE( resourceId ), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE );
	::SendMessage( this->Widget::itsHandle, WM_SETICON, ICON_BIG, reinterpret_cast< LPARAM >( hIcon ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::setIconSmall( const SmartUtil::tstring & filePathName )
{
	HICON hIcon = ( HICON )::LoadImage( 0, filePathName.c_str(), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR | LR_LOADFROMFILE );
	::SendMessage( this->Widget::itsHandle, WM_SETICON, ICON_SMALL, reinterpret_cast< LPARAM >( hIcon ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::setIconLarge( const SmartUtil::tstring & filePathName )
{
	HICON hIcon = ( HICON )::LoadImage( 0, filePathName.c_str(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	::SendMessage( this->Widget::itsHandle, WM_SETICON, ICON_BIG, reinterpret_cast< LPARAM >( hIcon ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::setCursor( int resourceId )
{
	HCURSOR hCur = ::LoadCursor( Application::instance().getAppHandle(), MAKEINTRESOURCE( resourceId ) );
	::SetClassLongPtr( this->Widget::itsHandle, GCLP_HCURSOR, reinterpret_cast< LONG >( hCur ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::setCursor( const SmartUtil::tstring & filePathName )
{
	HICON hCur = ( HICON )::LoadImage( 0, filePathName.c_str(), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	::SetClassLongPtr( this->Widget::itsHandle, GCLP_HCURSOR, reinterpret_cast< LONG >( hCur ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetWindowBase< EventHandlerClass, MessageMapPolicy >::WidgetWindowBase( Widget * parent )
	: Widget( parent, 0, true )
{
	this->Widget::itsCtrlId = 0;
}

// end namespace SmartWin
}

#endif
