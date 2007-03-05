// $Revision: 1.20 $
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
#ifndef WidgetRichTextBox_h
#define WidgetRichTextBox_h

#ifndef WINCE // Doesn't exist in Windows CE based systems

#include <richedit.h>
#include <commctrl.h>
#include "WidgetTextBox.h"
#include "../LibraryLoader.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

template< class EventHandlerClass, class MessageMapPolicy, class TextBoxType >
class WidgetRichTextBox;

template< class EventHandlerClass, class MessageMapPolicy >
class RichTextBox
{
public:
	enum canSetReadOnly
	{};
	typedef WidgetRichTextBox< EventHandlerClass, MessageMapPolicy, RichTextBox< EventHandlerClass, MessageMapPolicy > > TextBoxType;
};

/// RichEdit Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html richedit.PNG
  * Class for creating a Rich Edit control. <br>
  * A Rich Edit Control is derived from WidgetTextBox and inherits ( mostly ) all 
  * properties and member functions of the WidgetTextBox Widget. <br>
  * In addition to the WidgetTextBox WidgetRichTextBox can display colored text, 
  * hyperlinks, OLE objects etc. <br>
  * A good example of the difference between those Widgets is the difference between 
  * notepad ( WidgetTextBox ) and wordpad ( WidgetRichTextBox )   
  */
template< class EventHandlerClass, class MessageMapPolicy, class TextBoxType /*only her to get the number of template parameters right in the base class*/ >
class WidgetRichTextBox :
	private virtual TrueWindow,
	public WidgetTextBox< EventHandlerClass, MessageMapPolicy, /*This is to DISABLE the OnlyEditControl thingies, Magic Enum Construct!*/RichTextBox< EventHandlerClass, MessageMapPolicy > >
{
	typedef MessageMapControl< EventHandlerClass, typename TextBoxType::TextBoxType, MessageMapPolicy > ThisMessageMap;
	friend class WidgetCreator< WidgetRichTextBox >;
public:
	/// Class type
	typedef WidgetRichTextBox< EventHandlerClass, MessageMapPolicy, TextBoxType > ThisType;

	/// Object type
	typedef WidgetRichTextBox< EventHandlerClass, MessageMapPolicy, TextBoxType > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetRichTextBox::ThisType WidgetType;

		FontPtr font;
		COLORREF backgroundColor;
		bool scrollBarHorizontallyFlag;
		bool scrollBarVerticallyFlag;
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

	// Removing compiler hickup...
	virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar );

	/// Actually creates the Rich Edit Control
	/** You should call WidgetFactory::createRichTextBox if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

	/// Sets the background color of the WidgetRichTextBox
	/** Call this function to alter the background color of the WidgetRichEdit. <br>
	  * To create a COLORREF ( color ) use the RGB macro.
	  */
	void setBackgroundColor( COLORREF color );

protected:
	// Constructor Taking pointer to parent
	explicit WidgetRichTextBox( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetRichTextBox()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy, class TextBoxType >
const typename WidgetRichTextBox< EventHandlerClass, MessageMapPolicy, TextBoxType >::Seed & WidgetRichTextBox< EventHandlerClass, MessageMapPolicy, TextBoxType >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, RICHEDIT_CLASS );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_MULTILINE | WS_BORDER | ES_WANTRETURN;
		d_DefaultValues.backgroundColor = RGB( 255, 255, 255 );
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_DefaultValues.scrollBarHorizontallyFlag = false;
		d_DefaultValues.scrollBarVerticallyFlag = false;
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy, class TextBoxType >
WidgetRichTextBox< EventHandlerClass, MessageMapPolicy, TextBoxType >::Seed::Seed()
{
	* this = WidgetRichTextBox::getDefaultSeed();
}

template< class EventHandlerClass, class MessageMapPolicy, class TextBoxType >
LRESULT WidgetRichTextBox< EventHandlerClass, MessageMapPolicy, TextBoxType >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
}

template< class EventHandlerClass, class MessageMapPolicy, class TextBoxType >
void WidgetRichTextBox< EventHandlerClass, MessageMapPolicy, TextBoxType >::create( const Seed & cs )
{
	// Need to load up RichEdit library!
	static LibraryLoader richEditLibrary( _T( "riched20.dll" ) );

	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetRichTextBox::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();
	setFont( cs.font );
	setBackgroundColor( cs.backgroundColor );
	setScrollBarHorizontally( cs.scrollBarHorizontallyFlag );
	setScrollBarVertically( cs.scrollBarVerticallyFlag );
}

template< class EventHandlerClass, class MessageMapPolicy, class TextBoxType >
WidgetRichTextBox< EventHandlerClass, MessageMapPolicy, TextBoxType >::WidgetRichTextBox( SmartWin::Widget * parent )
	: WidgetTextBox< EventHandlerClass, MessageMapPolicy, TextBoxType >( parent )
	, Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Cant have a TextBox without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy, class TextBoxType >
void WidgetRichTextBox< EventHandlerClass, MessageMapPolicy, TextBoxType >::setBackgroundColor( COLORREF color )
{
	::SendMessage( this->Widget::itsHandle, EM_SETBKGNDCOLOR, 0, static_cast< LPARAM >( color ) );
}

// end namespace SmartWin
}

#endif //! WINCE

#endif
