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

#ifndef DWT_ComboBox_h
#define DWT_ComboBox_h

#include "../aspects/AspectColor.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectCollection.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectSelection.h"
#include "../aspects/AspectText.h"
#include "Control.h"

namespace dwt {

/// ComboBox Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html combobox.PNG
  * Class for creating a ComboBox control Widget. <br>
  * A ComboBox is a drop down "curtain" making it possible for the user to choose one
  * value at a time from a list of values.
  */
class ComboBox :
	public CommonControl,
	// Aspects
	public AspectClickable< ComboBox >,
	public AspectCollection<ComboBox, int>,
	public AspectColor< ComboBox >,
	public AspectColorCtlImpl<ComboBox>,
	public AspectDblClickable< ComboBox >,
	public AspectKeyboard< ComboBox >,
	public AspectFont< ComboBox >,
	public AspectPainting< ComboBox >,
	public AspectSelection< ComboBox, int >,
	public AspectText< ComboBox >
{
	typedef CommonControl BaseType;
	friend class WidgetCreator< ComboBox >;
	friend class AspectCollection<ComboBox, int>;
	friend class AspectColor<ComboBox>;
	friend class AspectSelection<ComboBox, int>;
	friend class AspectClickable<ComboBox>;
	friend class AspectDblClickable<ComboBox>;

public:
	/// Class type
	typedef ComboBox ThisType;

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

		/// Use extended ui
		bool extended;

		/// Fills with default parameters
		Seed();
	};

	/// Appends a value to the ComboBox.
	/** The return value is the index of the new item appended.
	  */
	int addValue( const tstring & val );

	/// Appends a value to the ComboBox.
	/** The return value is the index of the new item appended.
	  */
	int insertValue(int pos, const tstring & val );

	/// Returns the string at the zero - based index of the items present in the
	/// ComboBox.
	/** Returns the string of the ComboBox at the given index. <br>
	  * Possible indices are [0.. getCount() - 1]
	  */
	tstring getValue( int index );

	/// Actually creates the ComboBox Control
	/** You should call WidgetFactory::createComboBox if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	void create( const Seed & cs = Seed() );

protected:
	ComboBox(Widget* parent);
	
	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~ComboBox()
	{}
	
private:
	
	// AspectSelection
	int getSelectedImpl() const;
	void setSelectedImpl( int idx );

	// AspectCollection
	void eraseImpl( int row );
	void clearImpl();
	size_t sizeImpl() const;

	// Aspect expectation implementation
	static const Message& getSelectionChangedMessage();

	// Aspect expectation implementation
	static const Message& getClickMessage();

	// Aspect expectation implementation
	static const Message& getDblClickMessage();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline const Message& ComboBox::getSelectionChangedMessage() {
	static const Message retVal( WM_COMMAND, CBN_SELENDOK );
	return retVal;
}

inline const Message& ComboBox::getClickMessage() {
	static const Message retVal( WM_COMMAND, CBN_DROPDOWN );
	return retVal;
}

inline const Message& ComboBox::getDblClickMessage() {
	static const Message retVal( WM_COMMAND, CBN_DBLCLK );
	return retVal;
}

inline int ComboBox::getSelectedImpl() const {
	return ComboBox_GetCurSel( handle() );
}

inline void ComboBox::setSelectedImpl( int idx ) {
	ComboBox_SetCurSel( handle(), idx );
}

inline void ComboBox::clearImpl()
{
	ComboBox_ResetContent( handle() );
}

inline void ComboBox::eraseImpl( int index )
{
	ComboBox_DeleteString( handle(), index );
}

inline int ComboBox::addValue( const tstring & val )
{
	int newIdx = ComboBox_AddString( handle(), ( TCHAR * ) val.c_str() );
	if ( newIdx == CB_ERR )
	{
		dwtWin32DebugFail("Error while trying to add string into ComboBox");
	}
	return newIdx;
}

inline int ComboBox::insertValue( int pos, const tstring & val )
{
	int newIdx = ComboBox_InsertString( handle(), pos, ( TCHAR * ) val.c_str() );
	if ( newIdx == CB_ERR )
	{
		dwtWin32DebugFail("Error while trying to insert string into ComboBox");
	}
	return newIdx;
}

inline size_t ComboBox::sizeImpl() const {
	return static_cast<size_t>(ComboBox_GetCount( handle() )); // Number of items present.
}


inline ComboBox::ComboBox( Widget* parent ) : BaseType(parent) {

}

}

#endif
