// $Revision: 1.12 $
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
#ifndef Font_h
#define Font_h

#include "SmartUtil.h"
#include "WindowsHeaders.h"
#include "WindowObject.h"
#include "xCeption.h"
#include "boost.h"

namespace SmartWin
{
// begin namespace SmartWin

#ifndef WINCE
/// Type of default installed fonts you can create with the Font class
/** If you want to use one of the default fonts from windows you can use this enum
  * and the Constructor to the Font class which takes the FontType parameter.
  */
typedef enum PredefinedFontTypes
	{
		SystemFixedFont = SYSTEM_FIXED_FONT,
		SystemFont = SYSTEM_FONT,
		OemFixedFont = OEM_FIXED_FONT,
		DefaultGuiFont = DEFAULT_GUI_FONT,
		DeviceDefaultFont = DEVICE_DEFAULT_FONT,
		AnsiVarFont = ANSI_VAR_FONT,
		AnsiFixedFont = ANSI_FIXED_FONT
	};
#else
/// Type of default installed fonts you can create with the Font class
/** If you want to use one of the default fonts from windows you can use this enum
  * and the Constructor to the Font class which takes the FontType parameter. Windows
  * CE only supports ONE type of predefined font for the moment which is SystemFont!
  */
typedef enum PredefinedFontTypes
{
	DefaultGuiFont = SYSTEM_FONT,
	SystemFont = SYSTEM_FONT
};
#endif

/// Class for creating a Font object.
/** This class is the type sent to the AspectFont realizing classes. <br>
  * Normally you would make an instance of this class and then stuff it into any
  * class object that realizes the AspectFont Aspect. <br>
  * One instance of this class can be shared among different Widgets ( even different
  * types of Widgets )
  */
class Font : public WindowObject
{
public:

	Font( PredefinedFontTypes inFontType )
		: isOwner( true )
	{
#ifndef WINCE
		itsHandle = static_cast< HFONT >( ::GetStockObject( inFontType ) );
		checkCreationOfFont();
#endif
	}

	/// Constructor taking all parameters
	/** The object can't be manipulated after creation so when creating an instance
	  * of this class be sure you are certain that you know what you want.
	  */
	Font( const SmartUtil::tstring & faceName, int height = 10,
		int width = 10,
		int weight = 2,
		DWORD charSet = ANSI_CHARSET,
		bool italic = false,
		bool underline = false,
		bool strikeOut = false,
		int escapementOrientation = 0,
		DWORD outputPrecision = OUT_DEFAULT_PRECIS,
		DWORD clipPrecision = CLIP_DEFAULT_PRECIS,
		DWORD quality = DEFAULT_QUALITY,
		DWORD pitchAndFamliy = FF_DONTCARE )
		: isOwner( true )
	{
		LOGFONT lf;
		lf.lfHeight = height;
		lf.lfWidth = width;
		lf.lfWeight = weight;
		lf.lfItalic = italic ? TRUE : FALSE;
		lf.lfUnderline = underline ? TRUE : FALSE;
		lf.lfStrikeOut = strikeOut ? TRUE : FALSE;
		lf.lfCharSet = static_cast< BYTE >( charSet );
		lf.lfOutPrecision = static_cast< BYTE >( outputPrecision );
		lf.lfClipPrecision = static_cast< BYTE >( clipPrecision );
		lf.lfQuality = static_cast< BYTE >( quality );
		lf.lfPitchAndFamily = static_cast< BYTE >( pitchAndFamliy );
#ifndef _tcscpy_s
		_tcsncpy( lf.lfFaceName, faceName.c_str(), 32 );
#else
		_tcscpy_s( lf.lfFaceName, 32, faceName.c_str() );
#endif
		lf.lfEscapement = escapementOrientation;
		itsHandle = ::CreateFontIndirect( & lf );
		checkCreationOfFont();
	}

	Font( HFONT font, bool owner )
		: isOwner( owner )
	{
		itsHandle = font;
	}
	/// DTOR cleaning up
	/** DTOR of this class automatically calls DeleteObject on the HFONT so be sure
	  * to create the instance as a SmartPtr if you wish to have access to it after
	  * the object goes out of scope.
	  */
	virtual ~Font()
	{
		if ( isOwner )
		{
			// If the font was made through CreateStockObject it is not (according
			// to documentation) dangerous to delete even this object, therefore
			// for simplicity we just do it anyway...
			::DeleteObject( itsHandle );
		}
	}

	/// Returns the HFONT to the Font object
	/** Be careful with that gun son, it's loaded!
	  */
	HFONT getHandle() const
	{
		return itsHandle;
	}

private:
	Font( const Font & ); // Never implemented
	Font & operator =( const Font & ); // Never implemented

	void checkCreationOfFont()
	{
		if ( 0 == itsHandle )
			throw xCeption( _T( "Couldn't create font" ) );
	}

	// The actual Windows Handle
	HFONT itsHandle;

	// false if class does not have ownership of font and should not destroy the HFONT
	bool isOwner;
};

/// \ingroup GlobalStuff
/// Font OBJECT type.
/** Use this typedefed pointer to ensure compatibility in future versions of SmartWin++
  */
typedef boost::shared_ptr< Font > FontPtr;

//TODO: shouldn't this be within #ifndef WINCE ... #endif
/// \ingroup GlobalStuff
/// Creates a Font from a StockId value and returns a pointer to it.
/** The returned object is of type boost::shared_ptr< Font >, but you should use the
  * typedef FontPtr and not <br>
  * the shared_ptr itself since this may change in future releases.
  */
FontPtr createFont( PredefinedFontTypes fontType );

/// \ingroup GlobalStuff
/// Creates a Font and returns a pointer to it.
/** The returned object is of type boost::shared_ptr< Font >, but you should use the
  * typedef FontPtr and not <br>
  * the shared_ptr itself since this may change in future releases.
  */
FontPtr createFont( const SmartUtil::tstring & faceName,
 int height = 10,
 int width = 10,
 int weight = 2,
 DWORD charSet = ANSI_CHARSET,
 bool italic = false,
 bool underline = false,
 bool strikeOut = false,
 int escapementOrientation = 0,
 DWORD outputPrecision = OUT_DEFAULT_PRECIS,
 DWORD clipPrecision = CLIP_DEFAULT_PRECIS,
 DWORD quality = DEFAULT_QUALITY,
 DWORD pitchAndFamliy = FF_DONTCARE
 );

// end namespace SmartWin
}

#endif
