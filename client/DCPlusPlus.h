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


#ifndef _DCPLUSPLUS_H
#define _DCPLUSPLUS_H

#ifdef _DEBUG

#ifdef WIN32
// Warning C4130: '==' : logical operation on address of string constant
#pragma warning (disable: 4130)
#endif

inline void CDECL debugTrace(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	
#ifdef WIN32
	char buf[512];
	
	_vsnprintf(buf, sizeof(buf), format, args);
	OutputDebugStringA(buf);
#else // WIN32
	vprintf(format, args);
#endif // WIN32
	va_end(args);
};

#define dcdebug debugTrace
#ifdef WIN32
#define dcassert(exp) _ASSERTE(exp)
#else
#include <assert.h>
#define dcassert(exp) assert(exp)
#endif
#define dcdrun(exp) exp
#else //_DEBUG
#define dcdebug true ? ((void)0) : printf
#define dcassert(exp) 
#define dcdrun(exp)
#endif //_DEBUG

// Make sure we're using the templates from algorithm...
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

typedef vector<string> StringList;
typedef StringList::iterator StringIter;
typedef StringList::const_iterator StringIterC;

typedef HASH_MAP<string, string> StringMap;
typedef StringMap::iterator StringMapIter;

#include "Singleton.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "version.h"

extern void startup();
extern void shutdown();

#define GETSET(type, name, name2) private: type name; public: type get##name2() const { return name; }; void set##name2(type a##name2) { name = a##name2; };
#define GETSETREF(type, name, name2) private: type name; public: const type& get##name2() const { return name; }; void set##name2(const type& a##name2) { name = a##name2; };

#endif // _DCPLUSPLUS_H

/**
 * @file DCPlusPlus.h
 * $Id: DCPlusPlus.h,v 1.27 2002/06/02 00:12:44 arnetheduck Exp $
 */

