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
#include "../MessageMapPolicyClasses.h"
#include "../aspects/AspectBackgroundColor.h"
#include "../aspects/AspectBorder.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectMouseClicks.h"
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
class WidgetTextBoxBase :
	public MessageMapPolicy< Policies::Subclassed >,

	// Aspect classes
	public AspectBackgroundColor< WidgetTextBoxBase >,
	public AspectBorder< WidgetTextBoxBase >,
	public AspectEnabled< WidgetTextBoxBase >,
	public AspectFocus< WidgetTextBoxBase >,
	public AspectFont< WidgetTextBoxBase >,
	public AspectKeyboard< WidgetTextBoxBase >,
	public AspectMouseClicks< WidgetTextBoxBase >,
	public AspectRaw< WidgetTextBoxBase >,
	public AspectSizable< WidgetTextBoxBase >,
	public AspectText< WidgetTextBoxBase >,
	public AspectUpdate< WidgetTextBoxBase >,
	public AspectVisible< WidgetTextBoxBase >
{
	friend class WidgetCreator< WidgetTextBoxBase >;

	typedef Dispatchers::VoidVoid<> Dispatcher;

public:
	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;

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

	/// Adds (or removes) the readonly property
	/** If you pass false you remove this ability <br>
	  * If you pass true or call function without arguments you force the control to
	  * display as a readonly text field.
	  */
	void setReadOnly( bool value = true );

	bool isReadOnly();
	
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

	void onTextChanged( const Dispatcher::F& f );
	
	int lineIndex(int l = -1);
	
	int lineFromChar(int c = -1);
	
	void setModify(bool modify = false);
	
	bool getModify();

protected:
	// Constructor Taking pointer to parent
	explicit WidgetTextBoxBase( SmartWin::Widget * parent );

	// To assure nobody accidentally deletes any heaped object of this type, parent
	// is supposed to do so when parent is killed...
	virtual ~WidgetTextBoxBase()
	{}
};

