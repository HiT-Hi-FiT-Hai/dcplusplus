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

#ifndef DWT_AspectFont_h
#define DWT_AspectFont_h

#include "../resources/Font.h"

namespace dwt {

/// Aspect class used by Widgets that have the possibility of setting the
/// "font" property of their objects.
/** \ingroup AspectClasses
  * E.g. the Table have a "font" Aspect therefore it realizes the AspectFont
  * through inheritance. <br>
  * Realizing the AspectFont means that a Widget can set the font used to render text
  * in the Widget, for a ComboBox this means that it will render items in the
  * dropdownlist and in the selected area with the given font while for a textbox
  * this means that it will render all text with the given font. <br>
  * Most Widgets which can render text in some way realize this Aspect.
  */
template< class WidgetType >
class AspectFont
{
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
public:
	/// Sets the font used by the Widget
	/** Changes the font of the Widget to the given font. Use the class Font to
	  * construct a font in which to set by this function.
	  */
	void setFont( const FontPtr& font, bool forceUpdate = true );

	/// Returns the font used by the Widget
	/** Returns the Font object currently being used by the Widget
	  */
	const FontPtr& getFont();

	// TODO: Remove credits and put on website!
	/// Function taking a PredefinedFontTypes type
	/** Examples are SystemFixedFont, SystemFont or DefaultGuiFont.
	  * -- credit to mm.
	  */
	void setFont( PredefinedFontTypes stockObjectFont, bool forceUpdate = true );
private:
	// Keep a reference around so it doesn't get deleted
	FontPtr font;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class WidgetType >
void AspectFont< WidgetType >::setFont( const FontPtr& font_, bool forceUpdate )
{
	font = font_;
	W().sendMessage(WM_SETFONT, reinterpret_cast< WPARAM >( font->handle() ), static_cast< LPARAM >( forceUpdate ) );
}

template< class WidgetType >
const FontPtr& AspectFont< WidgetType >::getFont()
{
	if(!font) {
		HFONT f = ( HFONT )W().sendMessage(WM_GETFONT);
		font = FontPtr( new Font( f, false ) );
	}
	return font;
}

template< class WidgetType >
void AspectFont< WidgetType >::setFont( PredefinedFontTypes stockObjectFont, bool forceUpdate )
{
	font = FontPtr();
	HANDLE hFont = static_cast< HFONT >( ::GetStockObject( stockObjectFont ) );
	W().sendMessage(WM_SETFONT, reinterpret_cast< WPARAM >( hFont ), static_cast< LPARAM >( forceUpdate ) );
}

}

#endif
