/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

/** Evaluates op(pair<T1, T2>.first, compareTo) */
template<class T1, class T2, class op = equal_to<T1> >
class CompareFirst {
public:
	CompareFirst(const T1& compareTo) : a(compareTo) { };
	bool operator()(const pair<T1, T2>& p) { return op()(p.first, a); };
private:
	const T1& a;
};

/** Evaluates op(pair<T1, T2>.second, compareTo) */
template<class T1, class T2, class op = equal_to<T2> >
class CompareSecond {
public:
	CompareSecond(const T2& compareTo) : a(compareTo) { };
	bool operator()(const pair<T1, T2>& p) { return op()(p.second, a); };
private:
	const T2& a;
};

template<class T>
struct PointerHash {
	static const size_t bucket_size = 4;
	static const size_t min_buckets = 8;
	size_t operator()(const T* a) const { return ((size_t)a)/sizeof(T); };
	bool operator()(const T* a, const T* b) { return a < b; };
};
template<>
struct PointerHash<void> {
	size_t operator()(const void* a) const { return ((size_t)a)>>2; };
};

/** 
 * Compares two values
 * @return -1 if v1 < v2, 0 if v1 == v2 and 1 if v1 > v2
 */
template<typename T1>
int compare(const T1& v1, const T1& v2) { return (v1 < v2) ? -1 : ((v1 == v2) ? 0 : 1); };

class Flags {
	public:
		Flags() : flags(0) { };
		Flags(const Flags& rhs) : flags(rhs.flags) { };
		Flags(int f) : flags(f) { };
		bool isSet(int aFlag) const { return (flags & aFlag) == aFlag; };
		void setFlag(int aFlag) { flags |= aFlag; };
		void unsetFlag(int aFlag) { flags &= ~aFlag; };

	private:
		int flags;
};

template<typename T>
class AutoArray {
	typedef T* TPtr;
	typedef T& TRef;
public:
	AutoArray(size_t size) : p(new T[size]) { };
	~AutoArray() { delete[] p; };
	operator TPtr() { return p; };

private:
	AutoArray(const AutoArray&) { };
	void operator=(const AutoArray&) { };

	TPtr p;
};

class Util  
{
public:
	static u_int32_t crcTable[];

	static string emptyString;

	static void initialize();

