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

#if !defined(AFX_STRINGTOKENIZER_H__E9B493AC_97A7_4A18_AF7C_06BFE1926A52__INCLUDED_)
#define AFX_STRINGTOKENIZER_H__E9B493AC_97A7_4A18_AF7C_06BFE1926A52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class StringTokenizer  
{
private:
	StringList tokens;
public:
	StringTokenizer(const string& aString, char aToken = '\n');
	StringList& getTokens() { return tokens; };
	~StringTokenizer() { };

};

#endif // !defined(AFX_STRINGTOKENIZER_H__E9B493AC_97A7_4A18_AF7C_06BFE1926A52__INCLUDED_)

/**
 * @file
 * $Id: StringTokenizer.h,v 1.5 2003/04/15 10:13:57 arnetheduck Exp $
 */
