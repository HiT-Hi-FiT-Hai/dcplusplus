/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

class Flags {
	public:
		Flags() : flags(0) { };
		bool isSet(int aFlag) { return (flags & aFlag) > 0; };
		void setFlag(int aFlag) { flags |= aFlag; };
		void unsetFlag(int aFlag) { flags &= ~aFlag; };

	private:
		int flags;
};

template<typename Listener>
class Speaker {
public:
	void fire(Listener::Types type) throw () {
		listenerCS.enter();
		vector<Listener*> tmp = listeners;
		for(vector<Listener*>::iterator i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onAction(type);
		}
		listenerCS.leave();
	};
	
	template<class T> 
		void fire(Listener::Types type, const T& param) throw () {
		listenerCS.enter();
		vector<Listener*> tmp = listeners;
		for(vector<Listener*>::iterator i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onAction(type, param);
		}
		listenerCS.leave();
	};
	
	template<class T, class T2> 
		void fire(Listener::Types type, const T& p, const T2& p2) throw() {
		listenerCS.enter();
		vector<Listener*> tmp = listeners;
		for(vector<Listener*>::iterator i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onAction(type, p, p2);
		}
		listenerCS.leave();
	};
	template<class T, class T2, class T3> 
		void fire(Listener::Types type, const T& p, const T2& p2, const T3& p3) throw() {
		listenerCS.enter();
		vector<Listener*> tmp = listeners;
		for(vector<Listener*>::iterator i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onAction(type, p, p2, p3);
		}
		listenerCS.leave();
	};
	template<class T, class T2, class T3, class T4, class T5, class T6> 
		void fire(Listener::Types type, const T& p, const T2& p2, const T3& p3, const T4& p4, const T5& p5, const T6& p6) throw() {
		listenerCS.enter();
		vector<Listener*> tmp = listeners;
		for(vector<Listener*>::iterator i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onAction(type, p, p2, p3, p4, p5, p6);
		}
		listenerCS.leave();
	};
	
	
	void addListener(Listener* aListener) {
		Lock l(listenerCS);
		if(find(listeners.begin(), listeners.end(), aListener) == listeners.end())
			listeners.push_back(aListener);
	}
	
	void removeListener(Listener* aListener) {
		Lock l(listenerCS);

		vector<Listener*>::iterator i = find(listeners.begin(), listeners.end(), aListener);
		if(i != listeners.end())
			listeners.erase(i);
	}
	
	void removeListeners() {
		Lock l(listenerCS);
		listeners.clear();
	}
protected:
	vector<Listener*> listeners;
	CriticalSection listenerCS;
};

template<typename T>
class Singleton {
public:
	static T* getInstance() {
		dcassert(instance);
		return instance;
	}
	
	static void newInstance() {
		if(instance)
			delete instance;
		
		instance = new T();
	}
	
	static void deleteInstance() {
		if(instance)
			delete instance;
		instance = NULL;
	}
protected:
	static T* instance;
};

class Util  
{
public:
	static string emptyString;
	static HBRUSH bgBrush;
	static COLORREF textColor;
	static COLORREF bgColor;
	static HFONT font;
	
	static void decodeFont(const string& setting, LOGFONT &dest);

