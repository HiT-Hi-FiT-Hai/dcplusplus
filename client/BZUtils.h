/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef _BZ_UTILS
#define _BZ_UTILS

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _WIN32
#include "../bzip2/bzlib.h"
#else
#include <bzlib.h>
#endif

class BZFilter {
public:
	BZFilter();
	~BZFilter();
	/**
	* Compress data.
	* @param in Input data
	* @param insize Input size (Set to 0 to indicate that no more data will follow)
	* @param out Output buffer
	* @param outsize Output size, set to compressed size on return.
	* @return True if there's more processing to be done.
	*/
	bool operator()(const void* in, size_t& insize, void* out, size_t& outsize);
private:
	bz_stream zs;
};

class UnBZFilter {
public:
	UnBZFilter();
	~UnBZFilter();
	/**
	* Decompress data.
	* @param in Input data
	* @param insize Input size (Set to 0 to indicate that no more data will follow)
	* @param out Output buffer
	* @param outsize Output size, set to decompressed size on return.
	* @return True if there's more processing to be done
	*/
	bool operator()(const void* in, size_t& insize, void* out, size_t& outsize);
private:
	bz_stream zs;
};

#endif // _BZ_UTILS

/**
* @file
* $Id: BZUtils.h,v 1.5 2005/01/05 19:30:27 arnetheduck Exp $
*/
