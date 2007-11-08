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
#ifndef WidgetSaveFile_h
#define WidgetSaveFile_h

#include "../WindowsHeaders.h"
#include "../../SmartUtil.h"
#include "../FreeCommonDialog.h"
#include "../aspects/AspectFileFilter.h"

namespace SmartWin
{
// begin namespace SmartWin

/// SaveFileDialog class
/** \ingroup WidgetControls
  * \image html savefile.PNG
  * Class for showing a Save File Dialog. <br>
  * Either derive from it or call WidgetFactory::createSaveFile. <br>
  * Related classes 
  * <ul>
  * <li>WidgetLoadFile</li>
  * <li>AspectFileFilter</li>
  * <li>WidgetFileCommon</li>
  * </ul>
  * Note!<br>
  * If you wish to use this class with Parent classes other than those from 
  * SmartWin++ you need to expose a public function called "parent" taking no 
  * arguments returning an HWND. <br>
  * the complete signature of the function will then be "HWND parent();"   
  */
template< class Parent >
class WidgetSaveFile
	: public virtual AspectFileFilter,
	public WidgetFileCommon
{
public:
	/// Class type
	typedef WidgetSaveFile< Parent > ThisType;

	/// Object type
	/** Note, not a pointer!!!!
	  */
	typedef ThisType ObjectType;

	/// Shows the dialog
	/** Returns string() or "empty string" if user press cancel. <br>
	  * Returns a "file path" if user presses ok.
	  */
	SmartUtil::tstring showDialog();

	/// Constructor Taking pointer to parent
	explicit WidgetSaveFile( Parent * parent = 0 );

	virtual ~WidgetSaveFile()
	{}

private:
	Parent * itsParent;
};

/// \ingroup GlobalStuff
/// A Free WidgetSaveFile dialog is a dialog which isn't "owned" by another Widget
typedef WidgetSaveFile< FreeCommonDialog > WidgetSaveFileFree;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class Parent >
SmartUtil::tstring WidgetSaveFile< Parent >::showDialog()
{
	TCHAR szFile[MAX_PATH + 1]; // buffer for file name
	szFile[0] = '\0';

	OPENFILENAME ofn; // common dialog box structure
	// Note!
	// If this one fizzles you have NOT supplied a parent with a "handle()"
	// function... You MUST supply a parent with a function "handle()" which
	// returns a HWND! All the Widgetxxx classes (except LoadFile, SaveFile and
	// MessageBox) have the "handle()" function...
	fillOutCommonStructure( ofn, itsParent->handle(), 0 ); // OFN_PATHMUSTEXIST ?
	ofn.lpstrFile = szFile;

	SmartUtil::tstring retVal;
	if ( ::GetSaveFileName( & ofn ) )
	{
		retVal = ofn.lpstrFile;
		backslashToForwardSlashForUnix( retVal );
	}
	return retVal;
}

template< class Parent >
WidgetSaveFile< Parent >::WidgetSaveFile( Parent * parent )
	: itsParent( parent )
{
}

// end namespace SmartWin
}

#endif
