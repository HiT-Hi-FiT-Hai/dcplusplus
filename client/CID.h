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

#if !defined(AFX_CID_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_)
#define AFX_CID_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_

#include "Encoder.h"
#include "Util.h"

class CID {
public:
	struct Hash {
		size_t operator()(const CID& c) const { return c.toHash(); }
	};
	CID() : cid(0) { }
	CID(u_int64_t c) : cid(c) { }
	CID(const string& base32) { Encoder::fromBase32(base32.c_str(),(u_int8_t*)&cid, sizeof(cid)); }
	CID(const CID& rhs) : cid(rhs.cid) { }

	operator u_int64_t() { return cid; }

	CID& operator=(const CID& rhs) { cid = rhs.cid; return *this; }		
	bool operator==(const CID& rhs) const { return cid == rhs.cid; }
	bool operator<(const CID& rhs) const { return cid < rhs.cid; }

	string toBase32() const { return Encoder::toBase32((u_int8_t*)&cid, sizeof(cid)); }
	string& toBase32(string& tmp) const { return Encoder::toBase32((u_int8_t*)&cid, sizeof(cid), tmp); }

	size_t toHash() const { size_t* p = (size_t*)&cid; return *p ^ *(p+1); };

	bool isZero() const { return cid == 0; }

	static CID generate() { 
		CID cid; 
		u_int32_t* c = (u_int32_t*)&cid.cid; 
		*(c++) = Util::rand();
		*(c++) = Util::rand();
		return cid;
	}

	static const u_int64_t zero = 0;
private:
	u_int64_t cid;
};

#endif

/**
* @file
* $Id: CID.h,v 1.1 2004/04/04 12:11:51 arnetheduck Exp $
*/
