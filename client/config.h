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

#ifdef WIN32
// Change these values to use different versions...don't know what happens though...=)
#define WINVER		0x0400
#define _WIN32_IE	0x0500
#define _RICHEDIT_VER	0x0200

#pragma warning(disable: 4711) // function 'xxx' selected for automatic inline expansion
#pragma warning(disable: 4786) // identifier was truncated to '255' characters in the debug information
#pragma warning(disable: 4290) // C++ Exception Specification ignored
#pragma warning(disable: 4127) // constant expression
#pragma warning(disable: 4512) // can't generate assignment operator (so what?)
#pragma warning(disable: 4710) // function not inlined

#endif

// This enables stlport's debug mode (and slows it down to a crawl...)
# ifdef _DEBUG
//# define _STLP_DEBUG 1
# else
# undef _STLP_DEBUG
# endif

// Remove this line if hashes are not available in your stl
// Hint: the once that comes with mcvc++ 6.0 doesn't have hashes...
#define HAS_HASH 1

// --- Shouldn't have to change anything under here...

// User maps instead of hash_maps if they're not available...(even if there's
// no need for the data to be sorted...
#ifdef HAS_HASH
#define HASH_MAP hash_map
#define HASH_MULTIMAP hash_multimap
#else
#define HASH_MAP map
#define HASH_MULTIMAP multimap
#endif

// These are defined by the C99 standard, but vc 6.0/7.0 doesn't understand them
#if _MSC_VER == 1200 || _MSC_VER == 1300
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
typedef signed __int64 int64_t;

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned long u_int32_t;
typedef unsigned __int64 u_int64_t;
#endif

// Just a test to see if things compile in gcc...
#ifndef WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

/**
 * @file config.h
 * $Id: config.h,v 1.6 2002/04/09 18:43:28 arnetheduck Exp $
 */