/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors 
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

#include <dwt/widgets/LoadDialog.h>

namespace SmartWin {

bool LoadDialog::open(SmartUtil::tstring& file)
{
	OPENFILENAME ofn = { sizeof(OPENFILENAME) }; // common dialog box structure
	fillOFN( ofn, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY );

	if ( ::GetOpenFileName( & ofn ) ) {
		file = ofn.lpstrFile;
		return true;
	}
	return false;
}

bool LoadDialog::open(std::vector<SmartUtil::tstring>& files) 
{ 
	OPENFILENAME ofn = { sizeof(OPENFILENAME) }; // common dialog box structure
	fillOFN( ofn, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT ); 
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
			files.push_back(directory); // string 'directory' contains full path 
		else 
		{ 
			while (fileName.length() > 0) 
			{ 
				filePath = directory + _T("\\") + fileName; 
				files.push_back(filePath); 
				array_p = array_p + fileName.length() + 1; // set pointer one position past null 
				fileName = array_p; // fileName is substring from array_p to next null  
			} 
		}  
		return true;
	} 
	return false; 
} 

}
