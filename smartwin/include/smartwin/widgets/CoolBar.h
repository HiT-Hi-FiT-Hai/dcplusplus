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
#ifndef CoolBar_h
#define CoolBar_h

#ifndef WINCE // Doesn't exist in Windows CE based systems

#include "../Widget.h"
#include "../Point.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectVisible.h"
#include "../xCeption.h"
#include "Button.h"
#include "CheckBox.h"
#include "ComboBox.h"
#include "DateTime.h"
#include "GroupBox.h"
#include "WidgetMenu.h"
#include "Spinner.h"
#include "TextBox.h"
#include "Tree.h"
#include "RadioButton.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

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
  */
class CoolBar :
	public MessageMapPolicy< Policies::Subclassed >,

	// Aspects
	public AspectEnabled< CoolBar >,
	public AspectFocus< CoolBar >,
	public AspectFont< CoolBar >,
	public AspectRaw< CoolBar >,
	private AspectSizable< CoolBar >,
	public AspectVisible< CoolBar >
{
	typedef SmartWin::AspectSizable< CoolBar > AspectSizable;
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
	typedef ThisType * ObjectType;

	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public Widget::Seed
	{
	public:
		Seed();
	};

	/// ComboBox object type.
	typedef ComboBox::ObjectType ComboBoxPtr;

	/// TextBox object type.
	typedef TextBox::ObjectType TextBoxPtr;

	/// Button object type.
	typedef Button::ObjectType ButtonPtr;

	/// Button object type.
	typedef Tree::ObjectType TreePtr;

	/// CheckBox object type.
	typedef CheckBox::ObjectType CheckBoxPtr;

	/// Spinner object type.
	typedef Spinner::ObjectType SpinnerPtr;

	/// GroupBox object type.
	typedef GroupBox::ObjectType GroupBoxPtr;

	/// RadioButton object type.
	typedef RadioButton::ObjectType RadioButtonPtr;

	/// DateTimePicker object type.
	typedef DateTime::ObjectType DateTimePtr;

	/// Menu object type.
	typedef WidgetMenu::ObjectType WidgetMenuPtr;

	/// Actually creates the Coolbar
	/** You should call WidgetFactory::createCoolbar if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	void create( const Seed & cs = Seed() );

	RadioButtonPtr sow( GroupBoxPtr & parent, const RadioButton::Seed & cs )
	{
#ifdef PORT_ME
		RadioButtonPtr retVal ( WidgetCreator< RadioButton >::create( parent, internal_::getTypedParentOrThrow < EventHandlerClass * >( this ), cs ) );
		retVal->setBounds( 0, 0, cs.location.width(), cs.location.size.y );
		parent->addChild( retVal );
		return retVal;
#endif
		return NULL;
	}

	/// Creates a Widget using its CreationalInfo
	/** Adds up the created widget into a new band in the Coolbar control
	  */
	template< class A_Seed >
	typename A_Seed::WidgetType::ObjectType sow( const /*typename*/ A_Seed & cs, const SmartUtil::tstring & label = _T("") )
	{
		typename A_Seed::WidgetType::ObjectType retVal ( WidgetCreator< typename A_Seed::WidgetType >::create( this, cs ) );
		retVal->setBounds( 0, 0, cs.location.width(), cs.location.size.y );
		this->addChild( retVal, cs.location.width(), cs.location.size.y, label );
		return retVal;
	}

	//TODO: update this help
	/// Creates a ComboBox Widget inside the Coolbar
	/** The returned ComboBox is roughly the same object type as a "normal"
	  * ComboBox, though size and position doesn't count since the Coolbar will
	  * control these properties itself. Apart from that the combobox has the same
	  * properties as a normal ComboBox. The width and the openedHeight
	  * properties is the width of the combobox and the height of the dropped down
	  * viewable area of the ComboBox. The bandHeight is the height of the actual
	  * band Note that the rect part of the CreationalStruct passed is more or less
	  * ignored...
	  */

	//template< >
	//typename CoolBar< EventHandlerClass >::ComboBoxPtr // Bug in VC++7.1 Koenig Lookup forces us to give full type of return value...
	//sow< typename CoolBar< EventHandlerClass >::ComboBox::Seed >
	// ( const typename CoolBar< EventHandlerClass >::Seed & cs, const SmartUtil::tstring & label = _T("") )
	//{
	// typename CoolBar< EventHandlerClass >::ObjectType retVal (WidgetCreator< typename CoolBar< EventHandlerClass > >::create( this, cs ));
	// retVal->setBounds( 0, 0, cs.rect.width(), cs.rect.size.y );
	// //TODO: use something like cs.itsOpenedHeight
	// this->addChild( retVal, cs.rect.width(), cs.rect.size.y, label );
	// return retVal;
	//}

	//TODO: Menu specialization

	/// Refreshes the Coolbar
	/** Call this one after the container widget has been resized to make sure the
	  * coolbar is having the right size...
	  */
	void refresh();

protected:
	// CTOR
	explicit CoolBar( SmartWin::Widget * parent );

	// To assure nobody accidentally deletes any heaped object of this type, parent
	// is supposed to do so when parent is killed...
	virtual ~CoolBar()
	{}

private:
	// Adds up the given child to a new rebar band...
	void addChild( Widget * child, unsigned width, unsigned height, const SmartUtil::tstring & txt );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline CoolBar::CoolBar( SmartWin::Widget * parent )
	: PolicyType( parent )
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
	SmartWin::Rectangle rect;
	if ( ::MoveWindow( this->handle(),
		rect.x(), rect.y(), rect.width(), rect.height(), TRUE ) == 0 )
	{
		xCeption err( _T( "Couldn't reposition windows" ) );
		throw err;
	}
}

// end namespace SmartWin
}

#endif  //WINCE
#endif  //CoolBar_h
