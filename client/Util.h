/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#if !defined(AFX_UTIL_H__1758F242_8D16_4C50_B40D_E59B3DD63913__INCLUDED_)
#define AFX_UTIL_H__1758F242_8D16_4C50_B40D_E59B3DD63913__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CriticalSection.h"

template<typename Listener>
class Speaker {
public:
	void addListener(Listener* aListener) {
		listenerCS.enter();
		listeners.push_back(aListener);
		listenerCS.leave();
	}
	
	void removeListener(Listener* aListener) {
		listenerCS.enter();
		for(vector<Listener*>::iterator i = listeners.begin(); i != listeners.end(); ++i) {
			if(*i == aListener) {
				listeners.erase(i);
				break;
			}
		}
		listenerCS.leave();
	}
	
	void removeListeners() {
		listenerCS.enter();
		listeners.clear();
		listenerCS.leave();
	}
protected:
	vector<Listener*> listeners;
	CriticalSection listenerCS;
};


class Util  
{
public:
	static bool browseSaveFile(string& target, HWND owner = NULL) {
		char buf[MAX_PATH];
		OPENFILENAME ofn;       // common dialog box structure
		
		memcpy(buf, target.c_str(), target.length() + 1);
		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = owner;
		ofn.lpstrFile = buf;
		ofn.nMaxFile = sizeof(buf);
		ofn.Flags = OFN_PATHMUSTEXIST;
		
		// Display the Open dialog box. 
		if (GetSaveFileName(&ofn)==TRUE) {
			target = ofn.lpstrFile;
			return true;
		}
		return false;
	}

	static bool browseDirectory(string& target, HWND owner = NULL) {
		char buf[MAX_PATH];
		BROWSEINFO bi;
		LPMALLOC ma;
		
		ZeroMemory(&bi, sizeof(bi));
		
		bi.hwndOwner = owner;
		bi.pszDisplayName = buf;
		bi.lpszTitle = "Choose folder";
		bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
		if(pidl != NULL) {
			SHGetPathFromIDList(pidl, buf);
			target = buf;
			target+='\\';

			if(SHGetMalloc(&ma) != E_FAIL) {
				ma->Free(pidl);
				ma->Release();
			}
			return true;
		}
		return false;
	}
			
	static void decodeUrl(const string& aUrl, string& aServer, short& aPort, string& aFile);
	static void ensureDirectory(const string& aFile)
	{
		string::size_type start = 0;
		
		while( (start = aFile.find('\\', start)) != string::npos) {
			CreateDirectory(aFile.substr(0, start+1).c_str(), NULL);
			start++;
		}
	}
	
	static LONGLONG getFileSize(const string& aName) {
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		
		hFind = FindFirstFile(aName.c_str(), &fd);
		
		if (hFind == INVALID_HANDLE_VALUE) {
			return -1;
		} else {
			FindClose(hFind);
			return ((ULONGLONG)fd.nFileSizeHigh << 32 | (ULONGLONG)fd.nFileSizeLow);
		}
	}
	static string shortenBytes(const string& aString) {
		return shortenBytes(_atoi64(aString.c_str()));
	}

	static string shortenBytes(LONGLONG aBytes) {
		char buf[64];
		if(aBytes < 1024) {
			sprintf(buf, "%I64d B", aBytes );
		} else if(aBytes < 1024*1024) {
			sprintf(buf, "%.02f kB", (double)aBytes/(1024.0) );
		} else if(aBytes < 1024*1024*1024) {
			sprintf(buf, "%.02f MB", (double)aBytes/(1024.0*1024.0) );
		} else if(aBytes < 1024I64*1024I64*1024I64*1024I64) {
			sprintf(buf, "%.02f GB", (double)aBytes/(1024.0*1024.0*1024.0) );
		} else {
			sprintf(buf, "%.02f TB", (double)aBytes/(1024.0*1024.0*1024.0*1024.0));
		}
		
		return buf;
	}
	
};

#endif // !defined(AFX_UTIL_H__1758F242_8D16_4C50_B40D_E59B3DD63913__INCLUDED_)

/**
 * @file Util.h
 * $Id: Util.h,v 1.5 2001/12/19 23:07:59 arnetheduck Exp $
 * @if LOG
 * $Log: Util.h,v $
 * Revision 1.5  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.4  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.3  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.2  2001/12/07 20:03:33  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.1  2001/12/02 11:18:10  arnetheduck
 * Added transfer totals and speed...
 *
 * @endif
 */

