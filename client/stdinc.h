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


#if !defined(AFX_STDINC_H__65559042_5D04_44EF_9ECF_E0A7FA6E1348__INCLUDED_)
#define AFX_STDINC_H__65559042_5D04_44EF_9ECF_E0A7FA6E1348__INCLUDED_

#pragma warning (disable: 4786) // identifier was truncated to '255' characters in the debug information

#include "config.h"

#ifdef WIN32

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <Winsock2.h>

#include <windows.h>
#include <crtdbg.h>

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
#ifdef HAS_HASH
#include <hash_map>
#endif

using namespace _STL;

#endif // !defined(AFX_STDINC_H__65559042_5D04_44EF_9ECF_E0A7FA6E1348__INCLUDED_)

/**
 * @file stdinc.h
 * $Id: stdinc.h,v 1.1 2002/04/09 18:46:32 arnetheduck Exp $
 */
