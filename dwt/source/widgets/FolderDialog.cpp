#include "../../include/smartwin/widgets/FolderDialog.h"

namespace SmartWin {

FolderDialog& FolderDialog::setRoot( const int csidl ) {
	if (FAILED(SHGetSpecialFolderLocation( getParentHandle(), csidl, &itsPidlRoot ))) {
		itsPidlRoot = NULL;
	}
	return *this;
}

bool FolderDialog::open(SmartUtil::tstring& folder) {
	BROWSEINFO bws = { 0 };
	bws.hwndOwner = getParentHandle();
	bws.pidlRoot = itsPidlRoot;
	if(!itsTitle.empty()) {
		bws.lpszTitle = itsTitle.c_str();
	}
	bws.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS | BIF_EDITBOX;
	if(!folder.empty()) {
		bws.lParam = reinterpret_cast<LPARAM>(folder.c_str());
		bws.lpfn = &browseCallbackProc;
	}

	// Avoid errors about missing cdroms, floppies etc..
	UINT oldErrorMode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
	
	LPITEMIDLIST lpIDL = SHBrowseForFolder( & bws );
	
	::SetErrorMode(oldErrorMode);
	
	if(lpIDL) {
		TCHAR buf[MAX_PATH + 1];
		if ( ::SHGetPathFromIDList( lpIDL, buf ) ) {
			folder = buf;
			
			if(folder.size() > 0 && folder[folder.size()-1] != _T('\\')) {
				folder += _T('\\');
			}
			
			::CoTaskMemFree(lpIDL);
			return true;
		}
		::CoTaskMemFree(lpIDL);
	}
	return false;
}

}
