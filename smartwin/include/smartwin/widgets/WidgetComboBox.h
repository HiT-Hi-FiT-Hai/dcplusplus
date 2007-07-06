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
#include "../MessageMapPolicyClasses.h"
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
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectMouseClicks.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectThreads.h"
#include "../aspects/AspectBorder.h"
#include "../aspects/AspectText.h"
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
template< class EventHandlerClass >
class WidgetComboBox :
	public MessageMapPolicy< Policies::Subclassed >,
	private virtual TrueWindow,

	// Aspects
	public AspectBackgroundColor< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectBorder< WidgetComboBox< EventHandlerClass > >,
	public AspectClickable< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectDblClickable< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectEnabled< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectFocus< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectFont< WidgetComboBox< EventHandlerClass > >,
	public AspectKeyboard< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectMouseClicks< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectPainting< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectRaw< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectSelection< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectSizable< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectText< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectThreads< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >,
	public AspectVisible< EventHandlerClass, WidgetComboBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetComboBox< EventHandlerClass > > >
{
	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;
	typedef MessageMapControl< EventHandlerClass, WidgetComboBox > MessageMapType;
	friend class WidgetCreator< WidgetComboBox >;
public:
	/// Class type
	typedef WidgetComboBox< EventHandlerClass > ThisType;

	/// Object type
	typedef ThisType * ObjectType;

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

		/// Use extended ui
		bool extended;
		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

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

	/// Appends a value to the ComboBox.
	/** The return value is the index of the new item appended.
	  */
	int insertValue(int pos, const SmartUtil::tstring & val );

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

template< class EventHandlerClass >
const typename WidgetComboBox< EventHandlerClass >::Seed & WidgetComboBox< EventHandlerClass >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, WC_COMBOBOX );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_DefaultValues.extended = true;
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass >
WidgetComboBox< EventHandlerClass >::Seed::Seed()
{
	* this = WidgetComboBox::getDefaultSeed();
}

template< class EventHandlerClass >
Message & WidgetComboBox< EventHandlerClass >::getSelectionChangedMessage()
{
	static Message retVal = Message( WM_COMMAND, CBN_SELENDOK );
	return retVal;
}

template< class EventHandlerClass >
Message & WidgetComboBox< EventHandlerClass >::getClickMessage()
{
	static Message retVal = Message( WM_COMMAND, CBN_DROPDOWN );
	return retVal;
}

template< class EventHandlerClass >
Message & WidgetComboBox< EventHandlerClass >::getDblClickMessage()
{
	static Message retVal = Message( WM_COMMAND, CBN_DBLCLK );
	return retVal;
}

template< class EventHandlerClass >
Message & WidgetComboBox< EventHandlerClass >::getBackgroundColorMessage()
{
	static Message retVal = Message( WM_CTLCOLORLISTBOX );
	return retVal;
}

template< class EventHandlerClass >
int WidgetComboBox< EventHandlerClass >::getSelectedIndex() const
{
	int retVal = ComboBox_GetCurSel( this->Widget::itsHandle );
	return retVal;
}

template< class EventHandlerClass >
void WidgetComboBox< EventHandlerClass >::setSelectedIndex( int idx )
{
	ComboBox_SetCurSel( this->Widget::itsHandle, idx );
}

template< class EventHandlerClass >
SmartUtil::tstring WidgetComboBox< EventHandlerClass >::getSelectedValue()
{
	int txtLength = ::GetWindowTextLength( this->Widget::itsHandle );
	boost::scoped_array< TCHAR >
		txt( new TCHAR[++txtLength] );
	::GetWindowText( this->Widget::itsHandle, txt.get(), txtLength );
	SmartUtil::tstring retVal = txt.get();
	return retVal;
}

template< class EventHandlerClass >
void WidgetComboBox< EventHandlerClass >::removeAllItems()
{
	ComboBox_ResetContent( this->Widget::itsHandle );
}

template< class EventHandlerClass >
void WidgetComboBox< EventHandlerClass >::removeItem( int index )
{
	ComboBox_DeleteString( this->Widget::itsHandle, index );
}

template< class EventHandlerClass >
int WidgetComboBox< EventHandlerClass >::addValue( const SmartUtil::tstring & val )
{
	int newIdx = ComboBox_AddString( this->Widget::itsHandle, ( TCHAR * ) val.c_str() );
	if ( newIdx == CB_ERR )
	{
		xCeption x( _T( "Error while trying to add string into ComboBox" ) );
		throw x;
	}
	return newIdx;
}

template< class EventHandlerClass >
int WidgetComboBox< EventHandlerClass >::insertValue( int pos, const SmartUtil::tstring & val )
{
	int newIdx = ComboBox_InsertString( this->Widget::itsHandle, pos, ( TCHAR * ) val.c_str() );
	if ( newIdx == CB_ERR )
	{
		xCeption x( _T( "Error while trying to insert string into ComboBox" ) );
		throw x;
	}
	return newIdx;
}


template< class EventHandlerClass >
int WidgetComboBox< EventHandlerClass >::getCount()
{
	return ComboBox_GetCount( this->Widget::itsHandle ); // Number of items present.
}

template< class EventHandlerClass >
SmartUtil::tstring WidgetComboBox< EventHandlerClass >::getValue( int index )
{
	// Uses CB_GETLBTEXTLEN and CB_GETLBTEXT
	int txtLength = ComboBox_GetLBTextLen( this->Widget::itsHandle, index );
	boost::scoped_array< TCHAR >
		txt( new TCHAR[++txtLength] );
	ComboBox_GetLBText( this->Widget::itsHandle, index, txt.get() );
	SmartUtil::tstring retVal = txt.get();
	return retVal;
}

template< class EventHandlerClass >
WidgetComboBox< EventHandlerClass >::WidgetComboBox( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a ComboBox without a parent...
	xAssert( parent, _T( "Cant have a WidgetComboBox without a parent..." ) );
}

template< class EventHandlerClass >
void WidgetComboBox< EventHandlerClass >::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, "Widget must have WS_CHILD style");
	PolicyType::create(cs);
	setFont( cs.font );
	if(cs.extended) {
		::SendMessage(this->handle(), CB_SETEXTENDEDUI, TRUE, 0);
	}
}

// end namespace SmartWin
}

#endif
