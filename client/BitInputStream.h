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

#if !defined(AFX_BITINPUTSTREAM_H__EAF695A9_6D5C_4791_88A2_3FA0D47697AF__INCLUDED_)
#define AFX_BITINPUTSTREAM_H__EAF695A9_6D5C_4791_88A2_3FA0D47697AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * A clumsy bit streamer, assumes that there's enough data to complete the operations.
 * No, doesn't operate on streams...=)
 */
class BitInputStream  
{
public:
	BitInputStream(const u_int8_t* aStream, int aStart) : bitPos(aStart*8), is(aStream) { };
	~BitInputStream() { };
	
	bool get() {
		bool ret = (((u_int8_t)is[bitPos>>3]) >> (bitPos&0x07)) & 0x01;
		bitPos++;
		return ret;
	}
	
	void skipToByte() {
		if(bitPos%8 != 0)
			bitPos = (bitPos & (~7)) + 8;
	}
	
	void skip(int n) {
		bitPos += n * 8;
		return ;
	}
private:
	int bitPos;
	const u_int8_t* is;
};

#endif // !defined(AFX_BITINPUTSTREAM_H__EAF695A9_6D5C_4791_88A2_3FA0D47697AF__INCLUDED_)

/**
 * @file BitInputStream.h
 * $Id: BitInputStream.h,v 1.7 2002/04/09 18:43:27 arnetheduck Exp $
 * @if LOG
 * $Log: BitInputStream.h,v $
 * Revision 1.7  2002/04/09 18:43:27  arnetheduck
 * Major code reorganization, to ease maintenance and future port...
 *
 * Revision 1.6  2002/01/20 22:54:45  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.5  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.4  2001/12/07 20:02:58  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.3  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.2  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
 *
 * Revision 1.1  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * @endif
 */