	static string encodeFont(LOGFONT const& font)
	{
		string res(font.lfFaceName);
		res += ',';
		res += Util::toString(font.lfHeight);
		res += ',';
		res += Util::toString(font.lfWeight);
		res += ',';
		res += Util::toString(font.lfItalic);
		return res;
	}
	
	
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
		bi.lParam = (LPARAM)target.c_str();
		bi.lpfn = &browseCallbackProc;
		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
		if(pidl != NULL) {
			SHGetPathFromIDList(pidl, buf);
			target = buf;

			if(target.size() > 0 && target[target.size()-1] != '\\')
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
	
	static string getAppPath() {
		TCHAR buf[MAX_PATH+1];
		GetModuleFileName(NULL, buf, MAX_PATH);
		int i = (strrchr(buf, '\\') - buf);
		return string(buf, i + 1);
		
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

	static string getTimeString() {
		char buf[64];
		time_t _tt;
		time(&_tt);
		tm* _tm = localtime(&_tt);
		strftime(buf, 64, "%X", _tm);
		return buf;
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

	static string toLower(const string& aString) {
		string tmp = aString;
		for(string::size_type i = 0; i < tmp.size(); i++) {
			tmp[i] = tolower(tmp[i]);
		}
		return tmp;
	}

	static LONGLONG toInt64(const string& aString) {
		return _atoi64(aString.c_str());
	}

	static int toInt(const string& aString) {
		return atoi(aString.c_str());
	}

	static double toDouble(const string& aString) {
		return atof(aString.c_str());
	}

	static float toFloat(const string& aString) {
		return (float)atof(aString.c_str());
	}

	static string toString(LONGLONG val) {
		char buf[32];
		return _i64toa(val, buf, 10);
	}

	static string toString(int val) {
		char buf[16];
		return itoa(val, buf, 10);
	}

	static string translateError(int aError) {
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			aError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);
		string tmp = (LPCTSTR)lpMsgBuf;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		return tmp;
	}

	static string getLocalIp() {
		string tmp;

		char buf[256];
		gethostname(buf, 256);
		hostent* he = gethostbyname(buf);
		if(he == NULL || he->h_addr_list[0] == 0)
			return "";
		sockaddr_in dest;
        int i = 0;

		// We take the first ip as default, but if we can find a better one, use it instead...
		memcpy(&(dest.sin_addr), he->h_addr_list[i++], he->h_length);
		tmp = inet_ntoa(dest.sin_addr);
		if( strncmp(tmp.c_str(), "192", 3) == 0 || 
			strncmp(tmp.c_str(), "169", 3) == 0 || 
			strncmp(tmp.c_str(), "127", 3) == 0 || 
			strncmp(tmp.c_str(), "10", 2) == 0 ) {

			while(he->h_addr_list[i]) {
				memcpy(&(dest.sin_addr), he->h_addr_list[i], he->h_length);
				string tmp2 = inet_ntoa(dest.sin_addr);
				if(	strncmp(tmp2.c_str(), "192", 3) != 0 &&
					strncmp(tmp2.c_str(), "169", 3) != 0 &&
					strncmp(tmp2.c_str(), "127", 3) != 0 &&
					strncmp(tmp2.c_str(), "10", 2) != 0) {
							
					tmp = tmp2;
				}
				i++;
			}
		}
		return tmp;
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

	static string validateString(const string& aNick) {	
		string nick = aNick;
		int i;
		while((i=nick.find('$')) != string::npos)
			nick.replace(i, 1, 1, '_');
		while((i=nick.find(' ')) != string::npos)
			nick.replace(i, 1, 1, '_');
		while((i=nick.find('|')) != string::npos)
			nick.replace(i, 1, 1, '_');
		return nick;
	}

private:
	static int CALLBACK browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData) {
		switch(uMsg) {
		case BFFM_INITIALIZED: 
			// WParam is TRUE since you are passing a path.
			// It would be FALSE if you were passing a pidl.
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
			break;
			
/*		case BFFM_SELCHANGED: 
			// Set the status window to the currently selected path.
			if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
			{
				SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
			}
			break;*/
		}
		return 0;
	}
		
		
};

#endif // !defined(AFX_UTIL_H__1758F242_8D16_4C50_B40D_E59B3DD63913__INCLUDED_)

/**
 * @file Util.h
 * $Id: Util.h,v 1.24 2002/01/26 21:09:51 arnetheduck Exp $
 * @if LOG
 * $Log: Util.h,v $
 * Revision 1.24  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.23  2002/01/26 12:06:40  arnetheduck
 * Småsaker
 *
 * Revision 1.22  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
 * Revision 1.21  2002/01/22 00:10:38  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
 *
 * Revision 1.20  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.19  2002/01/19 19:07:39  arnetheduck
 * Last fixes before 0.13
 *
 * Revision 1.18  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.17  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.16  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.15  2002/01/11 16:13:33  arnetheduck
 * Fixed some locks and bugs, added type field to the search frame
 *
 * Revision 1.14  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.13  2002/01/09 19:01:35  arnetheduck
 * Made some small changed to the key generation and search frame...
 *
 * Revision 1.12  2002/01/07 23:05:48  arnetheduck
 * Resume rollback implemented
 *
 * Revision 1.11  2002/01/06 11:13:07  arnetheduck
 * Last fixes before 0.10
 *
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

