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

#ifndef _Z_UTILS
#define _Z_UTILS

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _WIN32
#include "../zlib/zlib.h"
#else
#include <zlib.h>
#endif

class ZFilter {
public:
	ZFilter();
	~ZFilter();
	/**
	 * Compress data.
	 * @param in Input data
	 * @param insize Input size (Set to 0 to indicate that no more data will follow)
	 * @param out Output buffer
	 * @param outsize Output size, set to compressed size on return.
	 * @return True if there's more processing to be done
	 */
	bool operator()(const void* in, size_t& insize, void* out, size_t& outsize);
private:
	z_stream zs;
};

class UnZFilter {
public:
	UnZFilter();
	~UnZFilter();
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
	z_stream zs;
};

class CRC32Filter {
public:
	CRC32Filter() : crc(crc32(0, NULL, 0)) { }
	void operator()(const void* buf, size_t len) { crc = crc32(crc, (const Bytef*)buf, len); }
	u_int32_t getValue() const { return crc; }
private:
	u_int32_t crc;
};

#endif // _Z_UTILS

/**
 * @file
 * $Id: ZUtils.h,v 1.4 2004/09/06 12:32:43 arnetheduck Exp $
 */
