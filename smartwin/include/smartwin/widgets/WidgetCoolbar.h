// $Revision: 1.18 $
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
#ifndef WidgetCoolbar_h
#define WidgetCoolbar_h

#ifndef WINCE // Doesn't exist in Windows CE based systems

#include "../BasicTypes.h"
#include "../MessageMapPolicyClasses.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectVisible.h"
#include "../xCeption.h"
#include "WidgetButton.h"
#include "WidgetCheckBox.h"
#include "WidgetComboBox.h"
#include "WidgetDateTimePicker.h"
#include "WidgetGroupBox.h"
#include "WidgetMenu.h"
#include "WidgetSpinner.h"
#include "WidgetTextBox.h"
#include "WidgetTreeView.h"

//TODO: complete migration to CreationalInfo.
//The position part of WidgetXxx::Seed::rect is disregarded

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
  * dockable toolbar ( see WidgetToolbar ) a Coolbar  Widget can also contain more 
  * complex Widgets lke for instance a WidgetComboBox, a WidgetTextBox and so on...          
  */
class WidgetCoolbar :
	public MessageMapPolicy< Policies::Subclassed >,

	// Aspects
	public AspectEnabled< WidgetCoolbar >,
	public AspectFocus< WidgetCoolbar >,
	public AspectFont< WidgetCoolbar >,
	public AspectRaw< WidgetCoolbar >,
	private AspectSizable< WidgetCoolbar >,
	public AspectVisible< WidgetCoolbar >
{
	typedef SmartWin::AspectSizable< WidgetCoolbar > AspectSizable;
	friend class WidgetCreator< WidgetCoolbar >;
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
	typedef WidgetCoolbar ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef WidgetCoolbar::ThisType TheWidgetType;

		//TODO: put variables to be filled here

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	/// ComboBox object type.
	typedef WidgetComboBox::ObjectType WidgetComboBoxPtr;

	/// TextBox object type.
	typedef WidgetTextBox<>::ObjectType WidgetTextBoxPtr;

	/// Button object type.
	typedef WidgetButton::ObjectType WidgetButtonPtr;

	/// Button object type.
	typedef WidgetTreeView::ObjectType WidgetTreeViewPtr;

	/// CheckBox object type.
	typedef WidgetCheckBox::ObjectType WidgetCheckBoxPtr;

	/// Spinner object type.
	typedef WidgetSpinner::ObjectType WidgetSpinnerPtr;

	/// GroupBox object type.
	typedef WidgetGroupBox::ObjectType WidgetGroupBoxPtr;

	/// RadioButton object type.
	typedef WidgetRadioButton::ObjectType WidgetRadioButtonPtr;

	/// DateTimePicker object type.
	typedef WidgetDateTimePicker::ObjectType WidgetDateTimePickerPtr;

	/// Menu object type.
	typedef WidgetMenu::ObjectType WidgetMenuPtr;

	/// Actually creates the Coolbar
	/** You should call WidgetFactory::createCoolbar if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

	WidgetRadioButtonPtr sow( WidgetGroupBoxPtr & parent, const WidgetRadioButton::Seed & cs )
	{
#ifdef PORT_ME
		WidgetRadioButtonPtr retVal ( WidgetCreator< WidgetRadioButton >::create( parent, internal_::getTypedParentOrThrow < EventHandlerClass * >( this ), cs ) );
		retVal->setBounds( 0, 0, cs.location.size.x, cs.location.size.y );
		parent->addChild( retVal );
		return retVal;
#endif
	}

	/// Creates a Widget using its CreationalInfo
	/** Adds up the created widget into a new band in the Coolbar control
	  */
	template< class A_Seed >
	typename A_Seed::WidgetType::ObjectType sow( const /*typename*/ A_Seed & cs, const SmartUtil::tstring & label = _T("") )
	{
		typename A_Seed::WidgetType::ObjectType retVal ( WidgetCreator< typename A_Seed::WidgetType >::create( this, cs ) );
		retVal->setBounds( 0, 0, cs.location.size.x, cs.location.size.y );
		this->addChild( retVal, cs.location.size.x, cs.location.size.y, label );
		return retVal;
	}

	//TODO: update this help
	/// Creates a ComboBox Widget inside the Coolbar
	/** The returned ComboBox is roughly the same object type as a "normal"
	  * ComboBox, though size and position doesn't count since the Coolbar will
	  * control these properties itself. Apart from that the combobox has the same
	  * properties as a normal WidgetComboBox. The width and the openedHeight
	  * properties is the width of the combobox and the height of the dropped down
	  * viewable area of the ComboBox. The bandHeight is the height of the actual
	  * band Note that the rect part of the CreationalStruct passed is more or less
	  * ignored...
	  */

	//template< >
	//typename WidgetCoolbar< EventHandlerClass >::WidgetComboBoxPtr // Bug in VC++7.1 Koenig Lookup forces us to give full type of return value...
	//sow< typename WidgetCoolbar< EventHandlerClass >::WidgetComboBox::Seed >
	// ( const typename WidgetCoolbar< EventHandlerClass >::Seed & cs, const SmartUtil::tstring & label = _T("") )
	//{
	// typename WidgetCoolbar< EventHandlerClass >::ObjectType retVal (WidgetCreator< typename WidgetCoolbar< EventHandlerClass > >::create( this, cs ));
	// retVal->setBounds( 0, 0, cs.rect.size.x, cs.rect.size.y );
	// //TODO: use something like cs.itsOpenedHeight
	// this->addChild( retVal, cs.rect.size.x, cs.rect.size.y, label );
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
	explicit WidgetCoolbar( SmartWin::Widget * parent );

	// To assure nobody accidentally deletes any heaped object of this type, parent
	// is supposed to do so when parent is killed...
	virtual ~WidgetCoolbar()
	{}

private:
	// Adds up the given child to a new rebar band...
	void addChild( Widget * child, unsigned width, unsigned height, const SmartUtil::tstring & txt );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline WidgetCoolbar::Seed::Seed()
{
	* this = WidgetCoolbar::getDefaultSeed();
}

inline WidgetCoolbar::WidgetCoolbar( SmartWin::Widget * parent )
	: PolicyType( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

inline void WidgetCoolbar::refresh()
{
	// This might look a bit stupid, but Windows API have some minor flaws. One of
	// those flaws is that a Coolbar (and a Toolbar) control must be "resized" with
	// a dummy value to make sure the Coolbar (&& the Toolbar) fills up the
	// complete area of the container Widget...

	// HC comment: sorry ;( but no smiley faces ;) anywhere, not even in comments.
	// They mess up with my macros to check delimiters ...
	SmartWin::Rectangle rect;
	if ( ::MoveWindow( this->handle(),
		rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, TRUE ) == 0 )
	{
		xCeption err( _T( "Couldn't reposition windows" ) );
		throw err;
	}
}

// end namespace SmartWin
}

#endif  //WINCE
#endif  //WidgetCoolbar_h
