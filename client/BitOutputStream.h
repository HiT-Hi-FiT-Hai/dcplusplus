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

#if !defined(AFX_BITOUTPUTSTREAM_H__EAF695A9_6D5C_4791_88A2_3FA0D47697AF__INCLUDED_)
#define AFX_BITOUTPUTSTREAM_H__EAF695A9_6D5C_4791_88A2_3FA0D47697AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class BitOutputStream  
{
public:
	BitOutputStream(string& aStream) : is(aStream), bitPos(0), next(0) { };
	~BitOutputStream() { };
	
	void put(vector<u_int8_t>& b) {
		for(vector<u_int8_t>::iterator i = b.begin(); i != b.end(); ++i) {
			next |=  (*i) << bitPos++;
			
			if(bitPos > 7) {
				bitPos-=8;
				is += next;
				next = 0;
			}
			
		}
	}
	
	void skipToByte() {
		if(bitPos > 0) {
			bitPos = 0;
			is += next;
			next = 0;
		}
	}
	
private:
	BitOutputStream& operator=(const BitOutputStream&);
	string& is;
	int bitPos;
	u_int8_t next;
};

#endif // !defined(AFX_BITINPUTSTREAM_H__EAF695A9_6D5C_4791_88A2_3FA0D47697AF__INCLUDED_)

/**
 * @file
 * $Id: BitOutputStream.h,v 1.13 2004/12/05 16:19:18 arnetheduck Exp $
 */