class WidgetTextBox : 
	public WidgetTextBoxBase 
{
public:
	typedef WidgetTextBox ThisType;
	
	typedef ThisType* ObjectType;
	
	/// Info for creation
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef WidgetTextBox::ThisType WidgetType;

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

	Point getContextMenuPos();
	
	int charFromPos(const SmartWin::Point& pt);
	
	int lineFromPos(const SmartWin::Point& pt);
	
	int lineIndex(int line);
	
	int lineLength(int c);
	
	SmartUtil::tstring getLine(int line);

	SmartUtil::tstring textUnderCursor(const Point& p);
	
	/// Actually creates the TextBox
	/** You should call WidgetFactory::createTextBox if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	friend class WidgetCreator< WidgetTextBox >;

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

inline Message & WidgetTextBoxBase::getUpdateMessage()
{
	static Message retVal = Message( WM_COMMAND, EN_UPDATE );
	return retVal;
}

inline Message & WidgetTextBoxBase::getBackgroundColorMessage()
{
	// TODO What if readonly status changes?
	static Message rw = Message( WM_CTLCOLOREDIT );
	static Message ro = Message( WM_CTLCOLORSTATIC );
	
	return this->isReadOnly() ? ro : rw;
}

inline void WidgetTextBoxBase::setSelection( long start, long end )
{
	this->sendMessage(EM_SETSEL, start, end );
}

inline void WidgetTextBoxBase::replaceSelection( const SmartUtil::tstring & txt, bool canUndo )
{
	this->sendMessage(EM_REPLACESEL, static_cast< WPARAM >( canUndo ? TRUE : FALSE ), reinterpret_cast< LPARAM >( txt.c_str() ) );
}

inline void WidgetTextBoxBase::addText( const SmartUtil::tstring & addtxt )
{
	setSelection( ( long ) this->getText().size() );
	replaceSelection( addtxt ); 
}

inline void WidgetTextBoxBase::addTextLines( const SmartUtil::tstring & addtxt )
{
	setSelection( ( long ) this->getText().size() );
	replaceSelection( this->replaceEndlWithLfCr( addtxt ) ); 
}

inline long WidgetTextBoxBase::findText( const SmartUtil::tstring & txt, unsigned offset ) const
{
	SmartUtil::tstring txtOfBox = this->getText();
	size_t position = txtOfBox.find( txt, offset );
	if ( position == std::string::npos )
		return - 1;
	return static_cast< long >( position );
}

inline long WidgetTextBoxBase::getCaretPos() {
	DWORD start, end;
	this->sendMessage(EM_GETSEL, reinterpret_cast< WPARAM >( & start ), reinterpret_cast< LPARAM >( & end ) );
	return static_cast< long >( end );
}

inline void WidgetTextBoxBase::showCaret() {
	this->sendMessage(EM_SCROLLCARET);
}

inline void WidgetTextBoxBase::setScrollBarHorizontally( bool value ) {
	Widget::addRemoveStyle( WS_HSCROLL, value );
}

inline void WidgetTextBoxBase::setScrollBarVertically( bool value ) {
	Widget::addRemoveStyle( WS_VSCROLL, value );
}

inline void WidgetTextBoxBase::setReadOnly( bool value ) {
	this->sendMessage(EM_SETREADONLY, static_cast< WPARAM >( value ) );
}

inline bool WidgetTextBoxBase::isReadOnly( ) {	
	return hasStyle(ES_READONLY);
}

inline void WidgetTextBoxBase::setBorder( bool value ) {
	this->Widget::addRemoveStyle( WS_BORDER, value );
}

inline void WidgetTextBoxBase::setTextLimit( DWORD maxChars ) { 
	this->sendMessage(EM_LIMITTEXT, static_cast< WPARAM >(maxChars) ); 
} 
 
inline DWORD WidgetTextBoxBase::getTextLimit() const { 
	return static_cast< DWORD >( this->sendMessage(EM_GETLIMITTEXT) );
}

inline void WidgetTextBoxBase::onTextChanged( const Dispatcher::F& f ) {
	this->setCallback(
		Message( WM_COMMAND, EN_CHANGE ), Dispatcher(f)
	);
}

inline int WidgetTextBoxBase::lineFromChar( int c ) {
	return this->sendMessage( EM_LINEFROMCHAR, c );
}

inline int WidgetTextBoxBase::lineIndex( int l ) {
	return this->sendMessage( EM_LINEINDEX, l );
}

inline void WidgetTextBoxBase::setModify( bool modify ) {
	this->sendMessage( EM_SETMODIFY, modify );
}

inline bool WidgetTextBoxBase::getModify( ) {
	return this->sendMessage( EM_GETMODIFY ) > 0;
}

inline WidgetTextBox::Seed::Seed() {
	* this = WidgetTextBox::getDefaultSeed();
}

inline WidgetTextBoxBase::WidgetTextBoxBase( SmartWin::Widget * parent )
	: PolicyType( parent )
{
}

inline WidgetTextBox::WidgetTextBox( SmartWin::Widget * parent )
	: WidgetTextBoxBase( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Cant have a TextBox without a parent..." ) );
}

inline void WidgetTextBox::setPassword( bool value, TCHAR pwdChar ) {
	this->sendMessage(EM_SETPASSWORDCHAR, static_cast< WPARAM >( value ? pwdChar : 0 ));
}

inline void WidgetTextBox::setNumbersOnly( bool value ) {
	this->Widget::addRemoveStyle( ES_NUMBER, value );
}

inline void WidgetTextBox::setLowerCase( bool value ) {
	this->Widget::addRemoveStyle( ES_LOWERCASE, value );
}

inline void WidgetTextBox::setUpperCase( bool value ) {
	this->Widget::addRemoveStyle( ES_UPPERCASE, value );
}

inline int WidgetTextBox::charFromPos(const SmartWin::Point& pt) {		
	LPARAM lp = MAKELPARAM(pt.x, pt.y);
	return LOWORD(::SendMessage(this->handle(), EM_CHARFROMPOS, 0, lp));
}

inline int WidgetTextBox::lineFromPos(const SmartWin::Point& pt) {
	LPARAM lp = MAKELPARAM(pt.x, pt.y);
	return HIWORD(::SendMessage(this->handle(), EM_CHARFROMPOS, 0, lp));
}

inline int WidgetTextBox::lineIndex(int line) {
	return static_cast<int>(::SendMessage(this->handle(), EM_LINEINDEX, static_cast<WPARAM>(line), 0));
}

inline int WidgetTextBox::lineLength(int c) {
	return static_cast<int>(::SendMessage(this->handle(), EM_LINELENGTH, static_cast<WPARAM>(c), 0));
}

// end namespace SmartWin
}

#endif
