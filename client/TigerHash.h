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

#ifndef _TIGER_HASH
#define _TIGER_HASH

#pragma once

class TigerHash {
public:
	/** Hash size in bytes */
	enum { HASH_SIZE = 24 };

	TigerHash() : pos(0) {
		res[0]=_LL(0x0123456789ABCDEF);
		res[1]=_LL(0xFEDCBA9876543210);
		res[2]=_LL(0xF096A5B4C3B2E187);
	}

	~TigerHash() {
	}

	/** Calculates the Tiger hash of the data. */
	void update(const void* data, u_int32_t len);
	/** Call once all data has been processed. */
	u_int8_t* finalize();

	u_int8_t* getResult() { return (u_int8_t*) res; };
private:
	enum { BLOCK_SIZE = 512/8 };
	/** 512 bit blocks for the compress function */
	u_int8_t tmp[512/8];
	/** State / final hash value */
	u_int64_t res[3];
	/** Total number of bytes compressed */
	u_int64_t pos;
	/** S boxes */
	static u_int64_t table[];

	void tigerCompress(const u_int64_t* data, u_int64_t state[3]);
};

#endif // _TIGER_HASH

/**
 * @file
 * $Id: TigerHash.h,v 1.2 2004/01/25 15:29:07 arnetheduck Exp $
 */
