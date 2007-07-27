// $Revision: 1.9 $
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
#ifndef WidgetChooseFolder_h
#define WidgetChooseFolder_h

#include "../../SmartUtil.h"
#include "../FreeCommonDialog.h"
#include "../WindowsHeaders.h"
#include <shlobj.h>

namespace SmartWin
{
// begin namespace SmartWin

/// ChooseFolderDialog class
/** \ingroup WidgetControls
  * \image html ChooseFolder.PNG
  * Class for showing a ChooseFolderDialog box. <br>
  * Either derive from it or call WidgetFactory::createChooseFolder. <br>
  * Note! <br>
  * If you wish to use this class with Parent classes other than those from SmartWin  
  * you need to expose a public function called "parent" taking no arguments returning 
  * and HWND in the Parent template parameter. <br>
  * the complete signature of the function will then be "HWND parent()"    
  */ 
template< class Parent >
class WidgetChooseFolder
{
public:
	/// Class type
	typedef WidgetChooseFolder< Parent > ThisType;

	/// Object type
	/** Note, not a pointer!!!!
	  */
	typedef ThisType ObjectType;

	/// Shows the dialog
	/** Returns string() or "empty string" if user press cancel. <br>
	  * Returns a "folder path" if user presses ok. <br>
	  * Use the inherited functions AspectfolderFilter::addFilter and
	  * AspectfolderFilter::activeFilter <br>
	  * before calling this function, if you wish the dialog to show only certain
	  * types of folders.
	  */
	SmartUtil::tstring showDialog();

	// Constructor Taking pointer to parent
	explicit WidgetChooseFolder( Parent * parent = 0 );

	/// Sets the root directory in the WidgetChooseFolder Widget
	/** If given your dialog will try to start with the given directory as root, otherwise it
	  * will use the desktop directory.
	  */ 
	void setRootDirectory( const int CSIDL = CSIDL_DESKTOPDIRECTORY )
	{
		bool ok = false;
		///WARNING: WINDOWSNT SHOULD NOT SUPPORT THIS FUNCTION
		if ( itsParent != NULL )
			ok = SUCCEEDED( SHGetSpecialFolderLocation( itsParent->handle(), CSIDL, & itsPidlRoot ) );
		else
			ok = SUCCEEDED( SHGetSpecialFolderLocation( NULL, CSIDL, & itsPidlRoot ) );
		if ( !ok )
			itsPidlRoot = NULL;
	}

	void setTitleText( SmartUtil::tstring TitleText )
	{
		itsTitleText = TitleText;
	}

	/// Sets the starting directory selected in the WidgetChooseFolder widget
	/** If given your dialog will try to start with the given directory selected.
	  */
	void setStartDirectory( SmartUtil::tstring startDir )
	{
		itsStartDir = startDir;
	}

	virtual ~WidgetChooseFolder()
	{
	}

private:
	static int CALLBACK browseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
	{
		switch( uMsg ) 
		{
		case BFFM_INITIALIZED: 
			ThisType* p = (ThisType*) lpData;
			if( p->itsStartDir.size() > 0 )
				SendMessage( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM) p->itsStartDir.c_str() );
			break;
		}
		return 0;
	}

	Parent * itsParent;
	SmartUtil::tstring itsTitleText;
	LPITEMIDLIST itsPidlRoot;
	SmartUtil::tstring itsStartDir;
};

typedef WidgetChooseFolder< FreeCommonDialog > WidgetChooseFolderFree;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class Parent >
SmartUtil::tstring WidgetChooseFolder< Parent >::showDialog()
{
	// WARNING!
	// If you ONLY include THIS file then you must explicitly link to comctl32.lib
	// since you won't get the default linking inclusion by the whole library!!
	CoInitialize( 0 );
	SmartUtil::tstring retVal = _T( "" );
	BROWSEINFO bws;
	ZeroMemory( & bws, sizeof( bws ) );
	bws.hwndOwner = itsParent->handle();
	bws.pidlRoot = itsPidlRoot;
	bws.pszDisplayName = NULL;
	bws.lpszTitle = itsTitleText.c_str();
#ifdef __WINE__
	bws.ulFlags = 0; // Wine 0.9.10 does not support the BIF_USENEWUI flag yet.
#else
	bws.ulFlags = BIF_USENEWUI;
#endif
	bws.lpfn = NULL;
	bws.lParam = (LPARAM) this;
	bws.lpfn = &browseCallbackProc;

	LPITEMIDLIST lpIDL = NULL;
	lpIDL = SHBrowseForFolder( & bws );
	if ( lpIDL )
	{
		TCHAR temp_path[MAX_PATH + 1];
		temp_path[0] = _T( '\0' );
		if ( !SHGetPathFromIDList( lpIDL, & temp_path[0] ) )
			temp_path[0] = _T( '\0' );
		else
			retVal = temp_path;

		LPMALLOC mal = NULL;
		if ( SHGetMalloc( & mal ) == E_FAIL || !mal )
			free( lpIDL );
		else
		{
			mal->Free( lpIDL );
			mal->Release();
		}
	}
	return retVal;
}

template< class Parent >
WidgetChooseFolder< Parent >::WidgetChooseFolder( Parent * parent )
 : itsParent( parent )
{
	itsTitleText = _T( "" );
	itsPidlRoot = NULL;
}

// end namespace SmartWin
}

#endif
