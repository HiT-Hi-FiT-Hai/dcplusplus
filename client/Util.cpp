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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "Util.h"

#include "SettingsManager.h"
#include "ResourceManager.h"

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <ctype.h>
#endif

#ifdef HAS_STLPORT
allocator<char> FastAllocBase::alloc;
#endif

string Util::emptyString;

bool Util::away = false;
string Util::awayMsg;
time_t Util::awayTime;
char Util::upper[256];
char Util::lower[256];
int8_t Util::cmp[256][256];
int8_t Util::cmpi[256][256];

/* CRC table for SFV files, ripped from pdSFV 1.2 */
u_int32_t Util::crcTable[] = {
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba
, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3
, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988
, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91
, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de
, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7
, 0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec
, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5
, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172
, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b
, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940
, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59
, 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116
, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f
, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924
, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d
, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a
, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433
, 0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818
, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01
, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e
, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457
, 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c
, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65
, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2
, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb
, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0
, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9
, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086
, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f
, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4
, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad
, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a
, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683
, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8
, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1
, 0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe
, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7
, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc
, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5
, 0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252
, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b
, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60
, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79
, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236
, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f
, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04
, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d
, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a
, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713
, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38
, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21
, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e
, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777
, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c
, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45
, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2
, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db
, 0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0
, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9
, 0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6
, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf
, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94
, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static void sgenrand(unsigned long seed);

void Util::initialize() {
	int i;
	for(i = 0; i < 256; ++i) {
#ifdef _WIN32
		upper[i] = (char)CharUpper((LPSTR)i);
		lower[i] = (char)CharLower((LPSTR)i);
#else
		upper[i] = (char)toupper(i);
		lower[i] = (char)tolower(i);
#endif
	}

	// Now initialize the compare table to the current locale (hm...hopefully we
	// won't have strange problems because of this (users from different locales for instance)
	for(i = 0; i < 256; ++i) {
		for(int j = 0; j < 256; ++j) {
			cmp[i][j] = (int8_t)::strncmp((char*)&i, (char*)&j, 1);
			cmpi[i][j] = (int8_t)::strncmp((char*)&lower[i], (char*)&lower[j], 1);
		}
	}

	sgenrand(time(NULL));
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
		if(k < j)
			aPort = (short)Util::toInt(url.substr(k+1, j-k-1));
	} else {
		k = j;
	}

	// Only the server should be left now...
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
	if( strncmp(tmp.c_str(), "192", 3) == 0 || 
		strncmp(tmp.c_str(), "169", 3) == 0 || 
		strncmp(tmp.c_str(), "127", 3) == 0 || 
		strncmp(tmp.c_str(), "10.", 3) == 0 ) {
		
		while(he->h_addr_list[i]) {
			memcpy(&(dest.sin_addr), he->h_addr_list[i], he->h_length);
			string tmp2 = inet_ntoa(dest.sin_addr);
			if(	strncmp(tmp2.c_str(), "192", 3) != 0 &&
				strncmp(tmp2.c_str(), "169", 3) != 0 &&
				strncmp(tmp2.c_str(), "127", 3) != 0 &&
				strncmp(tmp2.c_str(), "10.", 3) != 0) {
				
				tmp = tmp2;
			}
			i++;
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

/**
 * @file
 * $Id: Util.cpp,v 1.41 2004/01/04 17:32:47 arnetheduck Exp $
 */

