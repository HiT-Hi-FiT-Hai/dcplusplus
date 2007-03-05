// $Revision: 1.24 $
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
#ifndef WidgetRadioButton_h
#define WidgetRadioButton_h

#include "../Widget.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectText.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectBorder.h"
#include "../MessageMapControl.h"
#include "../TrueWindow.h"
#include "../aspects/AspectFont.h"
#include "../xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

 /** sideeffect = \par Side Effects :
   */
/// Button Control class
 /** \ingroup WidgetControls
   * \WidgetUsageInfo
   * \image html radiogroup.PNG
   * Class for creating a Radio Button Control. <br>
   * A Radio Button is a Widget which can be grouped together with other Radio Button
   * controls and only ONE of them can be "selected" at a time, it can in addition
   * contain descriptive text. <br>
   * By selecting one of the Radio Buttons grouped together you will also deselect the
   * previously selected one.
   */
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetRadioButton :
	public MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >,
	public virtual TrueWindow,

	// Aspects
	public AspectBackgroundColor< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectBorder< WidgetRadioButton< EventHandlerClass, MessageMapPolicy > >,
	public AspectClickable< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectDblClickable< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectEnabled< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFocus< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFont< WidgetRadioButton< EventHandlerClass, MessageMapPolicy > >,
	public AspectPainting< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectSizable< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectText< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectThreads< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetRadioButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetRadioButton, MessageMapPolicy > ThisMessageMap;
	friend class WidgetCreator< WidgetRadioButton >;
public:
	/// Class type
	typedef WidgetRadioButton< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetRadioButton< EventHandlerClass, MessageMapPolicy > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetRadioButton::ThisType WidgetType;

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

	// Contract needed by AspectClickable Aspect class
	static inline Message & getClickMessage();

	// Contract needed by AspectClickable Aspect class
	static inline Message & getDblClickMessage();

	// Contract needed by AspectBackgroundColor Aspect class
	static inline Message & getBackgroundColorMessage();

	/// Returns true if the RadioButton is selected
	 /** Call this function to determine if the RadioButton is selected or not,
	   * returns true if it is selected
	   */
	bool getChecked();

	/// Sets the checked value of the RadioButton
	 /** Call this function to either check the RadioButton or to uncheck the
	   * RadioButton
	   */
	void setChecked( bool value = true );

	/// Actually creates the Button Control
	/** You should call WidgetFactory::createRadioButton if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( EventHandlerClass * parent, const Seed & cs = getDefaultSeed() );

protected:
	// Constructor Taking pointer to parent
	explicit WidgetRadioButton( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetRadioButton()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetRadioButton< EventHandlerClass, MessageMapPolicy >::Seed & WidgetRadioButton< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, _T( "BUTTON" ) );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetRadioButton< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetRadioButton::getDefaultSeed();
}

template< class EventHandlerClass, class MessageMapPolicy >
LRESULT WidgetRadioButton< EventHandlerClass, MessageMapPolicy >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetRadioButton< EventHandlerClass, MessageMapPolicy >::getClickMessage()
{
	static Message retVal = Message( WM_COMMAND, BN_CLICKED );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetRadioButton< EventHandlerClass, MessageMapPolicy >::getDblClickMessage()
{
	static Message retVal = Message( WM_COMMAND, BN_DBLCLK );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetRadioButton< EventHandlerClass, MessageMapPolicy >::getBackgroundColorMessage()
{
	static Message retVal = Message( WM_CTLCOLORBTN );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetRadioButton< EventHandlerClass, MessageMapPolicy >::WidgetRadioButton( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetRadioButton< EventHandlerClass, MessageMapPolicy >::getChecked()
{
	return ::SendMessage( this->Widget::itsHandle, BM_GETCHECK, 0, 0 ) == BST_CHECKED;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetRadioButton< EventHandlerClass, MessageMapPolicy >::setChecked( bool value )
{
	::SendMessage( this->Widget::itsHandle, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0 );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetRadioButton< EventHandlerClass, MessageMapPolicy >::create( EventHandlerClass * parent, const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetRadioButton::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	this->Widget::itsParent = parent;
	ThisMessageMap::createMessageMap();
	setFont( cs.font );
	// TODO: this was registered with the application when Widget::create was
	// called. Will they collide?
	this->registerWidget();
}

// end namespace SmartWin
}

#endif
