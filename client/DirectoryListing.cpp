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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "DirectoryListing.h"
#include "StringTokenizer.h"

void DirectoryListing::load(string& in) {
	StringTokenizer t(in);

	StringList& tokens = t.getTokens();
	int ident = 0;
	
	Directory* cur = root;

	for(StringIter i = tokens.begin(); i != tokens.end(); ++i) {
		string tok = *i;
		int j = tok.find_first_not_of('\t');
		while(j < ident) {
			cur = cur->parent;
			dcassert(cur != NULL);
			ident--;
		}
		if( tok.find('|')!=string::npos) {
			// this must be a file...
			tok = tok.substr(j);
			j = tok.find('|');
			cur->files.push_back(new File(cur, tok.substr(0, j), _atoi64(tok.substr(j+1).c_str())));
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
	string dir = d->name+"\\";
	Directory* cur = d->parent;
	while(cur!=root) {
		dir = cur->name +"\\" + dir;
		cur = cur->parent;
	}
	return dir;
}

/**
 * @file DirectoryListing.cpp
 * $Id: DirectoryListing.cpp,v 1.3 2001/12/13 19:21:57 arnetheduck Exp $
 * @if LOG
 * $Log: DirectoryListing.cpp,v $
 * Revision 1.3  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.2  2001/11/29 19:10:54  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.1  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * @endif
 */
