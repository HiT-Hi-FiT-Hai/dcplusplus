/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

class BitInputStream  
{
public:
	BitInputStream(const string& aStream, int aStart) : is(aStream), bitPos(aStart*8) { };
	~BitInputStream() { };
	
	bool get() {
		bool ret = (((BYTE)is[bitPos>>3]) >> (bitPos&0x07)) & 0x01;
		bitPos++;
		return ret;
	}
	
	void skipToByte() {
		bitPos = ((bitPos>>3)+1)<<3;
	}
	
	void skip(int n) {
		bitPos += n * 8;
		return ;
	}
private:
	int bitPos;
	const string& is;
};

#endif // !defined(AFX_BITINPUTSTREAM_H__EAF695A9_6D5C_4791_88A2_3FA0D47697AF__INCLUDED_)

/**
 * @file BitInputStream.h
 * $Id: BitInputStream.h,v 1.2 2001/12/03 20:52:19 arnetheduck Exp $
 * @if LOG
 * $Log: BitInputStream.h,v $
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
