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

#include "WidgetTextBox.h"
#include "../LibraryLoader.h"
#include <richedit.h>

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

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
class WidgetRichTextBox :
	public WidgetTextBoxBase
{
	friend class WidgetCreator< WidgetRichTextBox >;
public:
	/// Class type
	typedef WidgetRichTextBox ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public Widget::Seed
	{
	public:
		typedef WidgetRichTextBox::ThisType WidgetType;

		FontPtr font;
		COLORREF backgroundColor;
		bool scrollBarHorizontallyFlag;
		bool scrollBarVerticallyFlag;

		/// Fills with default parameters
		Seed();
	};

	/// Actually creates the Rich Edit Control
	/** You should call WidgetFactory::createRichTextBox if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	void create( const Seed & cs = Seed() );

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

inline WidgetRichTextBox::WidgetRichTextBox( SmartWin::Widget * parent )
	: WidgetTextBoxBase( parent )
{
}

inline void WidgetRichTextBox::setBackgroundColor( COLORREF color )
{
	this->sendMessage(EM_SETBKGNDCOLOR, 0, static_cast< LPARAM >( color ) );
}

// end namespace SmartWin
}

#endif //! WINCE

#endif
