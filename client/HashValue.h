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

#ifndef _HASH_VALUE
#define _HASH_VALUE

#pragma once
#include "FastAlloc.h"

template<class Hasher>
struct HashValue : FastAlloc<HashValue<Hasher> >{
	enum { SIZE = Hasher::HASH_SIZE };

	HashValue() { };
	HashValue(u_int8_t* aData) { memcpy(data, aData, SIZE); }
	HashValue(const string& base32) { Encoder::fromBase32(base32.c_str(), data, SIZE); };
	HashValue(const HashValue& rhs) { memcpy(data, rhs.data, SIZE); }
	HashValue& operator=(const HashValue& rhs) { memcpy(data, rhs.data, SIZE); return *this; }
	bool operator==(const HashValue& rhs) const { return memcmp(data, rhs.data, SIZE) == 0; }

	string toBase32() { return Encoder::toBase32(data, SIZE); };

	u_int8_t data[SIZE];
};

#endif // _HASH_VALUE

/**
* @file
* $Id: HashValue.h,v 1.1 2004/02/16 13:52:04 arnetheduck Exp $
*/
