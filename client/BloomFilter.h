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

#ifndef _BLOOM_FILTER
#define _BLOOM_FILTER

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZUtils.h"

struct CRC32Hash {
	size_t operator()(const void* buf, size_t len) { f(buf, len); return f.getValue(); }
private:
	CRC32Filter f;
};

template<size_t N, class HashFunc = CRC32Hash>
class BloomFilter {
public:
	BloomFilter(size_t tableSize) { table.resize(tableSize); };
	~BloomFilter() { };

	void add(const string& s) { xadd(s, N); }
	bool match(const StringList& s) { 
		for(StringList::const_iterator i = s.begin(); i != s.end(); ++i) {
			if(!match(*i))
				return false;
		}
		return true;
	}
	bool match(const string& s) { 
		if(s.length() >= N) {
			string::size_type l = s.length() - N;
			for(string::size_type i = 0; i <= l; ++i) {
				if(!table[getPos(s, i, N)]) {
					return false;
				}
			}
		}
		return true;
	}
	void clear() {
		size_t s = table.size();
		table.clear();
		table.resize(s);
	}
#ifdef TESTER
	void print_table_status() {
		int tot = 0;
		for (unsigned int i = 0; i < table.size(); ++i) if (table[i] == true) ++tot;

		std::cout << "table status: " << tot << " of " << table.size()
			<< " filled, for an occupancy percentage of " << (100.*tot)/table.size()
			<< "%" << std::endl;
	}
#endif
private:
	void xadd(const string& s, size_t n) {
		if(s.length() >= n) {
			string::size_type l = s.length() - n;
			for(string::size_type i = 0; i <= l; ++i) {
				table[getPos(s, i, n)] = true;
			}
		} 
	}
	size_t getPos(const string& s, int i, int l) { HashFunc h; return (h(&s[i], l) % table.size()); }
	
	vector<bool> table;
};

#endif // _BLOOM_FILTER

/**
 * @file
 * $Id: BloomFilter.h,v 1.5 2004/03/02 09:30:19 arnetheduck Exp $
 */
