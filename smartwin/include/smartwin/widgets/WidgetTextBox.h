// $Revision: 1.38 $
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
#ifndef WidgetTextBox_h
#define WidgetTextBox_h

#include "../../SmartUtil.h"
#include "../aspects/AspectBackgroundColor.h"
#include "../aspects/AspectBorder.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectText.h"
#include "../aspects/AspectUpdate.h"
#include "../aspects/AspectVisible.h"
#include "../xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

template< class TextBoxType >
class WidgetTextBox;

class NormalTextBox
{
public:
	enum canSetOnlyUpperCase
	{};
	enum canSetOnlyLowerCase
	{};
	enum canSetOnlyPassword
	{};
	enum canSetOnlyNumbers
	{};
	enum canSetReadOnly
	{};
	typedef WidgetTextBox< NormalTextBox > TextBoxType;
};

#ifdef _MSC_VER
#pragma warning( disable : 4101 )
#endif

/// Text Box Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html textbox.PNG
  * Class for creating a Text Box Control. <br>
  * An Text Box is a window in which you can write and copy, paste into, normally
  * you're favourite text editor which you're developing C++ in would be a Text Box
  * control. ( or perhaps a Rich Edit Control which has some additional features )
  * <br>
  * You can send and retrieve the text contained in the control and do lots of other
  * types of manipulation of the control. <br>
  * Related classes <br>
  * < ul > < li >WidgetRichTextBox< /li > < /ul >
  */
