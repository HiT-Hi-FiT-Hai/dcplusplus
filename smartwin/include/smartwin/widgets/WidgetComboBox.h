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

#include "../MessageMapPolicyClasses.h"
#include "../aspects/AspectBackgroundColor.h"
#include "../aspects/AspectBorder.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectMouseClicks.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSelection.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectText.h"
#include "../aspects/AspectVisible.h"
#include "../xCeption.h"
#include "WidgetTextBox.h"

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
class WidgetComboBox :
	public MessageMapPolicy< Policies::Subclassed >,
	
	// Aspects
	public AspectBackgroundColor< WidgetComboBox >,
	public AspectBorder< WidgetComboBox >,
	public AspectClickable< WidgetComboBox >,
	public AspectDblClickable< WidgetComboBox >,
	public AspectEnabled< WidgetComboBox >,
	public AspectFocus< WidgetComboBox >,
	public AspectFont< WidgetComboBox >,
	public AspectKeyboard< WidgetComboBox >,
	public AspectMouseClicks< WidgetComboBox >,
	public AspectPainting< WidgetComboBox >,
	public AspectRaw< WidgetComboBox >,
	public AspectSelection< WidgetComboBox >,
	public AspectSizable< WidgetComboBox >,
	public AspectText< WidgetComboBox >,
	public AspectVisible< WidgetComboBox >
{
	friend class WidgetCreator< WidgetComboBox >;
public:
	/// Class type
	typedef WidgetComboBox ThisType;

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
		typedef WidgetComboBox::ThisType WidgetType;

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

	typedef WidgetTextBox::ObjectType WidgetTextBoxPtr;
	WidgetTextBoxPtr getTextBox();
	
protected:
	/// Constructor Taking pointer to parent
	explicit WidgetComboBox( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetComboBox()
	{}
	
private:
	WidgetTextBoxPtr textBox;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline WidgetComboBox::Seed::Seed()
{
	* this = WidgetComboBox::getDefaultSeed();
}

inline Message & WidgetComboBox::getSelectionChangedMessage()
{
	static Message retVal = Message( WM_COMMAND, CBN_SELENDOK );
	return retVal;
}

inline Message & WidgetComboBox::getClickMessage()
{
	static Message retVal = Message( WM_COMMAND, CBN_DROPDOWN );
	return retVal;
}

inline Message & WidgetComboBox::getDblClickMessage()
{
	static Message retVal = Message( WM_COMMAND, CBN_DBLCLK );
	return retVal;
}

inline Message & WidgetComboBox::getBackgroundColorMessage()
{
	static Message retVal = Message( WM_CTLCOLORLISTBOX );
	return retVal;
}

inline int WidgetComboBox::getSelectedIndex() const
{
	int retVal = ComboBox_GetCurSel( handle() );
	return retVal;
}

inline void WidgetComboBox::setSelectedIndex( int idx )
{
	ComboBox_SetCurSel( handle(), idx );
}

inline SmartUtil::tstring WidgetComboBox::getSelectedValue()
{
	int txtLength = ::GetWindowTextLength( handle() );
	boost::scoped_array< TCHAR >
		txt( new TCHAR[++txtLength] );
	::GetWindowText( handle(), txt.get(), txtLength );
	SmartUtil::tstring retVal = txt.get();
	return retVal;
}

inline void WidgetComboBox::removeAllItems()
{
	ComboBox_ResetContent( handle() );
}

inline void WidgetComboBox::removeItem( int index )
{
	ComboBox_DeleteString( handle(), index );
}

inline int WidgetComboBox::addValue( const SmartUtil::tstring & val )
{
	int newIdx = ComboBox_AddString( handle(), ( TCHAR * ) val.c_str() );
	if ( newIdx == CB_ERR )
	{
		xCeption x( _T( "Error while trying to add string into ComboBox" ) );
		throw x;
	}
	return newIdx;
}

inline int WidgetComboBox::insertValue( int pos, const SmartUtil::tstring & val )
{
	int newIdx = ComboBox_InsertString( handle(), pos, ( TCHAR * ) val.c_str() );
	if ( newIdx == CB_ERR )
	{
		xCeption x( _T( "Error while trying to insert string into ComboBox" ) );
		throw x;
	}
	return newIdx;
}


inline int WidgetComboBox::getCount()
{
	return ComboBox_GetCount( handle() ); // Number of items present.
}

inline SmartUtil::tstring WidgetComboBox::getValue( int index )
{
	// Uses CB_GETLBTEXTLEN and CB_GETLBTEXT
	int txtLength = ComboBox_GetLBTextLen( handle(), index );
	boost::scoped_array< TCHAR >
		txt( new TCHAR[++txtLength] );
	ComboBox_GetLBText( handle(), index, txt.get() );
	SmartUtil::tstring retVal = txt.get();
	return retVal;
}

inline WidgetComboBox::WidgetComboBox( SmartWin::Widget * parent )
	: PolicyType( parent ), textBox(0)
{
	// Can't have a ComboBox without a parent...
	xAssert( parent, _T( "Cant have a WidgetComboBox without a parent..." ) );
}

// end namespace SmartWin
}

#endif
