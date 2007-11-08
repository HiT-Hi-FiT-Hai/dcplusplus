/*
  Copyright (c) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

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
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ClipBoard_h
#define ClipBoard_h

#include "../SmartUtil.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Class for commonalities between the different specializations of the ClipBoard
/// class
/** Class is not directly instantiable, you must derive from it or use one of the
  * ClipBoard specialized classes.
  */
class ClipBoardBase
{
public:
	/// Types of clipboard format
	/** Basically just maps to the defines from winuser.h which is prefixed with CF_
	  * in front of the name
	  */
	enum ClipBoardFormat
	{ TEXT = CF_TEXT, BITMAP = CF_BITMAP, PALETTE = CF_PALETTE, WAVE = CF_WAVE, UNICODETEXT = CF_UNICODETEXT
	};

	/// Checks to see if the queried clipboard format is available
	/** Returns true if the queried clipboard format is available, otherwise false.
	  */
	bool isClipBoardFormatAvailable( ClipBoardFormat format )
	{
		return ::IsClipboardFormatAvailable( static_cast< unsigned >( format ) ) == TRUE;
	}

protected:
	virtual ~ClipBoardBase()
	{}
	ClipBoardBase()
	{}

private:
	// Private and never implemented since we try to avoid slicing, there is
	// basically no point in having a copy of this object.
	ClipBoardBase( const ClipBoardBase & );
	ClipBoardBase & operator =( const ClipBoardBase & );
};

// The generic version is "uninstantiable".
template< class Type >
class ClipBoard
{
	// Private Constructor, Supposed to be "uninstantiabble".
	ClipBoard();
};

/// Class for manipulating the clipboard, specialized for SmartUtil::tstring
/// clipboard operations.
/** At the moment SmartWin only supports SmartUtil::tstring clipboard operations.
  * <br>
  * More clipboard formats will be supported in later version. <br>
  * Every clipboard class is a Singleton to make access easy. <br>
  * <b>Just remember that if you stuff things into the clipboard then access it again
  * later as the type you stuffed it in as!</b>
  */
template< >
class ClipBoard< SmartUtil::tstring > : public ClipBoardBase
{
public:
	/// Returns the actual instance of the object
	/** Every clipboard specialized class is a singleton, partially too be easily
	  * accessible but also to ensure serial access to the object. <br>
	  * SmartWin uses "instance" all through the library to access the instance
	  * object of singleton classes, use this function to retrieve the instance
	  * object.
	  */
	static ClipBoard & instance()
	{
		static ClipBoard retVal;
		return retVal;
	}

	/// Sets clipboard data to given string
	/** Takes a "parent" window which will "own" the clipboard and a
	  * SmartUtil::tstring which will be stuffed into the clipboard and made <br>
	  * accessible for other applications as CF_TEXT or your own application. <br>
	  * Just remember that if you use this specialized instance of the clipboard
	  * class you must access the clipboard again (through getClipBoardData) <br>
	  * through the same specialization (SmartUtil::tstring specialization).
	  */
	void setClipBoardData( const SmartUtil::tstring & str, const Widget * owner )
	{
		if ( !::OpenClipboard( owner->handle() ) )
			throw SmartWin::xCeption( _T( "Couldn't open the clipboard" ) );
		try
		{
			::EmptyClipboard();
			HGLOBAL handle = ::GlobalAlloc( GMEM_MOVEABLE, sizeof( TCHAR ) * str.size() + 1 );
			if ( 0 == handle )
				throw SmartWin::xCeption( _T( "Couldn't allocate memory to hold clipboard data" ) );
			TCHAR * buffer = reinterpret_cast< TCHAR * >( GlobalLock( handle ) );
			memcpy( buffer, str.c_str(), sizeof( TCHAR ) * str.size() );
			GlobalUnlock( handle );
			SetClipboardData( CF_TEXT, handle );
			CloseClipboard();
		}
		catch ( ... )
		{
			::CloseClipboard();
			throw;
		}
	}

	/// Retrieves clipboard data from the clipboard (assumes clipboard format is string)
	/** When accessing the clipboard we need to "send a rquest" from a specific
	  * window. <br>
	  * The "owner" parameter defines this window. <br>
	  * If you try to access the clipboard you will get an empty string ("") returned
	  * if the clipboard is either empty or if the clipboard data was of the wrong
	  * format.
	  */
	SmartUtil::tstring getClipBoardData( const Widget * owner ) const
	{
		if ( !::IsClipboardFormatAvailable( CF_TEXT ) )
			return _T( "" );
		if ( !::OpenClipboard( owner->handle() ) )
			return _T( "" );
		HANDLE handle = ::GetClipboardData( CF_TEXT );
		if ( 0 == handle )
			return _T( "" );
		SmartUtil::tstring retVal( reinterpret_cast< TCHAR * >( GlobalLock( handle ) ) );
		GlobalUnlock( handle );
		CloseClipboard();
		return retVal;
	}

private:
	// Private and never implemented since we try to avoid slicing, there is
	// basically no point in having a copy of this object.
	ClipBoard( const ClipBoard & );
	ClipBoard & operator =( const ClipBoard & );

	// Private Constructor to ensure Singleton logic
	ClipBoard()
	{}
};

/// \ingroup GlobalStuff
// For easy access of the most common ClipBoard specializations.
typedef ClipBoard< SmartUtil::tstring > ClipBoardString;

// end namespace SmartWin
}

#endif
