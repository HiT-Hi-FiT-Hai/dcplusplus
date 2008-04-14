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

#include <dwt/widgets/FolderDialog.h>

namespace dwt {

FolderDialog& FolderDialog::setRoot( const int csidl ) {
	if (FAILED(SHGetSpecialFolderLocation( getParentHandle(), csidl, &itsPidlRoot ))) {
		itsPidlRoot = NULL;
	}
	return *this;
}

bool FolderDialog::open(tstring& folder) {
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
