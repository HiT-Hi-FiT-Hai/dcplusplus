#include "../../include/smartwin/widgets/LoadDialog.h"

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
