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

#ifndef DWT_CoolBar_h
#define DWT_CoolBar_h

#ifndef WINCE // Doesn't exist in Windows CE based systems

#include "../Policies.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectVisible.h"
#include "../xCeption.h"

namespace dwt {

/// Coolbar Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html coolbar.PNG
  * A coolbar is a strip of buttons normally associated with menu commands, like  for 
  * instance Internet Explorer has ( unless you have made them invisible ) a toolbar 
  * of buttons, one for going "home", one to stop rendering of the  current page, one 
  * to view the log of URL's you have been to etc... In addition to serving like a 
  * dockable toolbar ( see ToolBar ) a Coolbar  Widget can also contain more 
  * complex Widgets lke for instance a ComboBox, a TextBox and so on...   
  * 
  * @todo This class needs some love...       
  */
class CoolBar :
	public MessageMap< Policies::Subclassed >,

	// Aspects
	public AspectEnabled< CoolBar >,
	public AspectFocus< CoolBar >,
	public AspectFont< CoolBar >,
	public AspectRaw< CoolBar >,
	private AspectSizable< CoolBar >,
	public AspectVisible< CoolBar >
{
	typedef MessageMap<Policies::Subclassed> BaseType;
	typedef dwt::AspectSizable< CoolBar > AspectSizable;
	friend class WidgetCreator< CoolBar >;
public:
	// Including the stuff we need from AspectSizable to make it accessible
	// Note here that since we DON'T want the setBounds functions we must
	// inherit privately from AspectSizable and include the stuff we WAN'T to
	// expose from AspectSizable in a public block of the class.
	using AspectSizable::getBounds;
	using AspectSizable::getSize;
	using AspectSizable::getPosition;
	using AspectSizable::getClientAreaSize;
	using AspectSizable::getTextSize;
	using AspectSizable::bringToFront;
	using AspectSizable::onSized;
	using AspectSizable::onMoved;

	/// Class type
	typedef CoolBar ThisType;

	/// Object type
	typedef ThisType* ObjectType;
	
	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;
		
		Seed();
	};

	/// Actually creates the Coolbar
	/** You should call WidgetFactory::createCoolbar if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	void create( const Seed & cs = Seed() );

	/// Refreshes the Coolbar
	/** Call this one after the container widget has been resized to make sure the
	  * coolbar is having the right size...
	  */
	void refresh();

protected:
	// CTOR
	explicit CoolBar( dwt::Widget * parent );

	// To assure nobody accidentally deletes any heaped object of this type, parent
	// is supposed to do so when parent is killed...
	virtual ~CoolBar()
	{}

private:
	// Adds up the given child to a new rebar band...
	void addChild( Widget * child, unsigned width, unsigned height, const tstring & txt );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline CoolBar::CoolBar( Widget * parent )
	: BaseType( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

inline void CoolBar::refresh()
{
	// This might look a bit stupid, but Windows API have some minor flaws. One of
	// those flaws is that a Coolbar (and a Toolbar) control must be "resized" with
	// a dummy value to make sure the Coolbar (&& the Toolbar) fills up the
	// complete area of the container Widget...

	// HC comment: sorry ;( but no smiley faces ;) anywhere, not even in comments.
	// They mess up with my macros to check delimiters ...
	Rectangle rect;
	if ( ::MoveWindow( this->handle(),
		rect.x(), rect.y(), rect.width(), rect.height(), TRUE ) == 0 )
	{
		xCeption err( _T( "Couldn't reposition windows" ) );
		throw err;
	}
}

}

#endif  //WINCE
#endif  //CoolBar_h
