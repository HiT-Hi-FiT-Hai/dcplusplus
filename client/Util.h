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
		bool isSet(int aFlag) const { return (flags & aFlag) > 0; };
		void setFlag(int aFlag) { flags |= aFlag; };
		void unsetFlag(int aFlag) { flags &= ~aFlag; };

	private:
		int flags;
};

template<typename Listener>
class Speaker {
public:

	void fire(typename Listener::Types type) throw() {
		Lock l(listenerCS);
		vector<Listener*> tmp = listeners;
		for(vector<Listener*>::iterator i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onAction(type);
		}
	};
	
	template<class T> 
		void fire(typename Listener::Types type, const T& param) throw () {
		Lock l(listenerCS);
		vector<Listener*> tmp = listeners;
		for(vector<Listener*>::iterator i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onAction(type, param);
		}
	};
	
	template<class T, class T2> 
		void fire(typename Listener::Types type, const T& p, const T2& p2) throw() {
		Lock l(listenerCS);
		vector<Listener*> tmp = listeners;
		for(vector<Listener*>::iterator i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onAction(type, p, p2);
		}
	};
	template<class T, class T2, class T3> 
		void fire(typename Listener::Types type, const T& p, const T2& p2, const T3& p3) throw() {
		Lock l(listenerCS);
		vector<Listener*> tmp = listeners;
		for(vector<Listener*>::iterator i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onAction(type, p, p2, p3);
		}
	};
	template<class T, class T2, class T3, class T4, class T5, class T6> 
		void fire(typename Listener::Types type, const T& p, const T2& p2, const T3& p3, const T4& p4, const T5& p5, const T6& p6) throw() {
		Lock l(listenerCS);
		vector<Listener*> tmp = listeners;
		for(vector<Listener*>::iterator i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onAction(type, p, p2, p3, p4, p5, p6);
		}
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

class Util  
{
public:
	static string emptyString;
			
	static void ensureDirectory(const string& aFile)
	{
#ifdef WIN32
		string::size_type start = 0;
		
		while( (start = aFile.find('\\', start)) != string::npos) {
			CreateDirectory(aFile.substr(0, start+1).c_str(), NULL);
			start++;
		}
#endif
	}
	
	static string getAppPath() {
#ifdef WIN32
		TCHAR buf[MAX_PATH+1];
		GetModuleFileName(NULL, buf, MAX_PATH);
		int i = (strrchr(buf, '\\') - buf);
		return string(buf, i + 1);
#else // WIN32
		return emptyString;
#endif // WIN32
	}	

	static string translateError(int aError) {
#ifdef WIN32
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
		string::size_type i;

		while( (i = tmp.find_last_of("\r\n")) != string::npos) {
			tmp.erase(i, 1);
		}
		return tmp;
#else // WIN32
		return emptyString;
#endif // WIN32
	}
	
	static void decodeUrl(const string& aUrl, string& aServer, short& aPort, string& aFile);

	static string formatBytes(const string& aString) {
		return formatBytes(toInt64(aString));
	}

	static string getShortTimeString() {
		char buf[8];
		time_t _tt;
		time(&_tt);
		tm* _tm = localtime(&_tt);
		strftime(buf, 8, "%H:%M", _tm);
		return buf;
	}

	static string getTimeString() {
		char buf[64];
		time_t _tt;
		time(&_tt);
		tm* _tm = localtime(&_tt);
		strftime(buf, 64, "%X", _tm);
		return buf;
	}
	
	static string formatBytes(int64_t aBytes) {
		char buf[64];
		if(aBytes < 1024) {
			sprintf(buf, "%d %s", (int)(aBytes&0xffffffff), CSTRING(B));
		} else if(aBytes < 1024*1024) {
			sprintf(buf, "%.02f %s", (double)aBytes/(1024.0), CSTRING(KB));
		} else if(aBytes < 1024*1024*1024) {
			sprintf(buf, "%.02f %s", (double)aBytes/(1024.0*1024.0), CSTRING(MB));
		} else if(aBytes < (int64_t)1024*1024*1024*1024) {
			sprintf(buf, "%.02f %s", (double)aBytes/(1024.0*1024.0*1024.0), CSTRING(GB));
		} else {
			sprintf(buf, "%.02f %s", (double)aBytes/(1024.0*1024.0*1024.0*1024.0), CSTRING(TB));
		}
		
		return buf;
	}

	static string formatSeconds(int64_t aSec) {
		char buf[64];
#ifdef WIN32
		sprintf(buf, "%01I64d:%02d:%02d", aSec / (60*60), (int)((aSec / 60) % 60), (int)(aSec % 60));
#else
		sprintf(buf, "%01lld:%02d:%02d", aSec / (60*60), (int)((aSec / 60) % 60), (int)(aSec % 60));
#endif		
		return buf;
	}

	static string formatParams(const string& msg, StringMap& params);

	static string toLower(const string& aString) {
		string tmp = aString;
		for(string::size_type i = 0; i < tmp.size(); i++) {
			tmp[i] = (char)tolower(tmp[i]);
		}
		return tmp;
	}

	static int64_t toInt64(const string& aString) {
#ifdef WIN32
		return _atoi64(aString.c_str());
#else
		return atoll(aString.c_str());
#endif
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

	static string toString(int64_t val) {
		char buf[32];
#ifdef WIN32
		return _i64toa(val, buf, 10);
#else
		sprintf(buf, "%lld", val);
		return buf;
#endif
	}

	static string toString(int val) {
		char buf[16];
		sprintf(buf, "%d", val);
		return buf;
	}

	static string toString(float val) {
		char buf[16];
		sprintf(buf, "%.2f", val);
		return buf;
	}

	static string getLocalIp();
	/**
	 * Case insensitive substring search.
	 * @return First position found or string::npos
	 */
	static string::size_type findSubString(const string& aString, const string& aSubString) {
		
		string::size_type blen = aSubString.size();
		if(blen == 0)
			return 0;
		string::size_type alen = aString.size();
		
		if(alen >= blen) {
			const char* a = aString.c_str();
			const char* b = aSubString.c_str();
			char bl = (char)tolower(b[0]);
			char bu = (char)toupper(b[0]);
			for(string::size_type pos = 0; pos < alen - blen + 1; pos++) {
				if( (a[pos] == bl) || (a[pos] == bu) ) {
					if(strnicmp(a+pos+1, b+1, blen-1) == 0)
						return pos;
				}
			}
		}
		return (string::size_type)string::npos;
	}

	static string validateNick(string tmp) {	
		string::size_type i;
		while( (i = tmp.find_first_of("|$ ")) != string::npos) {
			tmp[i]='_';
		}
		return tmp;
	}

	static string validateMessage(string tmp) {
		string::size_type i;
		while( (i = tmp.find_first_of("|$")) != string::npos) {
			tmp[i]='_';
		}
		return tmp;
	}

	static bool getAway() { return away; };
	static void setAway(bool aAway) { away = aAway; };
	static const string& getAwayMessage() { 
		return awayMsg.empty() ? defaultMsg : awayMsg;
	};
	static void setAwayMessage(const string& aMsg) { awayMsg = aMsg; };
	
private:
	static bool away;
	static string awayMsg;
	static const string defaultMsg;	
};

#endif // !defined(AFX_UTIL_H__1758F242_8D16_4C50_B40D_E59B3DD63913__INCLUDED_)

/**
 * @file Util.h
 * $Id: Util.h,v 1.42 2002/05/09 15:26:46 arnetheduck Exp $
 */

