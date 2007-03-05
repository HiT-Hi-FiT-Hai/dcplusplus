// $Revision: 1.10 $
/*
  Copyright ( c ) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

	  * Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.
	  * Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.
	  * Neither the name of the SmartWin ++ nor the names of its contributors
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
#ifndef AspectFont_h
#define AspectFont_h

#include "boost.h"
#include "../Widget.h"
#include "../TrueWindow.h"
#include "../Font.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Aspect class used by Widgets that have the possibility of setting the
/// "font" property of their objects.
/** \ingroup AspectClasses
  * E.g. the WidgetDataGrid have a "font" Aspect therefore it realizes the AspectFont
  * through inheritance. <br>
  * Realizing the AspectFont means that a Widget can set the font used to render text
  * in the Widget, for a ComboBox this means that it will render items in the
  * dropdownlist and in the selected area with the given font while for a textbox
  * this means that it will render all text with the given font. <br>
  * Most Widgets which can render text in some way realize this Aspect.
  */
template< class WidgetType >
class AspectFont
	: private virtual TrueWindow
{
public:
	/// Sets the font used by the Widget
	/** Changes the font of the Widget to the given font. Use the class Font to
	  * construct a font in which to set by this function.
	  */
	void setFont( FontPtr font, bool forceUpdate = true );

	/// Returns the font used by the Widget
	/** Returns the Font object currently being used by the Widget
	  */
	FontPtr getFont();

	// TODO: Remove credits and put on website!
	/// Function taking a PredefinedFontTypes type
	/** Examples are SystemFixedFont, SystemFont or DefaultGuiFont.
	  * -- credit to mm.
	  */
	void setFont( PredefinedFontTypes stockObjectFont, bool forceUpdate = true );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class WidgetType >
void AspectFont< WidgetType >::setFont( FontPtr font, bool forceUpdate )
{
	::SendMessage( static_cast< WidgetType * >( this )->handle(), WM_SETFONT, reinterpret_cast< WPARAM >( font->getHandle() ), static_cast< LPARAM >( forceUpdate ) );
	addObject( font );
}

template< class WidgetType >
FontPtr AspectFont< WidgetType >::getFont()
{
	HFONT font = ( HFONT )::SendMessage( static_cast< WidgetType * >( this )->handle(), WM_GETFONT, 0, 0 );
	boost::shared_ptr< Font > retVal( new Font( font, false ) );
	return retVal;
}

template< class WidgetType >
void AspectFont< WidgetType >::setFont( PredefinedFontTypes stockObjectFont, bool forceUpdate )
{
	HANDLE hFont = static_cast< HFONT >( ::GetStockObject( stockObjectFont ) );
	::SendMessage( static_cast< WidgetType * >( this )->handle(), WM_SETFONT, reinterpret_cast< WPARAM >( hFont ), static_cast< LPARAM >( forceUpdate ) );
}

// end namespace SmartWin
}

#endif
