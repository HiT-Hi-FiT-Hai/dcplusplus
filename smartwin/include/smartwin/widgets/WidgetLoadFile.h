// $Revision: 1.15 $
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
#ifndef WidgetLoadFile_h
#define WidgetLoadFile_h

#include "../../SmartUtil.h"
#include "../FreeCommonDialog.h"
#include "../WindowsHeaders.h"
#include "../aspects/AspectFileFilter.h"
#include <vector>

namespace SmartWin
{
// begin namespace SmartWin

/// LoadFileDialog class
/** \ingroup WidgetControls
  * \image html loadfile.PNG
  * Class for showing a LoadFileDialog box. <br>
  * Either derive from it or call WidgetFactory::createLoadFile. <br>
  * Related classes 
  * <ul>
  * <li>WidgetSaveFile</li>
  * <li>AspectFileFilter</li>
  * <li>WidgetFileCommon</li>
  * </ul>
  * Note! <br>
  * If you wish to use this class with Parent classes other than those from SmartWin 
  * you need to expose a public function called "parent" taking no arguments returning 
  * and HWND in the Parent template parameter. <br>
  * the complete signature of the function will then be "HWND parent()"   
  */
template< class Parent >
class WidgetLoadFile
	: public virtual AspectFileFilter,
	public WidgetFileCommon
{
public:
	/// Class type
	typedef WidgetLoadFile< Parent > ThisType;

	/// Object type
	/** Note, not a pointer!!!!
	  */
	typedef ThisType ObjectType;

	/// Shows the dialog
	/** Returns string() or "empty string" if user press cancel. <br>
	  * Returns a "file path" if user presses ok. <br>
	  * Use the inherited functions AspectFileFilter::addFilter and
	  * AspectFileFilter::activeFilter <br>
	  * before calling this function, if you wish the dialog to show only certain
	  * types of files.
	  */
	SmartUtil::tstring showDialog();

	/// Shows the dialog
	/** Returns an empty vector if user press cancel. <br>
	  * Returns a vector of "file paths" if user presses ok. <br>
	  * Use the inherited functions AspectFileFilter::addFilter and
	  * AspectFileFilter::activeFilter <br>
	  * before calling this function, if you wish the dialog to show only certain
	  * types of files.
	  */
	std::vector<SmartUtil::tstring> showDialogMultiSelect();

	// Constructor Taking pointer to parent
	explicit WidgetLoadFile( Parent * parent = 0 );

	virtual ~WidgetLoadFile()
	{}
private:
	Parent * itsParent;
};

/// \ingroup GlobalStuff
/// A Free WidgetLoadFile dialog is a dialog which isn't "owned" by another Widget
typedef WidgetLoadFile< FreeCommonDialog > WidgetLoadFileFree;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class Parent >
SmartUtil::tstring WidgetLoadFile< Parent >::showDialog()
{
	TCHAR szFile[PATH_BUFFER_SIZE]; // buffer for file name
	szFile[0] = '\0';

	OPENFILENAME ofn; // common dialog box structure
	// Note!
	// If this one fizzles you have NOT supplied a parent with a "handle()"
	// function... You MUST supply a parent with a function "handle()" which
	// returns a HWND! All the Widgetxxx classes (except LoadFile, SaveFile and
	// MessageBox) have the "handle()" function...
	fillOutCommonStructure( ofn, itsParent->handle(), OFN_FILEMUSTEXIST );
	ofn.lpstrFile = szFile;
	ofn.Flags |= OFN_FILEMUSTEXIST;

	SmartUtil::tstring retVal;
	if ( ::GetOpenFileName( & ofn ) )
	{
		retVal = ofn.lpstrFile;
		backslashToForwardSlashForUnix( retVal );
	}
	return retVal;
}

template< class Parent > 
std::vector<SmartUtil::tstring> WidgetLoadFile<Parent>::showDialogMultiSelect() 
{ 
	TCHAR szFile[PATH_BUFFER_SIZE]; // buffer for file name 
	szFile[0] = '\0'; 

	OPENFILENAME ofn; // common dialog box structure 
	// Note! 
	// If this one fizzles you have NOT supplied a parent with a "handle()" function... 
	// You MUST supply a parent with a function "handle()" which returns a HWND! 
	// All the Widgetxxx classes (except LoadFile, SaveFile and MessageBox) have the "handle()" function... 
	fillOutCommonStructure( ofn, itsParent->handle(), OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER ); 
	ofn.lpstrFile = szFile; 
	std::vector<SmartUtil::tstring> retVal; 
	if( ::GetOpenFileName(&ofn) ) 
	{  
		// If a single file is selected, the lpstrFile string is just the path terminated by TWO null bytes 
		// If multiple files are selected, the format of the string returned is: 
		// DIRECTORY_PATH + '\0' + FILE_NAME_1 + '\0' + FILE_NAME_2 + '\0' + ... + FILE_NAME_N + '\0' + '\0' 
		// (Note the last file name is terminated by two null bytes) 

		SmartUtil::tstring fileName; 
		SmartUtil::tstring filePath; 
		SmartUtil::tstring directory; 
		directory = ofn.lpstrFile; // tstring ends at first null 
		TCHAR *array_p = ofn.lpstrFile + directory.length() + 1; // set pointer to one position past null 
		fileName = array_p; // fileName is substring from array_p to next null 
		if (fileName.length() == 0) // only one file was selected 
			retVal.push_back(directory); // string 'directory' contains full path 
		else 
		{ 
			while (fileName.length() > 0) 
			{ 
				filePath = directory + _T("\\") + fileName; 
				backslashToForwardSlashForUnix(filePath);  
				retVal.push_back(filePath); 
				array_p = array_p + fileName.length() + 1; // set pointer one position past null 
				fileName = array_p; // fileName is substring from array_p to next null  
			} 
		}  
	} 
	return retVal; 
} 

template< class Parent >
WidgetLoadFile< Parent >::WidgetLoadFile( Parent * parent )
	: itsParent( parent )
{}

// end namespace SmartWin
}

#endif
