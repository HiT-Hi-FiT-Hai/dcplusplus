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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "StringTokenizer.h"

StringTokenizer::StringTokenizer(const string& aString, char aToken /* = '\n' */) {
	string tmp = aString;

	string::size_type i = 0;
	string::size_type j = 0;
	while( (i=tmp.find_first_of(aToken, j)) != string::npos ) {
		tokens.push_back(tmp.substr(j, i-j));
		j = i + 1;
	}
	if(j < tmp.size())
		tokens.push_back(tmp.substr(j, tmp.size()-j));
}

/**
 * @file StringTokenizer.cpp
 * $Id: StringTokenizer.cpp,v 1.4 2002/01/20 22:54:46 arnetheduck Exp $
 * @if LOG
 * $Log: StringTokenizer.cpp,v $
 * Revision 1.4  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.3  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.2  2001/12/21 23:52:30  arnetheduck
 * Last commit for five days
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */

