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

#include "stdinc.h"
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
 * $Id: StringTokenizer.cpp,v 1.6 2002/04/13 12:57:23 arnetheduck Exp $
 */

