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

#ifndef DWT_WidgetChooseFolder_h
#define DWT_WidgetChooseFolder_h

#include "../Widget.h"
#include "../tstring.h"
#include <shlobj.h>

namespace dwt {

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
class FolderDialog
{
public:
	/// Class type
	typedef FolderDialog ThisType;

	/// Object type
	/** Note, not a pointer!!!!
	  */
	typedef ThisType ObjectType;

	// Constructor Taking pointer to parent
	explicit FolderDialog( Widget * parent = 0 );

	/// Shows the dialog
	/** Returns string() or "empty string" if user press cancel. <br>
	  * Returns a "folder path" if user presses ok. <br>
	  * Use the inherited functions AspectfolderFilter::addFilter and
	  * AspectfolderFilter::activeFilter <br>
	  * before calling this function, if you wish the dialog to show only certain
	  * types of folders.
	  */
	bool open(tstring& folder);

	/// Sets the root directory in the WidgetChooseFolder Widget
	/** If given your dialog will try to start with the given directory as root, otherwise it
	  * will use the desktop directory.
	  */ 
	FolderDialog& setRoot( const int CSIDL = CSIDL_DESKTOPDIRECTORY );

	FolderDialog& setTitle( const tstring& title );
	
	~FolderDialog();

private:
	static int CALLBACK browseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
	{
		if(lpData && uMsg == BFFM_INITIALIZED) {
			::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		}
		return 0;
	}

	Widget* itsParent;
	tstring itsTitle;
	LPITEMIDLIST itsPidlRoot;
	
	HWND getParentHandle() { return itsParent ? itsParent->handle() : NULL; }

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline FolderDialog::FolderDialog( Widget * parent )
 : itsParent( parent ), itsPidlRoot(NULL)
{
}

inline FolderDialog& FolderDialog::setTitle( const tstring& title ) {
	itsTitle = title;
	return *this;
}

inline FolderDialog::~FolderDialog() {
	::CoTaskMemFree(itsPidlRoot);
}

}

#endif
