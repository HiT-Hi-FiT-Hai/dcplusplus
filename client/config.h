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

#ifdef _WIN32
// Change these values to use different versions...don't know what happens though...=)
#define WINVER		0x0500
#define _WIN32_IE	0x0500
#define _RICHEDIT_VER	0x0200

#pragma warning(disable: 4711) // function 'xxx' selected for automatic inline expansion
#pragma warning(disable: 4786) // identifier was truncated to '255' characters in the debug information
#pragma warning(disable: 4290) // C++ Exception Specification ignored
#pragma warning(disable: 4512) // can't generate assignment operator
#pragma warning(disable: 4710) // function not inlined

#ifndef CDECL
#define CDECL _cdecl
#endif

#else

#ifndef CDECL
#define CDECL
#endif

#endif

// Remove this line if hashes are not available in your stl
// Hint: the once that comes with mcvc++ 6.0 doesn't have hashes...
#define HAS_HASH 1

// Remove this if you want to try using the msvc 7.x STL
#define HAS_STLPORT 1

// --- Shouldn't have to change anything under here...
#if (defined(HAS_STLPORT))
#if !(defined(_REENTRANT))
#define _REENTRANT 1
#endif
#if !defined(_DEBUG)
#define _STLP_NO_EXCEPTIONS
#undef _STLP_DEBUG
#else
// This enables stlport's debug mode (and slows it down to a crawl...)
//#define _STLP_DEBUG 1
#endif
#endif
#define BZ_NO_STDIO

// User maps instead of hash_maps if they're not available...(even if there's
// no need for the data to be sorted...
#ifdef HAS_HASH
#define HASH_MAP hash_map
#define HASH_MULTIMAP hash_multimap

#ifdef HAS_STLPORT
#define HASH_MAP_X(key, type, hfunc, eq, order) hash_map<key, type, hfunc, eq >
#define HASH_MULTIMAP_X(key, type, hfunc, eq, order) hash_multimap<key, type, hfunc, eq >
#else // Assume the msvc 7.x stl
#define HASH_MAP_X(key, type, hfunc, eq, order) hash_map<key, type, hfunc >
#define HASH_MULTIMAP_X(key, type, hfunc, eq, order) hash_multimap<key, type, hfunc >
#endif

#else
#define HASH_MAP map
#define HASH_MAP_X(key, type, hfunc, eq, order) map<key, type, order >
#define HASH_MULTIMAP multimap
#endif


// These are defined by the C99 standard, but vc 6.0/7.0/7.1 doesn't understand them
#if _MSC_VER == 1200 || _MSC_VER == 1300 || _MSC_VER == 1310
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
typedef signed __int64 int64_t;

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned long u_int32_t;
typedef unsigned __int64 u_int64_t;
#endif

#ifdef _MSC_VER
#define _LL(x) x##I64
#else
#define _LL(x) x##LL
#endif

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STR "\\"
#define I64_FMT "%I64d"
#else
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"
#define I64_FMT "%lld"
#endif

/**
 * @file
 * $Id: config.h,v 1.20 2004/01/24 20:44:22 arnetheduck Exp $
 */
