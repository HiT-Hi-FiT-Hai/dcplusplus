/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#include "stdinc.h"
#include "DCPlusPlus.h"

#include "Util.h"
#include "File.h"

#include "SettingsManager.h"
#include "ResourceManager.h"
#include "StringTokenizer.h"
#include "SettingsManager.h"

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <ctype.h>
#endif
#include <locale.h>

#include "CID.h"

#include "FastAlloc.h"

FastCriticalSection FastAllocBase::cs;

string Util::emptyString;
wstring Util::emptyStringW;
tstring Util::emptyStringT;

bool Util::away = false;
string Util::awayMsg;
time_t Util::awayTime;

Util::CountryList Util::countries;
string Util::appPath;

static void sgenrand(unsigned long seed);

void Util::initialize() {
	setlocale(LC_ALL, "");

	Text::initialize();

	sgenrand((unsigned long)time(NULL));

	try {
		string file = Util::getAppPath() + "GeoIpCountryWhois.csv";
		string data = File(file, File::READ, File::OPEN).read();

		const char* start = data.c_str();
		string::size_type i = 0;
		string::size_type j = 0;
		string::size_type k = 0;
		CountryIter last = countries.end();
		
		for(;;) {
			i = data.find(',', k);
			if(i == string::npos)
				break;

			j = data.find('\n', i);
			if(j == string::npos)
				break;

			u_int16_t* country = (u_int16_t*)(start + i + 1);
			last = countries.insert(last, make_pair(Util::toUInt32(start + k), *country));

			k = j + 1;
		}
	} catch(const FileException&) {
	}

#ifdef _WIN32
	TCHAR buf[MAX_PATH+1];
	GetModuleFileName(NULL, buf, MAX_PATH);
	appPath = Text::fromT(buf);
	appPath.erase(appPath.rfind('\\') + 1);
#else // _WIN32
	char* home = getenv("HOME");
	if (home) {
		Util::toUtf8(home, appPath);
		appPath += "/.dc++/";
	}
#endif // _WIN32

}

