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

#include "DirectoryListing.h"
#include "StringTokenizer.h"

void DirectoryListing::load(string& in) {
	StringTokenizer t(in);

	StringList& tokens = t.getTokens();
	int ident = 0;
	
	Directory* cur = root;

	for(StringIter i = tokens.begin(); i != tokens.end(); ++i) {
		string& tok = *i;
		int j = tok.find_first_not_of('\t');
		while(j < ident) {
			cur = cur->getParent();
			dcassert(cur != NULL);
			ident--;
		}
		string::size_type k = tok.find('|', j);
		if( k!=string::npos) {
			// this must be a file...
			cur->files.push_back(new File(cur, tok.substr(j, k-j), Util::toInt64(tok.substr(k+1))));
		} else {
			// A directory
			Directory* d = new Directory(cur, tok.substr(j, tok.length()-j-1));
			cur->directories.push_back(d);
			cur = d;
			ident++;
		}
	}
}


string DirectoryListing::getPath(Directory* d) {
	string dir = d->getName()+"\\";
	Directory* cur = d->getParent();
	while(cur!=root) {
		dir = cur->getName() +"\\" + dir;
		cur = cur->getParent();
	}
	return dir;
}

/**
 * @file DirectoryListing.cpp
 * $Id: DirectoryListing.cpp,v 1.9 2002/04/13 12:57:22 arnetheduck Exp $
 */
