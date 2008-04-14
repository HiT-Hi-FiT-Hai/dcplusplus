/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  SmartWin++

  Copyright (c) 2005 Thomas Hansen

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor SmartWin++ nor the names of its contributors 
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

#ifndef DWT_StatusBar_h
#define DWT_StatusBar_h

#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectText.h"
#include "Control.h"

namespace dwt {

/// Policy class for StatusBar with sections instead of one large area where
/// you can add information
/** A Status Bar with sections is a normal statusbar except instead of one large area
  * where you can add text there are several smaller sections which divides the
  * status bar into smaller areas where you can separate information.
  */
class Section
{
public:
	/// Initializes the sections of the StatusBar
	/** Use this one to set the number of sections and the width of them
	  */
	void setSections( const std::vector< unsigned > & width );

	/// Sets the text of the given section number
	/** Use this one to set the text of a specific section of the StatusBar
	  */
	void setText( const SmartUtil::tstring & newText, unsigned partNo );
};

// Forward declaration
template< class TypeOfStatusBar >
class StatusBar;

// Note, this Aspect class indirectly brings in AspectText while the Section one DOES NOT!
/// Policy class for a StatusBar with no sections.
/** Policy class for a StatusBar with no sections or rather with _one big_
  * section which will hold all the text of the Status Bar Control within the same
  * area! <br>
  * Use the setText member ( which is included by inheritance to AspectText ) to set
  * the text of the Status Bar Control!
  */
class NoSection :
	public AspectText< StatusBar<NoSection> >
{
};

/// StatusBar class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html status.PNG
  * Class for creating a Status Bar Control. <br>
  * A status bar is a status info line normally residing in the bottom of a
  * Window or other container type of Widget. <br>
  * You can then send text to that window to show e.g. "status info" regarding the
  * Window which owns the Status Bar Control. A good example of an application
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
template< class TypeOfStatusBar = NoSection >
class StatusBar :
	public TypeOfStatusBar,
	public CommonControl,
	
	// Aspects
	public AspectClickable< StatusBar< TypeOfStatusBar > >,
	public AspectDblClickable< StatusBar< TypeOfStatusBar > >,
	public AspectFont< StatusBar< TypeOfStatusBar > >,
	public AspectPainting< StatusBar< TypeOfStatusBar > >
{
	typedef CommonControl BaseType;
	friend class WidgetCreator< StatusBar >;
	friend class AspectClickable< StatusBar< TypeOfStatusBar > >;
	friend class AspectDblClickable< StatusBar< TypeOfStatusBar > >;
public:
	/// Class type
	typedef StatusBar<TypeOfStatusBar> ThisType;

	/// Object type
	typedef ThisType* ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;

		FontPtr font;

		/// Fills with default parameters
		explicit Seed(bool sizeGrip = true);
	};

	/// Refreshes the status bar, must be called after main window has been resized
	/** Refreshes the status bar, call this one whenever you need to redraw the
	  * status bar, typical example is when you have resized the container Widget.
	  * <br>
	  * Normally you would call this function after _EVERY_ single resize the main
	  * Window which owns the status bar gets. <br>
	  * Call this one in the onSized event handler for your Window.
	  */
	void refresh();

	/// Actually creates the StatusBar
	/** You should call WidgetFactory::createStatusBar if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	void create( const Seed & cs = Seed() );

protected:
	// Constructor Taking pointer to parent
	explicit StatusBar( dwt::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~StatusBar()
	{}
	
	// Contract needed by AspectClickable Aspect class
	static Message getClickMessage();

	// Contract needed by AspectDblClickable Aspect class
	static Message getDblClickMessage();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class TypeOfStatusBar >
StatusBar< TypeOfStatusBar >::Seed::Seed(bool sizeGrip) : BaseType::Seed(STATUSCLASSNAME, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS) {
	if(sizeGrip) {
		style |= SBARS_SIZEGRIP;
	}
}

inline void Section::setSections( const std::vector< unsigned > & width )
{
	StatusBar< Section > * This
		= static_cast< StatusBar < Section > * >( this );

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

inline void Section::setText( const SmartUtil::tstring & newText, unsigned partNo )
{
	StatusBar< Section > * This
		= static_cast< StatusBar < Section > * >( this );
	::SendMessage( This->handle(), SB_SETTEXT, static_cast< WPARAM >( partNo ), reinterpret_cast< LPARAM >( newText.c_str() ) );
}

template< class TypeOfStatusBar >
void StatusBar< TypeOfStatusBar >::refresh()
{
	// A status bar can't really be resized since its size is controlled by the
	// parent window. But to not let the status bar "hang" we need to refresh its
	// size after the main window is being resized.
	dwt::Rectangle rect;
	if ( ::MoveWindow( this->handle(),
		rect.x(), rect.y(), rect.width(), rect.height(), TRUE ) == 0 )
	{
		xCeption err( _T( "Couldn't reposition windows" ) );
		throw err;
	}
}

template< class TypeOfStatusBar >
Message StatusBar< TypeOfStatusBar >::getClickMessage()
{
	return Message( WM_NOTIFY, NM_CLICK );
}

template< class TypeOfStatusBar >
Message StatusBar< TypeOfStatusBar >::getDblClickMessage()
{
	return Message( WM_NOTIFY, NM_DBLCLK );
}

template< class TypeOfStatusBar >
StatusBar< TypeOfStatusBar >::StatusBar( dwt::Widget * parent )
	: BaseType( parent )
{
}

template< class TypeOfStatusBar >
void StatusBar< TypeOfStatusBar >::create( const Seed & cs )
{
	BaseType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

// end namespace dwt
}

#endif
