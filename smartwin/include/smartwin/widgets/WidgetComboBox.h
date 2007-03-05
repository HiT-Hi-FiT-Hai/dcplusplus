// $Revision: 1.25 $
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
#ifndef WidgetComboBox_h
#define WidgetComboBox_h

#include <commctrl.h>
#include "../MessageMapControl.h"
#include "../TrueWindow.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectSelection.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectKeyPressed.h"
#include "../aspects/AspectMouseClicks.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectThreads.h"
#include "../aspects/AspectBorder.h"
#include "../xCeption.h"
#include "SmartUtil.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/// ComboBox Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html combobox.PNG
  * Class for creating a ComboBox control Widget. <br>
  * A ComboBox is a drop down "curtain" making it possible for the user to choose one
  * value at a time from a list of values.
  */
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetComboBox :
	public MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >,
	private virtual TrueWindow,

	// Aspects
	public AspectBackgroundColor< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectBorder< WidgetComboBox< EventHandlerClass, MessageMapPolicy > >,
	public AspectClickable< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectDblClickable< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectEnabled< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFocus< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFont< WidgetComboBox< EventHandlerClass, MessageMapPolicy > >,
	public AspectKeyPressed< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectMouseClicks< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectPainting< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectSelection< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectSizable< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectThreads< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetComboBox, MessageMapPolicy > ThisMessageMap;
	friend class WidgetCreator< WidgetComboBox >;
public:
	/// Class type
	typedef WidgetComboBox< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetComboBox< EventHandlerClass, MessageMapPolicy > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetComboBox::ThisType WidgetType;

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

	// Aspect expectation implementation
	static Message & getSelectionChangedMessage();

	// Aspect expectation implementation
	static Message & getClickMessage();

	// Aspect expectation implementation
	static Message & getDblClickMessage();

	// Aspect expectation implementation
	static Message & getBackgroundColorMessage();

	// Commented in AspectSelection
	int getSelectedIndex() const;

	// Commented in AspectSelection
	void setSelectedIndex( int idx );

	/// Return the selected value of the ComboBox
	/** If no item is actually selected the return value is "".
	  */
	SmartUtil::tstring getSelectedValue();

	/// Removes all items from the ComboBox
	/** After this call there will be zero items in the ComboBox.
	  */
	void removeAllItems();

	/// Remove an item from the ComboBox
	void removeItem( int index );

	/// Appends a value to the ComboBox.
	/** The return value is the index of the new item appended.
	  */
	int addValue( const SmartUtil::tstring & val );

	/// Returns the number of items present in the ComboBox.
	/** Returns the number of items present in the ComboBox.
	  */
	int getCount();

	/// Returns the string at the zero - based index of the items present in the
	/// ComboBox.
	/** Returns the string of the ComboBox at the given index. <br>
	  * Possible indices are [0.. getCount() - 1]
	  */
	SmartUtil::tstring getValue( int index );

	/// Actually creates the ComboBox Control
	/** You should call WidgetFactory::createComboBox if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

	static bool isValidSelectionChanged( LPARAM lPar )
	{ return true;
	}

protected:
	/// Constructor Taking pointer to parent
	explicit WidgetComboBox( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetComboBox()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetComboBox< EventHandlerClass, MessageMapPolicy >::Seed & WidgetComboBox< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, WC_COMBOBOX );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetComboBox< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetComboBox::getDefaultSeed();
}

template< class EventHandlerClass, class MessageMapPolicy >
LRESULT WidgetComboBox< EventHandlerClass, MessageMapPolicy >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetComboBox< EventHandlerClass, MessageMapPolicy >::getSelectionChangedMessage()
{
	static Message retVal = Message( WM_COMMAND, CBN_SELENDOK );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetComboBox< EventHandlerClass, MessageMapPolicy >::getClickMessage()
{
	static Message retVal = Message( WM_COMMAND, CBN_DROPDOWN );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetComboBox< EventHandlerClass, MessageMapPolicy >::getDblClickMessage()
{
	static Message retVal = Message( WM_COMMAND, CBN_DBLCLK );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetComboBox< EventHandlerClass, MessageMapPolicy >::getBackgroundColorMessage()
{
	static Message retVal = Message( WM_CTLCOLORLISTBOX );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetComboBox< EventHandlerClass, MessageMapPolicy >::getSelectedIndex() const
{
	int retVal = ComboBox_GetCurSel( this->Widget::itsHandle );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetComboBox< EventHandlerClass, MessageMapPolicy >::setSelectedIndex( int idx )
{
	ComboBox_SetCurSel( this->Widget::itsHandle, idx );
}

template< class EventHandlerClass, class MessageMapPolicy >
SmartUtil::tstring WidgetComboBox< EventHandlerClass, MessageMapPolicy >::getSelectedValue()
{
	int txtLength = ::GetWindowTextLength( this->Widget::itsHandle );
	boost::scoped_array< TCHAR >
		txt( new TCHAR[++txtLength] );
	::GetWindowText( this->Widget::itsHandle, txt.get(), txtLength );
	SmartUtil::tstring retVal = txt.get();
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetComboBox< EventHandlerClass, MessageMapPolicy >::removeAllItems()
{
	ComboBox_ResetContent( this->Widget::itsHandle );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetComboBox< EventHandlerClass, MessageMapPolicy >::removeItem( int index )
{
	ComboBox_DeleteString( this->Widget::itsHandle, index );
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetComboBox< EventHandlerClass, MessageMapPolicy >::addValue( const SmartUtil::tstring & val )
{
	int newIdx = ComboBox_AddString( this->Widget::itsHandle, ( TCHAR * ) val.c_str() );
	if ( newIdx == CB_ERR )
	{
		xCeption x( _T( "Error while trying to add string into ComboBox" ) );
		throw x;
	}
	return newIdx;
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetComboBox< EventHandlerClass, MessageMapPolicy >::getCount()
{
	return ComboBox_GetCount( this->Widget::itsHandle ); // Number of items present.
}

template< class EventHandlerClass, class MessageMapPolicy >
SmartUtil::tstring WidgetComboBox< EventHandlerClass, MessageMapPolicy >::getValue( int index )
{
	// Uses CB_GETLBTEXTLEN and CB_GETLBTEXT
	int txtLength = ComboBox_GetLBTextLen( this->Widget::itsHandle, index );
	boost::scoped_array< TCHAR >
		txt( new TCHAR[++txtLength] );
	ComboBox_GetLBText( this->Widget::itsHandle, index, txt.get() );
	SmartUtil::tstring retVal = txt.get();
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetComboBox< EventHandlerClass, MessageMapPolicy >::WidgetComboBox( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a ComboBox without a parent...
	xAssert( parent, _T( "Cant have a WidgetComboBox without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetComboBox< EventHandlerClass, MessageMapPolicy >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetComboBox::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();
	setFont( cs.font );
}

// end namespace SmartWin
}

#endif
