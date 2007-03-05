// $Revision: 1.29 $
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
#ifndef WidgetStatusBar_h
#define WidgetStatusBar_h

#include "../WindowsHeaders.h"
#include "../Widget.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectText.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectMouseClicks.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectRightClickable.h"
#include "../aspects/AspectThreads.h"
#include "../aspects/AspectBorder.h"
#include "../MessageMapControl.h"
#include "../TrueWindow.h"
#include "../xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/// Policy class for WidgetStatusBar with sections instead of one large area where
/// you can add information
/** A Status Bar with sections is a normal statusbar except instead of one large area
  * where you can add text there are several smaller sections which divides the
  * status bar into smaller areas where you can separate information.
  */
template< class EventHandlerClass, class MessageMapPolicy >
class Section
{
public:
	/// Initializes the sections of the WidgetStatusBar
	/** Use this one to set the number of sections and the width of them
	  */
	void setSections( const std::vector< unsigned > & width );

	/// Sets the text of the given section number
	/** Use this one to set the text of a specific section of the WidgetStatusBar
	  */
	void setText( const SmartUtil::tstring & newText, unsigned partNo );
};

// Forward declaration
template< class EventHandlerClass, class MessageMapPolicy, class TypeOfStatusBar >
class WidgetStatusBar;

// Note, this Aspect class indirectly brings in AspectText while the Section one DOES NOT!
/// Policy class for a WidgetStatusBar with no sections.
/** Policy class for a WidgetStatusBar with no sections or rather with _one big_
  * section which will hold all the text of the Status Bar Control within the same
  * area! <br>
  * Use the setText member ( which is included by inheritance to AspectText ) to set
  * the text of the Status Bar Control!
  */
template< class EventHandlerClass, class MessageMapPolicy >
class NoSection :
	public AspectText< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, NoSection< EventHandlerClass, MessageMapPolicy > >, MessageMapControl< EventHandlerClass, WidgetStatusBar < EventHandlerClass, MessageMapPolicy, NoSection< EventHandlerClass, MessageMapPolicy > >, MessageMapPolicy > >
{
};

/// StatusBar class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html status.PNG
  * Class for creating a Status Bar Control. <br>
  * A status bar is a status info line normally residing in the bottom of a
  * WidgetWindow or other container type of Widget. <br>
  * You can then send text to that window to show e.g. "status info" regarding the
  * WidgetWindow which owns the Status Bar Control. A good example of an application
  * with a status bar is for instance Internet Explorer which ( unless you have made
  * it invisible ) has a strip of status information at the bottom showing for
  * instance the security settings of the current page and how far in the download
  * process you are currently etc... <br>
  * Note that there are TWO DIFFERENT status bar controls though, one which does have
  * "sections" which sub divides the status bar into several smaller sections which
  * are independant of eachother and another type which is a "flat strip" containing
  * only one large portion of text. <br>
  * The default one is the flat one, use Section as the last template parameter to
  * use the one with sections!
  */
template< class EventHandlerClass, class MessageMapPolicy, class TypeOfStatusBar = NoSection< EventHandlerClass, MessageMapPolicy > >
class WidgetStatusBar :
	public TypeOfStatusBar,
	public MessageMapControl< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapPolicy >,
	private virtual TrueWindow,

	// Aspects
	public AspectBorder< WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar > >,
	public AspectClickable< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapControl< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapPolicy > >,
	public AspectDblClickable< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapControl< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapPolicy > >,
	public AspectFont< WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar > >,
	public AspectMouseClicks< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapControl< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapPolicy > >,
	public AspectPainting< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapControl< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapControl< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapPolicy > >,
	public AspectRightClickable< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapControl< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapPolicy > >,
	// GCC chokes on private inheritance here since we're casting to WidgetType in some of the member functions in AspectSizable!!
	protected AspectSizable< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapControl< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapPolicy > >,
	public AspectThreads< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapControl< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapControl< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapPolicy > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetStatusBar, MessageMapPolicy > ThisMessageMap;
	typedef AspectSizable< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapControl< EventHandlerClass, WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >, MessageMapPolicy > > AspectSizable;
	friend class WidgetCreator< WidgetStatusBar >;
