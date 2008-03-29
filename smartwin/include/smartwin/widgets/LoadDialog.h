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
#ifndef LoadDialog_h
#define LoadDialog_h

#include "../Widget.h"
#include "../../SmartUtil.h"
#include "../aspects/AspectFileFilter.h"
#include <vector>

namespace SmartWin
{
// begin namespace SmartWin

/// LoadFileDialog class
/** \ingroup WidgetControls
  * \image html loadfile.PNG
  * Class for showing a LoadFileDialog box. <br>
  * \sa SaveDialog
  * \sa AspectFileFilter
  */
class LoadDialog
	: public AspectFileFilter
{
public:
	/// Class type
	typedef LoadDialog ThisType;

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
	explicit LoadDialog( Widget * parent = 0 );

	~LoadDialog() { }
private:
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline SmartUtil::tstring LoadDialog::showDialog()
{
	TCHAR szFile[PATH_BUFFER_SIZE]; // buffer for file name
	szFile[0] = '\0';

	OPENFILENAME ofn = { sizeof(OPENFILENAME) }; // common dialog box structure
	fillOFN( ofn, getParentHandle(), OFN_FILEMUSTEXIST );
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

inline std::vector<SmartUtil::tstring> LoadDialog::showDialogMultiSelect() 
{ 
	TCHAR szFile[PATH_BUFFER_SIZE]; // buffer for file name 
	szFile[0] = '\0'; 

	OPENFILENAME ofn = { sizeof(OPENFILENAME) }; // common dialog box structure
	fillOFN( ofn, getParentHandle(), OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER ); 
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

inline LoadDialog::LoadDialog( Widget * parent )
	: AspectFileFilter( parent )
{}

// end namespace SmartWin
}

#endif
