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

#ifndef STRINGSEARCH_H
#define STRINGSEARCH_H

/**
 * A class that implements a fast substring search algo suited for matching
 * one pattern against many strings (currently Quick Search, a variant of
 * Boyer-Moore. Code based on "A very fast substring search algorithm" by 
 * D. Sunday).
 * @todo Perhaps find an algo suitable for matching multiple substrings.
 */
class StringSearch {
public:
	typedef vector<StringSearch> List;
	typedef List::iterator Iter;

	explicit StringSearch(const string& aPattern) throw() : pattern(Util::toLower(aPattern)) { 
		initDelta1();
	};
	StringSearch(const StringSearch& rhs) throw() : pattern(rhs.pattern) { 
		memcpy(delta1, rhs.delta1, sizeof(delta1));
	};
	const StringSearch& operator=(const StringSearch& rhs) {
		memcpy(delta1, rhs.delta1, sizeof(delta1));
		pattern = rhs.pattern;
		return *this;
	}
	const StringSearch& operator=(const string& rhs) {
		pattern = Util::toLower(rhs);
		initDelta1();
		return *this;
	}

	bool operator==(const StringSearch& rhs) { return pattern == rhs.pattern; };

	const string& getPattern() const { return pattern; };

	/** Match a text against the pattern */
	bool match(const string& aText) const throw() {
		// u_int8_t to avoid problems with signed char pointer arithmetic
		u_int8_t *tx = (u_int8_t*)aText.c_str();
		u_int8_t *px = (u_int8_t*)pattern.c_str();

		string::size_type plen = pattern.length();

		if(aText.length() < plen) {
			return false;
		}

		u_int8_t *end = tx + aText.length() - plen + 1;
		while(tx < end) {
			size_t i = 0;
			for(; px[i] && (px[i] == Util::toLower(tx[i])); ++i)
				;		// Empty!
			
			if(px[i] == 0) 
				return true;

			tx += delta1[Util::toLower(tx[plen])];
		}

		return false;
	}

private:
	enum { ASIZE = 256 };
	/** 
	 * Delta1 shift, u_int16_t because we expect all patterns to be shorter than 2^16
	 * chars.
	 */
	u_int16_t delta1[ASIZE];
	string pattern;

	void initDelta1() {
		u_int16_t x = (u_int16_t)(pattern.length() + 1);
		u_int16_t i;
		for(i = 0; i < ASIZE; ++i) {
			delta1[i] = x;
		}
		// x = pattern.length();
		x--;
		u_int8_t* p = (u_int8_t*)pattern.data();
		for(i = 0; i < x; ++i) {
			delta1[p[i]] = (u_int16_t)(x - i);
		}
	}
};

#endif STRINGSEARCH_H
/**
 * @file
 * $Id: StringSearch.h,v 1.4 2003/05/28 11:53:04 arnetheduck Exp $
 */
