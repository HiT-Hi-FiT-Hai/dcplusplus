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
		if(find(listeners.begin(), listeners.end(), aListener) == listeners.end())
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

class StringInfo {
public:
	StringInfo(LPARAM lp = NULL, const string& s = "") : lParam(lp), str(s) { };
	string str;
	LPARAM lParam;
};

class StringListInfo {
public:
	StringListInfo(LPARAM lp = NULL) : lParam(lp) { };
	StringList l;
	LPARAM lParam;
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
	static string formatBytes(const string& aString) {
		return formatBytes(toInt64(aString));
	}

	static string formatBytes(LONGLONG aBytes) {
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
	
	static string formatSeconds(int aSec) {
		char buf[64];
		sprintf(buf, "%01d:%02d:%02d", aSec / (60*60), (aSec / 60) % 60, aSec % 60);
		return buf;
	}

	static LONGLONG toInt64(const string& aString) {
		return _atoi64(aString.c_str());
	}

	static int toInt(const string& aString) {
		return atoi(aString.c_str());
	}

	static string toString(LONGLONG val) {
		char buf[32];
		return _i64toa(val, buf, 10);
	}

	static string toString(int val) {
		char buf[16];
		return itoa(val, buf, 10);
	}
	static string getLocalIp() {
		char buf[256];
		gethostname(buf, 256);
		hostent* he = gethostbyname(buf);
		sockaddr_in dest;
        memcpy(&(dest.sin_addr), he->h_addr_list[0], he->h_length);
		return inet_ntoa(dest.sin_addr);
	}
	/**
	 * Case insensitive substring search.
	 * @return First position found or string::npos
	 */
	static string::size_type findSubString(const string& aString, const string& aSubString) {
		
		string::size_type alen = aString.size();
		string::size_type blen = aSubString.size();
		
		if(alen >= blen) {
			const char* a = aString.c_str();
			const char* b = aSubString.c_str();
			
			for(string::size_type pos = 0; pos < alen - blen + 1; pos++) {
				if(strnicmp(a+pos, b, blen) == 0)
					return pos;
			}
		}
		return string::npos;
	}
};

#endif // !defined(AFX_UTIL_H__1758F242_8D16_4C50_B40D_E59B3DD63913__INCLUDED_)

/**
 * @file Util.h
 * $Id: Util.h,v 1.10 2002/01/05 19:06:09 arnetheduck Exp $
 * @if LOG
 * $Log: Util.h,v $
 * Revision 1.10  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.8  2002/01/02 16:12:33  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.7  2001/12/30 15:03:45  arnetheduck
 * Added framework to handle incoming searches
 *
 * Revision 1.6  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
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