	static void ensureDirectory(const string& aFile)
	{
#ifdef WIN32
		string::size_type start = 0;
		
		while( (start = aFile.find_first_of("\\/", start)) != string::npos) {
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

	static string getAppName() {
#ifdef WIN32
		TCHAR buf[MAX_PATH+1];
		DWORD x = GetModuleFileName(NULL, buf, MAX_PATH);
		return string(buf, x);
#else // WIN32
		return emptyString;
#endif // WIN32
	}	

	static string getTempPath() {
#ifdef WIN32
		TCHAR buf[MAX_PATH + 1];
		DWORD x = GetTempPath(MAX_PATH, buf);
		return string(buf, x);
#else
		return emptyString;
#endif
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

	static string getFilePath(const string& path) {
		string::size_type i = path.rfind('\\');
		return (i != string::npos) ? path.substr(0, i + 1) : path;
	}
	static string getFileName(const string& path) {
		string::size_type i = path.rfind('\\');
		return (i != string::npos) ? path.substr(i + 1) : path;
	}
	static string getFileExt(const string& path) {
		string::size_type i = path.rfind('.');
		return (i != string::npos) ? path.substr(i) : Util::emptyString;
	}
	static string getLastDir(const string& path) {
		string::size_type i = path.rfind('\\');
		if(i == string::npos)
			return Util::emptyString;
		string::size_type j = path.rfind('\\', i-1);
		return (j != string::npos) ? path.substr(j+1, j-i-1) : path;
	}
	
	static void decodeUrl(const string& aUrl, string& aServer, short& aPort, string& aFile);
	static string filterFileName(const string& aFile);
	
	static string formatBytes(const string& aString) {
		return formatBytes(toInt64(aString));
	}

	static string getShortTimeString() {
		char buf[8];
		time_t _tt = time(NULL);
		tm* _tm = localtime(&_tt);
		if(_tm == NULL) {
			strcpy(buf, "xx:xx");
		} else {
			strftime(buf, 8, "%H:%M", _tm);
		}
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
	static string formatTime(const string &msg, const time_t tm);

	static string formatNumber(int64_t aNumber) {
		char buf[64];
#ifdef WIN32
		char number[64];
		sprintf(number, "%I64d", aNumber);
		SetLocaleInfoA(GetUserDefaultLCID(), LOCALE_IDIGITS, "0");
		GetNumberFormatA(LOCALE_USER_DEFAULT, 0, number, NULL, buf, sizeof(buf)/sizeof(buf[0]));
#else
		sprintf(buf, "%lld", aNumber);
#endif		
		return buf;
	}

	static string formatNumber(const string& aString) {
#ifdef WIN32
		char buf[64];
		SetLocaleInfoA(GetUserDefaultLCID(), LOCALE_IDIGITS, "0");
		GetNumberFormatA(LOCALE_USER_DEFAULT, 0, aString.c_str(), NULL, buf, sizeof(buf)/sizeof(buf[0]));
#else
		return aString; //formatNumber(toInt64(aString));
#endif
	}


	static string toLower(const string& aString) { return toLower(aString.c_str(), aString.length()); };
	static string toLower(const char* aString, int len = -1) {
		string tmp;
		tmp.resize((len == -1) ? strlen(aString) : len);
		for(string::size_type i = 0; aString[i]; i++) {
			tmp[i] = toLower(aString[i]);
		}
		return tmp;
	}
	static char toLower(char c) { return lower[(u_int8_t)c]; };
	static u_int8_t toLower(u_int8_t c) { return lower[c]; };
	static void toLower2(string& aString) {
		for(string::size_type i = 0; i < aString.length(); ++i) {
			aString[i] = toLower(aString[i]);
		}
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

	static string toString(const int64_t& val) {
		char buf[32];
#ifdef WIN32
		return _i64toa(val, buf, 10);
#else
		sprintf(buf, "%lld", val);
		return buf;
#endif
	}

	static string toString(const u_int32_t& val) {
		char buf[16];
		sprintf(buf, "%u", val);
		return buf;
	}
	static string toString(const int& val) {
		char buf[16];
		sprintf(buf, "%d", val);
		return buf;
	}
	static string toString(const long& val) {
		char buf[16];
		sprintf(buf, "%ld", val);
		return buf;
	}
	static string toString(const double& val) {
		char buf[16];
		sprintf(buf, "%.2f", val);
		return buf;
	}

	static string getLocalIp();
	/**
	 * Case insensitive substring search.
	 * @return First position found or string::npos
	 */
	static string::size_type findSubString(const string& aString, const string& aSubString, string::size_type start = 0) {
		if(aString.length() < start)
			return (string::size_type)string::npos;

		if(aString.length() < aSubString.length())
			return (string::size_type)string::npos;

		if(aSubString.empty())
			return 0;

		u_int8_t* tx = (u_int8_t*)aString.c_str();
		u_int8_t* px = (u_int8_t*)aSubString.c_str();

		u_int8_t p = Util::toLower(px[0]);

		u_int8_t* end = tx + aString.length() - aSubString.length() + 1;

		tx += start;

		while(tx < end) {
			if(p == Util::toLower(tx[0])) {
				int i = 1;

				for(; px[i] && Util::toLower(px[i]) == Util::toLower(tx[i]); ++i)
					;	// Empty

				if(px[i] == 0)
					return tx - (u_int8_t*)aString.c_str();
			}

			tx++;
		}
		return (string::size_type)string::npos;
	}

	/* Table-driven versions of strnicmp and stricmp */
	static int stricmp(const char* a, const char* b) {
		// return ::stricmp(a, b);
		while(*a && (cmpi[(u_int8_t)*a][(u_int8_t)*b] == 0)) {
			a++; b++;
		}
		return cmpi[(u_int8_t)*a][(u_int8_t)*b];
	}
	static int strnicmp(const char* a, const char* b, int n) {
		// return ::strnicmp(a, b, n);
		while(n && *a && (cmpi[(u_int8_t)*a][(u_int8_t)*b] == 0)) {
			n--; a++; b++;
		}
		return (n == 0) ? 0 : cmpi[(u_int8_t)*a][(u_int8_t)*b];
	}
	static int stricmp(const string& a, const string& b) { return stricmp(a.c_str(), b.c_str()); };
	static int strnicmp(const string& a, const string& b, int n) { return strnicmp(a.c_str(), b.c_str(), n); };
	
	static string validateNick(string tmp) {	
		string::size_type i;
		while( (i = tmp.find_first_of("|$ ")) != string::npos) {
			tmp[i]='_';
		}
		return tmp;
	}

	static string validateMessage(string tmp, bool reverse, bool checkNewLines = true);

	static string getOsVersion();

	static bool getAway() { return away; };
	static void setAway(bool aAway) {
		away = aAway;
		if (away)
			awayTime = time(NULL);
	};
	static string getAwayMessage();

	static void setAwayMessage(const string& aMsg) { awayMsg = aMsg; };

	static u_int32_t rand();
	static u_int32_t rand(u_int32_t high) { return rand() % high; };
	static u_int32_t rand(u_int32_t low, u_int32_t high) { return rand(high-low) + low; };
	static double randd() { return ((double)rand()) / ((double)0xffffffff); };

private:
	static bool away;
	static string awayMsg;
	static time_t awayTime;
	static char upper[];
	static char lower[];
	static int8_t cmp[256][256];
	static int8_t cmpi[256][256];
};

/** Case insensitive hash function for strings */
struct noCaseStringHash {
#if _MSC_VER == 1200 
	enum {bucket_size = 4}; 
	enum {min_buckets = 8}; 
#else 
	static const size_t bucket_size = 4; 
	static const size_t min_buckets = 8; 
#endif // _MSC_VER == 1200

	size_t operator()(const string& s) const {
		size_t x = 0;
		const char* y = s.data();
		string::size_type j = s.size();
		for(string::size_type i = 0; i < j; ++i) {
			x = x*31 + (size_t)Util::toLower(y[i]);
		}
		return x;
	}
	bool operator()(const string& a, const string& b) const {
		return Util::stricmp(a, b) == -1;
	}
};

/** Case insensitive string comparison */
struct noCaseStringEq {
	bool operator()(const string& a, const string& b) const {
		return Util::stricmp(a.c_str(), b.c_str()) == 0;
	}
};

/** Case insensitive string ordering */
struct noCaseStringLess {
	bool operator()(const string& a, const string& b) const {
		return Util::stricmp(a.c_str(), b.c_str()) == -1;
	}
};


#endif // !defined(AFX_UTIL_H__1758F242_8D16_4C50_B40D_E59B3DD63913__INCLUDED_)

/**
 * @file
 * $Id: Util.h,v 1.62 2003/10/20 21:04:55 arnetheduck Exp $
 */