string Util::validateMessage(string tmp, bool reverse, bool checkNewLines) {
	string::size_type i = 0;

	if(reverse) {
		while( (i = tmp.find("&#36;", i)) != string::npos) {
			tmp.replace(i, 5, "$");
			i++;
		}
		i = 0;
		while( (i = tmp.find("&#124;", i)) != string::npos) {
			tmp.replace(i, 6, "|");
			i++;
		}
		i = 0;
		while( (i = tmp.find("&amp;", i)) != string::npos) {
			tmp.replace(i, 5, "&");
			i++;
		}
		if(checkNewLines) {
			// Check all '<' and '|' after newlines...
			i = 0;
			while( (i = tmp.find('\n', i)) != string::npos) {
				if(i + 1 < tmp.length()) {
					if(tmp[i+1] == '[' || tmp[i+1] == '<') {
						tmp.insert(i+1, "- ");
						i += 2;
					}
				}
				i++;
			}
		}
	} else {
		i = 0;
		while( (i = tmp.find("&amp;", i)) != string::npos) {
			tmp.replace(i, 1, "&amp;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find("&#36;", i)) != string::npos) {
			tmp.replace(i, 1, "&amp;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find("&#124;", i)) != string::npos) {
			tmp.replace(i, 1, "&amp;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find('$', i)) != string::npos) {
			tmp.replace(i, 1, "&#36;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find('|', i)) != string::npos) {
			tmp.replace(i, 1, "&#124;");
			i += 5;
		}
	}
	return tmp;
}

static const char badChars[] = { 
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
		31, '<', '>', '/', '"', '|', '?', '*', 0
};

/**
 * Replaces all strange characters in a file with '_'
 * @todo Check for invalid names such as nul and aux...
 */
string Util::validateFileName(string tmp) {
	string::size_type i = 0;

	// First, eliminate forbidden chars
	while( (i = tmp.find_first_of(badChars, i)) != string::npos) {
		tmp[i] = '_';
		i++;
	}

	// Then, eliminate all ':' that are not the second letter ("c:\...")
	i = 0;
	while( (i = tmp.find(':', i)) != string::npos) {
		if(i == 1) {
			i++;
			continue;
		}
		tmp[i] = '_';	
		i++;
	}

	// Remove the .\ that doesn't serve any purpose
	i = 0;
	while( (i = tmp.find("\\.\\", i)) != string::npos) {
		tmp.erase(i+1, 2);
	}

	// Remove any double \\ that are not at the beginning of the path...
	i = 1;
	while( (i = tmp.find("\\\\", i)) != string::npos) {
		tmp.erase(i+1, 1);
	}

	// And last, but not least, the infamous ..\! ...
	i = 0;
	while( ((i = tmp.find("\\..\\", i)) != string::npos) ) {
		tmp[i + 1] = '_';
		tmp[i + 2] = '_';
		tmp[i + 3] = '_';
		i += 2;
	}
	return tmp;
}

string Util::getShortTimeString() {
	char buf[255];
	time_t _tt = time(NULL);
	tm* _tm = localtime(&_tt);
	if(_tm == NULL) {
		strcpy(buf, "xx:xx");
	} else {
		strftime(buf, 254, SETTING(TIME_STAMPS_FORMAT).c_str(), _tm);
	}
	return buf;
}

/**
 * Decodes a URL the best it can...
 * Default ports:
 * http:// -> port 80
 * dchub:// -> port 411
 */
void Util::decodeUrl(const string& url, string& aServer, short& aPort, string& aFile) {
	// First, check for a protocol: xxxx://
	string::size_type i = 0, j, k;
	
	aServer = emptyString;
	aFile = emptyString;

	if( (j=url.find("://", i)) != string::npos) {
		// Protocol found
		string protocol = url.substr(0, j);
		i = j + 3;

		if(protocol == "http") {
			aPort = 80;
		} else if(protocol == "dchub") {
			aPort = 411;
		}
	}

	if( (j=url.find('/', i)) != string::npos) {
		// We have a filename...
		aFile = url.substr(j);
	}

	if( (k=url.find(':', i)) != string::npos) {
		// Port
		if(j == string::npos) {
			aPort = (short)Util::toInt(url.substr(k+1));
		} else if(k < j) {
			aPort = (short)Util::toInt(url.substr(k+1, j-k-1));
		}
	} else {
		k = j;
	}

	if(k == string::npos)
		aServer = url;
	else
		aServer = url.substr(i, k-i);
}

string Util::getAwayMessage() { 
	return (formatTime(awayMsg.empty() ? SETTING(DEFAULT_AWAY_MESSAGE) : awayMsg, awayTime)) + " <DC++ v" VERSIONSTRING ">";
}
string Util::formatBytes(int64_t aBytes) {
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

string Util::formatExactSize(int64_t aBytes) {
	char buf[64];
#ifdef _WIN32
		char number[64];
		NUMBERFMTA nf;
		sprintf(number, "%I64d", aBytes);
		char Dummy[16];
    
		/*No need to read these values from the system because they are not
		used to format the exact size*/
		nf.NumDigits = 0;
		nf.LeadingZero = 0;
		nf.NegativeOrder = 0;
		nf.lpDecimalSep = ",";

		GetLocaleInfoA( LOCALE_SYSTEM_DEFAULT, LOCALE_SGROUPING, Dummy, 16 );
		nf.Grouping = atoi(Dummy);
		GetLocaleInfoA( LOCALE_SYSTEM_DEFAULT, LOCALE_STHOUSAND, Dummy, 16 );
		nf.lpThousandSep = Dummy;

		GetNumberFormatA(LOCALE_USER_DEFAULT, 0, number, &nf, buf, sizeof(buf)/sizeof(buf[0]));
#else
		sprintf(buf, "%'lld", aBytes);
#endif
		sprintf(buf, "%s %s", buf, CSTRING(B));
		return buf;
}

string Util::getLocalIp() {
	string tmp;
	
	char buf[256];
	gethostname(buf, 255);
	hostent* he = gethostbyname(buf);
	if(he == NULL || he->h_addr_list[0] == 0)
		return Util::emptyString;
	sockaddr_in dest;
	int i = 0;
	
	// We take the first ip as default, but if we can find a better one, use it instead...
	memcpy(&(dest.sin_addr), he->h_addr_list[i++], he->h_length);
	tmp = inet_ntoa(dest.sin_addr);
	if(Util::isPrivateIp(tmp) || strncmp(tmp.c_str(), "169", 3) == 0) {
		while(he->h_addr_list[i]) {
			memcpy(&(dest.sin_addr), he->h_addr_list[i], he->h_length);
			string tmp2 = inet_ntoa(dest.sin_addr);
			if(!Util::isPrivateIp(tmp2) && strncmp(tmp2.c_str(), "169", 3) != 0) {
				tmp = tmp2;
			}
			i++;
		}
	}
	return tmp;
}

bool Util::isPrivateIp(string const& ip) {
	struct in_addr addr;

	addr.s_addr = inet_addr(ip.c_str());

	if (addr.s_addr  != INADDR_NONE) {
		unsigned long haddr = ntohl(addr.s_addr);
		return ((haddr & 0xff000000) == 0x0a000000 || // 10.0.0.0/8
				(haddr & 0xff000000) == 0x7f000000 || // 127.0.0.0/8
				(haddr & 0xfff00000) == 0xac100000 || // 172.16.0.0/12
				(haddr & 0xffff0000) == 0xc0a80000);  // 192.168.0.0/16
	}
	return false;
}



typedef const u_int8_t* ccp;
static wchar_t utf8ToLC(ccp& str) {
	wchar_t c = 0;
	if(str[0] & 0x80) {
		if(str[0] & 0x40) {
			if(str[0] & 0x20) {
				if(str[1] == 0 || str[2] == 0 ||
					!((((unsigned char)str[1]) & ~0x3f) == 0x80) ||
					!((((unsigned char)str[2]) & ~0x3f) == 0x80))
				{
					str++;
					return 0;
				}
				c = ((wchar_t)(unsigned char)str[0] & 0xf) << 12 |
					((wchar_t)(unsigned char)str[1] & 0x3f) << 6 |
					((wchar_t)(unsigned char)str[2] & 0x3f);
				str += 3;
			} else {
				if(str[1] == 0 ||
					!((((unsigned char)str[1]) & ~0x3f) == 0x80)) 
				{
					str++;
					return 0;
				}
				c = ((wchar_t)(unsigned char)str[0] & 0x1f) << 6 |
					((wchar_t)(unsigned char)str[1] & 0x3f);
				str += 2;
			}
		} else {
			str++;
			return 0;
		}
	} else {
		wchar_t c = Text::asciiToLower((char)str[0]);
		str++;
		return c;
	}

	return Text::toLower(c);
}

string::size_type Util::findSubString(const string& aString, const string& aSubString, string::size_type start) throw() {
	if(aString.length() < start)
		return (string::size_type)string::npos;

	if(aString.length() - start < aSubString.length())
		return (string::size_type)string::npos;

	if(aSubString.empty())
		return 0;

	// Hm, should start measure in characters or in bytes? bytes for now...
	u_int8_t* tx = (u_int8_t*)aString.c_str() + start;
	u_int8_t* px = (u_int8_t*)aSubString.c_str();

	u_int8_t* end = tx + aString.length() - aSubString.length() + 1;

	wchar_t wp = utf8ToLC(px);

	while(tx < end) {
		u_int8_t* otx = tx;
		if(wp == utf8ToLC(tx)) {
			u_int8_t* px2 = px;
			u_int8_t* tx2 = tx;

			for(;;) {
				if(*px2 == 0)
					return otx - (u_int8_t*)aString.c_str();

				if(utf8ToLC(px2) != utf8ToLC(tx2))
					break;
			}
		}
	}
	return (string::size_type)string::npos;
}

int Util::stricmp(const char* a, const char* b) {
	while(*a) {
		wchar_t ca = 0, cb = 0;
		int na = Text::utf8ToWc(a, ca);
		int nb = Text::utf8ToWc(b, cb);
		if(ca != cb) {
			return (int)cb - (int)ca;
		}
		a+= na < 0 ? 1 : na;
		b+= nb < 0 ? 1 : nb;
	}
	wchar_t ca = 0, cb = 0;
	Text::utf8ToWc(a, ca);
	Text::utf8ToWc(b, cb);
	return (int)cb - (int)ca;
}

int Util::strnicmp(const char* a, const char* b, size_t n) {
	const char* end = a + n;
	while(*a && a < end) {
		wchar_t ca = 0, cb = 0;
		int na = Text::utf8ToWc(a, ca);
		int nb = Text::utf8ToWc(b, cb);
		if(ca != cb) {
			return (int)cb - (int)ca;
		}
		a+= na < 0 ? 1 : na;
		b+= nb < 0 ? 1 : nb;
	}
	wchar_t ca = 0, cb = 0;
	Text::utf8ToWc(a, ca);
	Text::utf8ToWc(b, cb);
	return (int)cb - (int)ca;
}

string Util::encodeURI(const string& aString, bool reverse) {
	// reference: rfc2396
	string tmp = aString;
	if(reverse) {
		string::size_type idx;
		for(idx = 0; idx < tmp.length(); ++idx) {
			if(tmp.length() > idx + 2 && tmp[idx] == '%' && isxdigit(tmp[idx+1]) && isxdigit(tmp[idx+2])) {
				tmp[idx] = fromHexEscape(tmp.substr(idx+1,2));
				tmp.erase(idx+1, 2);
			} else { // reference: rfc1630, magnet-uri draft
				if(tmp[idx] == '+')
					tmp[idx] = ' ';
			}
		}
	} else {
		const string disallowed = ";/?:@&=+$," // reserved
			                      "<>#%\" "    // delimiters
		                          "{}|\\^[]`"; // unwise
		string::size_type idx, loc;
		for(idx = 0; idx < tmp.length(); ++idx) {
			if(tmp[idx] == ' ') {
				tmp[idx] = '+';
			} else {
				if(tmp[idx] <= 0x1F || tmp[idx] >= 0x7f || (loc = disallowed.find_first_of(tmp[idx])) != string::npos) {
					tmp.replace(idx, 1, toHexEscape(tmp[idx]));
					idx+=2;
				}
			}
		}
	}
	return tmp;
}

/**
 * This function takes a string and a set of parameters and transforms them according to
 * a simple formatting rule, similar to strftime. In the message, every parameter should be
 * represented by %[name]. It will then be replaced by the corresponding item in 
 * the params stringmap. After that, the string is passed through strftime with the current
 * date/time and then finally written to the log file. If the parameter is not present at all,
 * it is removed from the string completely...
 */
string Util::formatParams(const string& msg, StringMap& params) {
	string result = msg;

	string::size_type i, j, k;
	i = 0;
	while (( j = result.find("%[", i)) != string::npos) {
		if( (result.size() < j + 2) || ((k = result.find(']', j + 2)) == string::npos) ) {
			break;
		}
		string name = result.substr(j + 2, k - j - 2);
		StringMapIter smi = params.find(name);
		if(smi == params.end()) {
			result.erase(j, k-j + 1);
			i = j;
		} else {
			if(smi->second.find('%') != string::npos) {
				string tmp = smi->second;	// replace all % in params with %% for strftime
				string::size_type m = 0;
				while(( m = tmp.find('%', m)) != string::npos) {
					tmp.replace(m, 1, "%%");
					m+=2;
				}
				result.replace(j, k-j + 1, tmp);
				i = j + tmp.size();
			} else {
				result.replace(j, k-j + 1, smi->second);
				i = j + smi->second.size();
			}
		}
	}

	result = formatTime(result, time(NULL));
	
	return result;
}

string Util::formatTime(const string &msg, const time_t t) {
	if (!msg.empty()) {
		size_t bufsize = msg.size() + 64;
		struct tm* loc = localtime(&t);

		if(!loc) {
			return Util::emptyString;
		}

		AutoArray<char> buf(new char[bufsize]);

		while(!strftime(buf, bufsize-1, msg.c_str(), loc)) {
			bufsize+=64;
			buf = new char[bufsize];
		}

		return string(buf);
	}
	return Util::emptyString;
}

/* Below is a high-speed random number generator with much
   better granularity than the CRT one in msvc...(no, I didn't
   write it...see copyright) */ 
/* Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.
   Any feedback is very welcome. For any question, comments,       
   see http://www.math.keio.ac.jp/matumoto/emt.html or email       
   matumoto@math.keio.ac.jp */       
/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializing the array with a NONZERO seed */
static void sgenrand(unsigned long seed) {
	/* setting initial seeds to mt[N] using         */
	/* the generator Line 25 of Table 1 in          */
	/* [KNUTH 1981, The Art of Computer Programming */
	/*    Vol. 2 (2nd Ed.), pp102]                  */
	mt[0]= seed & 0xffffffff;
	for (mti=1; mti<N; mti++)
		mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;
}

u_int32_t Util::rand() {
	unsigned long y;
	static unsigned long mag01[2]={0x0, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= N) { /* generate N words at one time */
		int kk;

		if (mti == N+1)   /* if sgenrand() has not been called, */
			sgenrand(4357); /* a default initial seed is used   */

		for (kk=0;kk<N-M;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		for (;kk<N-1;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

		mti = 0;
	}

	y = mt[mti++];
	y ^= TEMPERING_SHIFT_U(y);
	y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
	y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
	y ^= TEMPERING_SHIFT_L(y);

	return y; 
}

string Util::getOsVersion() {
#ifdef _WIN32
	string os;

	OSVERSIONINFOEX ver;
	memset(&ver, 0, sizeof(OSVERSIONINFOEX));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if(!GetVersionEx((OSVERSIONINFO*)&ver)) {
		ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if(!GetVersionEx((OSVERSIONINFO*)&ver)) {
			os = "Windows (version unknown)";
		}
	}

	if(os.empty()) {
		if(ver.dwPlatformId != VER_PLATFORM_WIN32_NT) {
			os = "Win9x/ME/Junk";
		} else if(ver.dwMajorVersion == 4) {
			os = "WinNT4";
		} else if(ver.dwMajorVersion == 5) {
			if(ver.dwMinorVersion == 0) {
				os = "Win2000";
			} else if(ver.dwMinorVersion == 1) {
				os = "WinXP";
			} else if(ver.dwMinorVersion == 2) {
				os = "Win2003";
			} else {
				os = "Unknown WinNT5";
			}

			if(ver.wProductType & VER_NT_WORKSTATION)
				os += " Pro";
			else if(ver.wProductType & VER_NT_SERVER)
				os += " Server";
			else if(ver.wProductType & VER_NT_DOMAIN_CONTROLLER)
				os += " DC";
		}

		if(ver.wServicePackMajor != 0) {
			os += "SP";
			os += Util::toString(ver.wServicePackMajor);
			if(ver.wServicePackMinor != 0) {
				os += '.';
				os += Util::toString(ver.wServicePackMinor);
			}
		}
	}

	return os;

#else // _WIN32
	utsname n;

	if(uname(&n) != 0) {
		return "unix (unknown version)";
	}

	return string(n.sysname) + " " + string(n.release) + " (" + string(n.machine) + ")";

#endif // _WIN32
}

/*	getIpCountry
	This function returns the country(Abbreviation) of an ip
	for exemple: it returns "PT", whitch standards for "Portugal"
	more info: http://www.maxmind.com/app/csv
*/
string Util::getIpCountry (string IP) {
	if (BOOLSETTING(GET_USER_COUNTRY)) {
		dcassert(count(IP.begin(), IP.end(), '.') == 3);

		//e.g IP 23.24.25.26 : w=23, x=24, y=25, z=26
		string::size_type a = IP.find('.');
		string::size_type b = IP.find('.', a+1);
		string::size_type c = IP.find('.', b+2);

		u_int32_t ipnum = (Util::toUInt32(IP.c_str()) << 24) | 
			(Util::toUInt32(IP.c_str() + a + 1) << 16) | 
			(Util::toUInt32(IP.c_str() + b + 1) << 8) | 
			(Util::toUInt32(IP.c_str() + c + 1) );

		CountryIter i = countries.lower_bound(ipnum);

		if(i != countries.end()) {
			return string((char*)&(i->second), 2);
		}
	}

	return Util::emptyString; //if doesn't returned anything already, something is wrong...
}

string Util::toDOS(const string& tmp) {
	if(tmp.empty())
		return Util::emptyString;

	string tmp2(tmp);

	if(tmp2[0] == '\r' && (tmp2.size() == 1 || tmp2[1] != '\n')) {
		tmp2.insert(1, "\n");
	}
	for(string::size_type i = 1; i < tmp2.size() - 1; ++i) {
		if(tmp2[i] == '\r' && tmp2[i+1] != '\n') {
			// Mac ending
			tmp2.insert(i+1, "\n");
			i++;
		} else if(tmp2[i] == '\n' && tmp2[i-1] != '\r') {
			// Unix encoding
			tmp2.insert(i, "\r");
			i++;
		}
	}
	return tmp2;
}

int Util::getOsMajor() 
{
#ifdef _WIN32
	OSVERSIONINFOEX ver;
	memset(&ver, 0, sizeof(OSVERSIONINFOEX));
	if(!GetVersionEx((OSVERSIONINFO*)&ver)) 
	{
		ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	}
	GetVersionEx((OSVERSIONINFO*)&ver);
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	return ver.dwMajorVersion;
#endif //_WIN32
}

int Util::getOsMinor() 
{
#ifdef _WIN32
	OSVERSIONINFOEX ver;
	memset(&ver, 0, sizeof(OSVERSIONINFOEX));
	if(!GetVersionEx((OSVERSIONINFO*)&ver)) 
	{
		ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	}
	GetVersionEx((OSVERSIONINFO*)&ver);
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	return ver.dwMinorVersion;
#endif //_WIN32
}
/**
 * @file
 * $Id: Util.cpp,v 1.66 2004/09/23 09:06:26 arnetheduck Exp $
 */

