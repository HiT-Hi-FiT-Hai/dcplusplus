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
	string& is;
	int bitPos;
	u_int8_t next;
};

#endif // !defined(AFX_BITINPUTSTREAM_H__EAF695A9_6D5C_4791_88A2_3FA0D47697AF__INCLUDED_)

/**
 * @file BitOuputStream.h
 * $Id: BitOutputStream.h,v 1.7 2002/04/09 18:43:27 arnetheduck Exp $
 * @if LOG
 * $Log: BitOutputStream.h,v $
 * Revision 1.7  2002/04/09 18:43:27  arnetheduck
 * Major code reorganization, to ease maintenance and future port...
 *
 * Revision 1.6  2002/01/20 22:54:45  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.5  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.4  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.3  2001/12/07 20:02:59  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.2  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
 *
 * Revision 1.1  2001/12/01 17:17:22  arnetheduck
 * New additions to the reworked connection manager and huffman encoder
 *
 * @endif
 */
