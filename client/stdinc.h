/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(STDINC_H)
#define STDINC_H

// This enables stlport's debug mode (and slows it down to a crawl...)
//#define _STLP_DEBUG 1
//#define _STLP_USE_NEWALLOC 1

// --- Shouldn't have to change anything under here...

#ifndef _REENTRANT
# define _REENTRANT 1
#endif

#ifndef BZ_NO_STDIO
#define BZ_NO_STDIO 1
#endif

#ifndef USE_SYS_STL
#define USE_SYS_STL 1
#endif

#define _STLP_DONT_USE_SHORT_STRING_OPTIM 1	// Lots of memory issues with this undefined...wonder what's up with that..
#define _STLP_USE_PTR_SPECIALIZATIONS 1
#define _STLP_NO_ANACHRONISMS 1
#define _STLP_NO_CUSTOM_IO 1
#define _STLP_NO_IOSTREAMS 1
#ifndef _DEBUG
# define _STLP_DONT_USE_EXCEPTIONS 1
#endif

#ifdef _MSC_VER

//disable the deprecated warnings for the CRT functions.
#define _CRT_SECURE_NO_DEPRECATE 1
#define _ATL_SECURE_NO_DEPRECATE 1
#define _CRT_NON_CONFORMING_SWPRINTFS 1

# pragma warning(disable: 4711) // function 'xxx' selected for automatic inline expansion
# pragma warning(disable: 4786) // identifier was truncated to '255' characters in the debug information
# pragma warning(disable: 4290) // C++ Exception Specification ignored
# pragma warning(disable: 4127) // constant expression
# pragma warning(disable: 4710) // function not inlined
# pragma warning(disable: 4503) // decorated name length exceeded, name was truncated
# pragma warning(disable: 4428) // universal-character-name encountered in source

typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

# ifndef CDECL
#  define CDECL _cdecl
# endif

#else // _MSC_VER

# ifndef CDECL
#  define CDECL
# endif

#endif // _MSC_VER

#ifdef _WIN32
# define _WIN32_WINNT 0x0501
# define _WIN32_IE	0x0500

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define _WTL_NO_CSTRING
#define _ATL_NO_OPENGL
#define _ATL_NO_MSIMG
#define _ATL_NO_COM
#define _ATL_NO_HOSTING
#define _ATL_NO_OLD_NAMES

#include <winsock2.h>

#include <windows.h>
#include <tchar.h>
#include <shlobj.h>

#else
#include <unistd.h>
#include <stdint.h>
#endif

#ifdef _MSC_VER
#include <crtdbg.h>
#else
#include <assert.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <sys/types.h>
#include <time.h>
#include <locale.h>

#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <deque>
#include <list>
#include <utility>
#include <functional>

#ifdef _STLPORT_VERSION
# define HASH_SET hash_set
# define HASH_MAP hash_map
// STLPort 5.0.2 hash_multimap buggy
# define HASH_MULTIMAP multimap
# define HASH_SET_X(key, hfunc, eq, order) hash_set<key, hfunc, eq >
# define HASH_MAP_X(key, type, hfunc, eq, order) hash_map<key, type, hfunc, eq >
# define HASH_MULTIMAP_X(key, type, hfunc, eq, order) multimap<key, type, order > 

#include <hash_map>
#include <hash_set>
using namespace std;

#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)  // Using GNU C++ library?
# define HASH_SET hash_set
# define HASH_MAP hash_map
# define HASH_MULTIMAP hash_multimap
# define HASH_SET_X(key, hfunc, eq, order) hash_set<key, hfunc, eq >
# define HASH_MAP_X(key, type, hfunc, eq, order) hash_map<key, type, hfunc, eq >
# define HASH_MULTIMAP_X(key, type, hfunc, eq, order) hash_multimap<key, type, hfunc, eq >

#include <ext/hash_map>
#include <ext/hash_set>
#include <ext/functional>
using namespace std;
using namespace __gnu_cxx;

// GNU C++ library doesn't have hash(std::string) or hash(long long int)
namespace __gnu_cxx {
	template<> struct hash<std::string> {
		size_t operator()(const std::string& x) const
		{ return hash<const char*>()(x.c_str()); }
	};
	template<> struct hash<long long int> {
		size_t operator()(long long int x) const { return x; }
	};
}

#elif defined(_MSC_VER)  // Assume the msvc stl
# define HASH_SET hash_set
# define HASH_MAP hash_map
# define HASH_MULTIMAP hash_multimap
# define HASH_SET_X(key, hfunc, eq, order) hash_set<key, hfunc >
# define HASH_MAP_X(key, type, hfunc, eq, order) hash_map<key, type, hfunc >
# define HASH_MULTIMAP_X(key, type, hfunc, eq, order) hash_multimap<key, type, hfunc >

#include <hash_map>
#include <hash_set>
using namespace std;
using namespace stdext;

#else
# define HASH_SET set
# define HASH_MAP map
# define HASH_SET_X(key, hfunc, eq, order)
# define HASH_MAP_X(key, type, hfunc, eq, order) map<key, type, order >
# define HASH_MULTIMAP multimap
# define HASH_MULTIMAP_X(key, type, hfunc, eq, order) multimap<key, type, order >
#endif

#endif // !defined(STDINC_H)