public:
	// Including the stuff we need from AspectSizable to make it accessible.
	// Note here that since we DON'T want the setBounds functions we must inherit
	// privately from AspectSizable and include the stuff we WAN'T to expose from
	// AspectSizable in a public block of the class.
	using AspectSizable::getBounds;
	using AspectSizable::getSize;
	using AspectSizable::getPosition;
	using AspectSizable::getClientAreaSize;
	using AspectSizable::getTextSize;
	using AspectSizable::bringToFront;
	using AspectSizable::onSized;
	using AspectSizable::onMoved;

	/// Class type
	typedef WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar > ThisType;

	/// Object type
	typedef WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetStatusBar::ThisType WidgetType;

		FontPtr font;

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	// Removing compiler hickup...
	virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar );

	/// Refreshes the status bar, must be called after main window has been resized
	/** Refreshes the status bar, call this one whenever you need to redraw the
	  * status bar, typical example is when you have resized the container Widget.
	  * <br>
	  * Normally you would call this function after _EVERY_ single resize the main
	  * WidgetWindow which owns the status bar gets. <br>
	  * Call this one in the onSized event handler for your WidgetWindow.
	  */
	void refresh();

	// Contract needed by AspectClickable Aspect class
	static inline Message & getClickMessage();

	// Contract needed by AspectDblClickable Aspect class
	static inline Message & getDblClickMessage();

	/// Actually creates the StatusBar
	/** You should call WidgetFactory::createStatusBar if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	// Constructor Taking pointer to parent
	explicit WidgetStatusBar( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetStatusBar()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy, class TypeOfStatusBar >
const typename WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >::Seed & WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, STATUSCLASSNAME );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP | WS_CLIPSIBLINGS | SS_NOTIFY;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy, class TypeOfStatusBar >
WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >::Seed::Seed()
{
	* this = WidgetStatusBar::getDefaultSeed();
}

template< class EventHandlerClass, class MessageMapPolicy >
void Section< EventHandlerClass, MessageMapPolicy >::setSections( const std::vector< unsigned > & width )
{
	WidgetStatusBar< EventHandlerClass, MessageMapPolicy, Section< EventHandlerClass, MessageMapPolicy > > * This
		= static_cast< WidgetStatusBar < EventHandlerClass, MessageMapPolicy, Section< EventHandlerClass, MessageMapPolicy > > * >( this );

	std::vector< unsigned > newVec( width );
	std::vector< unsigned >::const_iterator origIdx = width.begin();
	unsigned offset = 0;
	for ( std::vector< unsigned >::iterator idx = newVec.begin();
		idx < newVec.end();
		++idx, ++origIdx )
	{
		* idx = ( * origIdx ) + offset;
		offset += * origIdx;
	}
	const unsigned * intArr = & newVec[0];
	const size_t size = newVec.size();
	::SendMessage( This->handle(), SB_SETPARTS, static_cast< WPARAM >( size ), reinterpret_cast< LPARAM >( intArr ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void Section< EventHandlerClass, MessageMapPolicy >::setText( const SmartUtil::tstring & newText, unsigned partNo )
{
	WidgetStatusBar< EventHandlerClass, MessageMapPolicy, Section< EventHandlerClass, MessageMapPolicy > > * This
		= static_cast< WidgetStatusBar < EventHandlerClass, MessageMapPolicy, Section< EventHandlerClass, MessageMapPolicy > > * >( this );
	::SendMessage( This->handle(), SB_SETTEXT, static_cast< WPARAM >( partNo ), reinterpret_cast< LPARAM >( newText.c_str() ) );
}

template< class EventHandlerClass, class MessageMapPolicy, class TypeOfStatusBar >
LRESULT WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
}

template< class EventHandlerClass, class MessageMapPolicy, class TypeOfStatusBar >
void WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >::refresh()
{
	// A status bar can't really be resized since its size is controlled by the
	// parent window. But to not let the status bar "hang" we need to refresh its
	// size after the main window is being resized.
	SmartWin::Rectangle rect;
	if ( ::MoveWindow( this->handle(),
		rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, TRUE ) == 0 )
	{
		xCeption err( _T( "Couldn't reposition windows" ) );
		throw err;
	}
}

template< class EventHandlerClass, class MessageMapPolicy, class TypeOfStatusBar >
Message & WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >::getClickMessage()
{
	static Message retVal = Message( WM_NOTIFY, NM_CLICK );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy, class TypeOfStatusBar >
Message & WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >::getDblClickMessage()
{
	static Message retVal = Message( WM_NOTIFY, NM_DBLCLK );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy, class TypeOfStatusBar >
WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >::WidgetStatusBar( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a ComboBox without a parent...
	xAssert( parent, _T( "Cant have a WidgetStatusBar without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy, class TypeOfStatusBar >
void WidgetStatusBar< EventHandlerClass, MessageMapPolicy, TypeOfStatusBar >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetStatusBar::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();
	setFont( cs.font );
}

// end namespace SmartWin
}

#endif
