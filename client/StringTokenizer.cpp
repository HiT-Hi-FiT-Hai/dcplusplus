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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "StringTokenizer.h"

StringTokenizer::StringTokenizer(const string& aString, char aToken /* = '\n' */) {
	string::size_type i = 0;
	string::size_type j = 0;
	while( (i=aString.find_first_of(aToken, j)) != string::npos ) {
		tokens.push_back(aString.substr(j, i-j));
		j = i + 1;
	}
	if(j < aString.size())
		tokens.push_back(aString.substr(j, aString.size()-j));
}

/**
 * @file StringTokenizer.cpp
 * $Id: StringTokenizer.cpp,v 1.7 2003/03/13 13:31:35 arnetheduck Exp $
 */

