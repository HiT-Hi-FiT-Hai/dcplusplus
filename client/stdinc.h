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


#if !defined(AFX_STDINC_H__65559042_5D04_44EF_9ECF_E0A7FA6E1348__INCLUDED_)
#define AFX_STDINC_H__65559042_5D04_44EF_9ECF_E0A7FA6E1348__INCLUDED_

#include "config.h"

#ifdef _WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define _WTL_NO_CSTRING
#define _ATL_NO_OPENGL
#define _ATL_NO_MSIMG
#define _ATL_NO_COM
#define _ATL_NO_HOSTING
#define _ATL_NO_OLD_NAMES

#include <Winsock2.h>

#include <windows.h>
#include <crtdbg.h>
#include <tchar.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <sys/types.h>

#include <time.h>

#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <deque>
#include <set>

#include <utility>

#ifdef HAVE_STLPORT
using namespace _STL;
#include <hash_map>

#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)  // Using GNU C++ library?
#include <ext/hash_map>
                                                                                
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
#else // __GLIBCPP__

using namespace std;
#include <hash_map>
using namespace stdext;

#endif // __GLIBCPP__

#endif // !defined(AFX_STDINC_H__65559042_5D04_44EF_9ECF_E0A7FA6E1348__INCLUDED_)

/**
 * @file
 * $Id: stdinc.h,v 1.11 2004/11/29 23:21:30 arnetheduck Exp $
 */