template< class TextBoxType = NormalTextBox >
class WidgetTextBox :
	public MessageMapPolicy< Policies::Subclassed >,

	// Aspect classes
	public AspectBackgroundColor< WidgetTextBox< TextBoxType > >,
	public AspectBorder< WidgetTextBox< TextBoxType > >,
	public AspectEnabled< WidgetTextBox< TextBoxType > >,
	public AspectFocus< WidgetTextBox< TextBoxType > >,
	public AspectFont< WidgetTextBox< TextBoxType > >,
	public AspectKeyboard< WidgetTextBox< TextBoxType > >,
	public AspectRaw< WidgetTextBox< TextBoxType > >,
	public AspectSizable< WidgetTextBox< TextBoxType > >,
	public AspectText< WidgetTextBox< TextBoxType > >,
	public AspectUpdate< WidgetTextBox< TextBoxType > >,
	public AspectVisible< WidgetTextBox< TextBoxType > >
{
	friend class WidgetCreator< WidgetTextBox >;
public:
	/// Class type
	typedef WidgetTextBox< TextBoxType > ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;

	/// Info for creation
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetTextBox< TextBoxType >::ThisType WidgetType;

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

	// Contract needed by AspectUpdate Aspect class
	static inline Message & getUpdateMessage();

	// Contract needed by AspectBackgroundColor Aspect class
	Message & getBackgroundColorMessage();

	/// Sets the current selection of the Edit Control
	/** Start means the offset of where the current selection shall start, if it is
	  * omitted it defaults to 0. <br>
	  * end means where it shall end, if it is omitted it defaults to - 1 or "the
	  * rest from start".
	  */
	void setSelection( long start = 0, long end = - 1 );

	/// Returns the current selected text from the text box
	/** The selected text of the text box is the return value from this.
	  */
	SmartUtil::tstring getSelection() const;


	/// Appends text to the text box
	/** The txt parameter is the new text to append to the text box.
	  */
	void addText( const SmartUtil::tstring & txt );

	/// Appends the text in the text box so that endl causes a new line.
	/** Just the same as addText except that CR are expanded to LF CR
	  * Replaces \n with \r\n so that Windows textbox understands "endl"
	  */
	void addTextLines( const SmartUtil::tstring & txt );



	/// Replaces the currently selected text in the text box with the given text parameter
	/** If canUndo is true this operation is stacked into the undo que ( can be
	  * undone ), else this operation cannot be undone. <br>
	  * Note! <br>
	  * If there is not currently any selected text, the input text is inserted at
	  * the current location of the caret.
	  */
	void replaceSelection( const SmartUtil::tstring & txt, bool canUndo = true );

	// TODO: Case sensitivity
	/// Finds the given text in the text field and returns true if successfully
	long findText( const SmartUtil::tstring & txt, unsigned offset = 0 ) const;

	/// Returns the position of the caret
	long getCaretPos();

	/// Call this function to scroll the caret into view
	/** If the caret is not visible within the currently scrolled in area, the Text
	  * Box will scroll either down or up until the caret is visible.
	  */
	void showCaret();

	/// Adds (or removes) the control horizontal scroll bars
	/** If you pass false you REMOVE the horizontal scrollbars of the control ( if
	  * there are any ) <br>
	  * If you pass true to the function, you ADD horizontal scrollbars. <br>
	  * Value defaults to true!
	  */
	void setScrollBarHorizontally( bool value = true );

	/// Adds (or removes) the control vertical scroll bars
	/** If you pass false you REMOVE the vertical scrollbars of the control ( if
	  * there are any ) <br>
	  * If you pass true to the function, you ADD vertical scrollbars.
	  */
	void setScrollBarVertically( bool value = true );

	/// Adds (or removes) upper case forcing
	/** If you pass false you remove this ability <br>
	  * If you pass true or call function without arguments you force the control to
	  * display all characters in UPPER CASE.
	  */
	void setUpperCase( bool value = true );

	/// Adds (or removes) lower case forcing
	/** If you pass false you remove this ability <br>
	  * If you pass true or call function without arguments you force the control to
	  * display all characters in lower case.
	  */
	void setLowerCase( bool value = true );

	/// Adds (or removes) the readonly property
	/** If you pass false you remove this ability <br>
	  * If you pass true or call function without arguments you force the control to
	  * display as a readonly text field.
	  */
	void setReadOnly( bool value = true );

	bool isReadOnly();
	
	/// Adds (or removes) the numbers property
	/** If you pass false you remove this ability <br>
	  * If you pass true or call function without arguments you force the control to
	  * display only numbers.
	  */
	void setNumbersOnly( bool value = true );

	/// Adds (or removes) the password property
	/** If you pass false you remove this ability <br>
	  * If you pass true or call function without arguments you force the control to
	  * display all its content as "hidden" meaning it will only display e.g. "*"
	  * instead of letters, useful for Text Box Controls which contains passwords or
	  * similar "secret" information.
	  */
	void setPassword( bool value = true, TCHAR pwdChar = '*' );

	/// Adds (or removes) a border surrounding the control
	/** If you pass false you REMOVE the border of the control ( if there is on )
	  * <br>
	  * If you pass true to the function, you ADD a border.
	  */
	void setBorder( bool value = true );


	/// Set the maximum number of characters that can be entered.
	/** Although this prevents user from entering more maxChars, Paste can overrun the limit.
	  */
	void setTextLimit( DWORD maxChars );
	 
	/// Returns the maximum number of characters that can be entered.
	/** Note that the maxChars returned will vary by OS if left unset.
	  */
	DWORD getTextLimit() const ;


	/// Actually creates the TextBox
	/** You should call WidgetFactory::createTextBox if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	// Constructor Taking pointer to parent
	explicit WidgetTextBox( SmartWin::Widget * parent );

	// To assure nobody accidentally deletes any heaped object of this type, parent
	// is supposed to do so when parent is killed...
	virtual ~WidgetTextBox()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class TextBoxType >
const typename WidgetTextBox< TextBoxType >::Seed & WidgetTextBox< TextBoxType >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, _T("Edit") );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_WANTRETURN;
		d_DefaultValues.exStyle = WS_EX_CLIENTEDGE;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class TextBoxType >
WidgetTextBox< TextBoxType >::Seed::Seed()
{
	* this = WidgetTextBox::getDefaultSeed();
}

template< class TextBoxType >
Message & WidgetTextBox< TextBoxType >::getUpdateMessage()
{
	static Message retVal = Message( WM_COMMAND, EN_UPDATE );
	return retVal;
}

template< class TextBoxType >
Message & WidgetTextBox< TextBoxType >::getBackgroundColorMessage()
{
	// TODO What if readonly status changes?
	static Message rw = Message( WM_CTLCOLOREDIT );
	static Message ro = Message( WM_CTLCOLORSTATIC );
	
	return this->isReadOnly() ? ro : rw;
}



template< class TextBoxType >
void WidgetTextBox< TextBoxType >::setSelection( long start, long end )
{
	::SendMessage( this->Widget::itsHandle, EM_SETSEL, start, end );
}

template< class TextBoxType >
SmartUtil::tstring WidgetTextBox< TextBoxType >::getSelection() const
{
	DWORD start, end;
	::SendMessage( this->Widget::itsHandle, EM_GETSEL, reinterpret_cast< WPARAM >( & start ), reinterpret_cast< LPARAM >( & end ) );
	SmartUtil::tstring retVal = this->getText().substr( start, end - start );
	return retVal;
}

template< class TextBoxType >
void WidgetTextBox< TextBoxType >::replaceSelection( const SmartUtil::tstring & txt, bool canUndo )
{
	::SendMessage( this->Widget::itsHandle, EM_REPLACESEL, static_cast< WPARAM >( canUndo ? TRUE : FALSE ), reinterpret_cast< LPARAM >( txt.c_str() ) );
}

template< class TextBoxType >
void WidgetTextBox< TextBoxType >::addText( const SmartUtil::tstring & addtxt )
{
	setSelection( ( long ) this->getText().size() );
	replaceSelection( addtxt ); 
}


template< class TextBoxType >
void WidgetTextBox< TextBoxType >::addTextLines( const SmartUtil::tstring & addtxt )
{
	setSelection( ( long ) this->getText().size() );
	replaceSelection( this->replaceEndlWithLfCr( addtxt ) ); 
}



template< class TextBoxType >
long WidgetTextBox< TextBoxType >::findText( const SmartUtil::tstring & txt, unsigned offset ) const
{
	const SmartUtil::tstring & txtOfBox = this->getText();
	size_t position = txtOfBox.find( txt, offset );
	if ( position == std::string::npos )
		return - 1;
	return static_cast< long >( position );
}

template< class TextBoxType >
long WidgetTextBox< TextBoxType >::getCaretPos()
{
	DWORD start, end;
	::SendMessage( this->Widget::itsHandle, EM_GETSEL, reinterpret_cast< WPARAM >( & start ), reinterpret_cast< LPARAM >( & end ) );
	return static_cast< long >( end );
}

template< class TextBoxType >
void WidgetTextBox< TextBoxType >::showCaret()
{
	::SendMessage( this->Widget::itsHandle, EM_SCROLLCARET, 0, 0 );
}

template< class TextBoxType >
void WidgetTextBox< TextBoxType >::setScrollBarHorizontally( bool value )
{
	Widget::addRemoveStyle( WS_HSCROLL, value );
}

template< class TextBoxType >
void WidgetTextBox< TextBoxType >::setScrollBarVertically( bool value )
{
	Widget::addRemoveStyle( WS_VSCROLL, value );
}

template< class TextBoxType >
void WidgetTextBox< TextBoxType >::setUpperCase( bool value )
{
	// If you get a compile time error her, you are trying to call this function
	// with an unsupported TextBox type, probably with a RichTextBox which doesn't
	// support this style...
	typename TextBoxType::canSetOnlyUpperCase dummy;
	this->Widget::addRemoveStyle( ES_UPPERCASE, value );
}

template< class TextBoxType >
void WidgetTextBox< TextBoxType >::setLowerCase( bool value )
{
	// If you get a compile time error her, you are trying to call this function
	// with an unsupported TextBox type, probably with a RichTextBox which doesn't
	// support this style...
	typename TextBoxType::canSetOnlyLowerCase dummy;
	this->Widget::addRemoveStyle( ES_LOWERCASE, value );
}

template< class TextBoxType >
void WidgetTextBox< TextBoxType >::setReadOnly( bool value )
{
	// If you get a compile time error here, you are trying to call this function
	// with an unsupported TextBox type, probably with a RichTextBox which doesn't
	// support this style...
	typename TextBoxType::canSetReadOnly dummy;
	if ( value )
	{
		::SendMessage( this->Widget::itsHandle, EM_SETREADONLY, static_cast< WPARAM >( TRUE ), 0 );
	}
	else
	{
		::SendMessage( this->Widget::itsHandle, EM_SETREADONLY, static_cast< WPARAM >( FALSE ), 0 );
	}
}


template< class TextBoxType >
bool WidgetTextBox< TextBoxType >::isReadOnly( )
{	
	return (::GetWindowLong(this->handle(), GWL_STYLE) & ES_READONLY) == ES_READONLY;
}


template< class TextBoxType >
void WidgetTextBox< TextBoxType >::setNumbersOnly( bool value )
{
	// If you get a compile time error her, you are trying to call this function
	// with an unsupported TextBox type, probably with a RichTextBox which doesn't
	// support this style...
	typename TextBoxType::canSetOnlyNumbers dummy;
	this->Widget::addRemoveStyle( ES_NUMBER, value );
}

template< class TextBoxType >
void WidgetTextBox< TextBoxType >::setPassword( bool value, TCHAR pwdChar )
{
	// If you get a compile time error her, you are trying to call this function
	// with an unsupported TextBox type, probably with a RichTextBox which doesn't
	// support this style...
	typename TextBoxType::canSetOnlyPassword dummy;
	if ( value )
	{
		::SendMessage( this->Widget::itsHandle, EM_SETPASSWORDCHAR, static_cast< WPARAM >( pwdChar ), 0 );
	}
	else
	{
		::SendMessage( this->Widget::itsHandle, EM_SETPASSWORDCHAR, static_cast< WPARAM >( pwdChar ), 0 );
	}
}

template< class TextBoxType >
void WidgetTextBox< TextBoxType >::setBorder( bool value )
{
	this->Widget::addRemoveStyle( WS_BORDER, value );
}

template< class TextBoxType > 
void WidgetTextBox< TextBoxType >::setTextLimit( DWORD maxChars ) 
{ 
	::SendMessage( this->Widget::itsHandle, EM_LIMITTEXT, static_cast< WPARAM >(maxChars), 0 ); 
} 
 
template< class TextBoxType > 
DWORD WidgetTextBox< TextBoxType >::getTextLimit() const 
{ 
	return static_cast< DWORD >( ::SendMessage( this->Widget::itsHandle, EM_GETLIMITTEXT , 0, 0 ) );
}


template< class TextBoxType >
WidgetTextBox< TextBoxType >::WidgetTextBox( SmartWin::Widget * parent )
	: PolicyType( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Cant have a TextBox without a parent..." ) );
}

template< class TextBoxType >
void WidgetTextBox< TextBoxType >::create( const typename WidgetTextBox::Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, "Widget must have WS_CHILD style");
	PolicyType::create(cs);
	setFont( cs.font );
}

// end namespace SmartWin
}

#endif
