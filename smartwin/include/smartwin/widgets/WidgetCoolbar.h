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

#include "boost.h"
#include "SmartUtil.h"
#include "../Widget.h"
#include "../MessageMapControl.h"
#include "../xCeption.h"

#include "WidgetComboBox.h"
#include "WidgetTextBox.h"
#include "WidgetGroupBox.h"

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
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetCoolbar :
	public MessageMapControl< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >,

	// Aspects
	public AspectEnabled< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFocus< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFont< WidgetCoolbar< EventHandlerClass, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	private AspectSizable< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetCoolbar, MessageMapPolicy > MessageMapType;
	typedef MessageMapControl< EventHandlerClass, WidgetCoolbar, MessageMapPolicy > ThisMessageMap;
	typedef AspectSizable< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetCoolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > > AspectSizable;
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
	typedef WidgetCoolbar< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetCoolbar< EventHandlerClass, MessageMapPolicy > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetCoolbar::ThisType TheWidgetType;

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

	/// ComboBox class type.
	typedef WidgetComboBox< EventHandlerClass, MessageMapPolicy > WidgetComboBox;

	/// ComboBox object type.
	typedef typename WidgetComboBox::ObjectType WidgetComboBoxPtr;

	/// TextBox class type.
	typedef WidgetTextBox< EventHandlerClass, MessageMapPolicy > WidgetTextBox;

	/// TextBox object type.
	typedef typename WidgetTextBox::ObjectType WidgetTextBoxPtr;

	/// Button class type.
	typedef WidgetButton< EventHandlerClass, MessageMapPolicy > WidgetButton;

	/// Button object type.
	typedef typename WidgetButton::ObjectType WidgetButtonPtr;

	/// Button class type.
	typedef WidgetTreeView< EventHandlerClass, MessageMapPolicy > WidgetTreeView;

	/// Button object type.
	typedef typename WidgetTreeView::ObjectType WidgetTreeViewPtr;

	/// CheckBox class type.
	typedef WidgetCheckBox< EventHandlerClass, MessageMapPolicy > WidgetCheckBox;

	/// CheckBox object type.
	typedef typename WidgetCheckBox::ObjectType WidgetCheckBoxPtr;

	/// Spinner class type.
	typedef WidgetSpinner< EventHandlerClass, MessageMapPolicy > WidgetSpinner;

	/// Spinner object type.
	typedef typename WidgetSpinner::ObjectType WidgetSpinnerPtr;

	/// GroupBox class type.
	typedef WidgetGroupBox< EventHandlerClass, MessageMapPolicy > WidgetGroupBox;

	/// GroupBox object type.
	typedef typename WidgetGroupBox::ObjectType WidgetGroupBoxPtr;

	/// RadioButton class type.
	typedef WidgetRadioButton< EventHandlerClass, MessageMapPolicy > WidgetRadioButton;

	/// RadioButton object type.
	typedef typename WidgetRadioButton::ObjectType WidgetRadioButtonPtr;

	/// DateTimePicker class type.
	typedef WidgetDateTimePicker< EventHandlerClass, MessageMapPolicy > WidgetDateTimePicker;

	/// DateTimePicker object type.
	typedef typename WidgetDateTimePicker::ObjectType WidgetDateTimePickerPtr;

	/// DateTimePicker class type.
	typedef WidgetMenu< EventHandlerClass, MessageMapPolicy > WidgetMenu;

	/// DateTimePicker object type.
	typedef typename WidgetMenu::ObjectType WidgetMenuPtr;

	/// Actually creates the Coolbar
	/** You should call WidgetFactory::createCoolbar if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

	WidgetRadioButtonPtr sow( WidgetGroupBoxPtr & parent, const typename WidgetRadioButton::Seed & cs )
	{
		WidgetRadioButtonPtr retVal ( WidgetCreator< typename WidgetRadioButton::Seed::WidgetType >::create( parent, internal_::getTypedParentOrThrow < EventHandlerClass * >( this ), cs ) );
		retVal->setBounds( 0, 0, cs.location.size.x, cs.location.size.y );
		parent->addChild( retVal );
		return retVal;
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
	//typename WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::WidgetComboBoxPtr // Bug in VC++7.1 Koenig Lookup forces us to give full type of return value...
	//sow< typename WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::WidgetComboBox::Seed >
	// ( const typename WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::Seed & cs, const SmartUtil::tstring & label = _T("") )
	//{
	// typename WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::ObjectType retVal (WidgetCreator< typename WidgetCoolbar< EventHandlerClass, MessageMapPolicy > >::create( this, cs ));
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

template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::Seed & WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, REBARCLASSNAME );
		d_DefaultValues.exStyle = WS_EX_TOOLWINDOW;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | RBS_VARHEIGHT | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER;
		//TODO: fill the values
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetCoolbar::getDefaultCInfo();
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::WidgetCoolbar( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetCoolbar::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();
	//TODO: use CreationalInfo parameters
}

//template< class EventHandlerClass, class MessageMapPolicy >
//typename WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::WidgetMenuPtr
//WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::createMenu
// ( unsigned width
// , unsigned height
// , const SmartUtil::tstring & txt
// )
//{
// WidgetMenu::ObjectType retVal = WidgetCreator< WidgetMenu >::create( this );
// this->addChild( retVal.get(), width, height, txt );
// return retVal;
//}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::refresh()
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

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetCoolbar< EventHandlerClass, MessageMapPolicy >::addChild( Widget * child,
	unsigned width, unsigned height, const SmartUtil::tstring & txt
	)
{
	REBARBANDINFO rbBand;
	rbBand.cbSize = sizeof( REBARBANDINFO );
	rbBand.fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE;
	if ( txt != _T( "" ) )
	{
		rbBand.fMask |= RBBIM_TEXT;
		rbBand.lpText = const_cast < TCHAR * >( txt.c_str() );
	}
	rbBand.hwndChild = child->handle();
	rbBand.cxMinChild = width;
	rbBand.cyMinChild = height;
	rbBand.cx = width;
	rbBand.fStyle = 0; //RBBS_GRIPPERALWAYS;
	if ( SendMessage( this->handle(), RB_INSERTBAND, ( WPARAM ) - 1, ( LPARAM ) & rbBand ) == 0 )
	{
		throw xCeption( _T( "There was a problem when trying to insert a band into your Coolbar object!" ) );
	}
}

// end namespace SmartWin
}

#endif  //WINCE
#endif  //WidgetCoolbar_h
