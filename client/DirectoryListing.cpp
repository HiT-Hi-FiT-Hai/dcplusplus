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

void DirectoryListing::load(const string& in) {
	StringTokenizer t(in);

	StringList& tokens = t.getTokens();
	string::size_type ident = 0;
	
	Directory* cur = root;

	for(StringIter i = tokens.begin(); i != tokens.end(); ++i) {
		string& tok = *i;
		string::size_type j = tok.find_first_not_of('\t');
		
		if(j == string::npos)
			break;

		while(j < ident) {
			cur = cur->getParent();
			dcassert(cur != NULL);
			ident--;
		}
		string::size_type k = tok.find('|', j);
		if(k!=string::npos) {
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
	string dir;
	dir.reserve(128);
	dir.append(d->getName());
	dir.append(1, '\\');

	Directory* cur = d->getParent();
	while(cur!=root) {
		dir.insert(0, cur->getName() + '\\');
		cur = cur->getParent();
	}
	return dir;
}

void DirectoryListing::download(Directory* aDir, const User::Ptr& aUser, const string& aTarget, QueueItem::Priority p /* = QueueItem::DEFAULT */) {
	string target = (aDir == getRoot()) ? aTarget : aTarget + aDir->getName() + '\\';
	// First, recurse over the directories
	for(Directory::Iter j = aDir->directories.begin(); j != aDir->directories.end(); ++j) {
		download(*j, aUser, target, p);
	}
	// Then add the files
	for(File::Iter i = aDir->files.begin(); i != aDir->files.end(); ++i) {
		File* file = *i;
		try {
			download(file, aUser, target + file->getName(), p);
		} catch(QueueException e) {
			// Catch it here to allow parts of directories to be added...
		} catch(FileException e) {
			//..
		}
	}
}

void DirectoryListing::download(const string& aDir, const User::Ptr& aUser, const string& aTarget, QueueItem::Priority p /* = QueueItem::DEFAULT */) {
	dcassert(aDir.size() > 2);
	dcassert(aDir[aDir.size() - 1] == '\\');
	Directory* d = find(aDir, getRoot());
	if(d != NULL)
		download(d, aUser, aTarget, p);
}

DirectoryListing::Directory* DirectoryListing::find(const string& aName, Directory* current) {
	string::size_type end = aName.find('\\');
	dcassert(end != string::npos);
	string cur = aName.substr(0, end);
	for(Directory::Iter i = current->directories.begin(); i != current->directories.end(); ++i) {
		Directory* d = *i;
		if(Util::stricmp(d->getName().c_str(), cur.c_str()) == 0) {
			if(end == (aName.size() - 1))
				return d;
			else
				return find(aName.substr(end + 1), d);
		}
	}
	return NULL;
}

int64_t DirectoryListing::Directory::getTotalSize() {
	int64_t x = getSize();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		x += (*i)->getTotalSize();
	}
	return x;
}

int DirectoryListing::Directory::getTotalFileCount() {
	int x = getFileCount();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		x += (*i)->getTotalFileCount();
	}
	return x;
}

/**
 * @file DirectoryListing.cpp
 * $Id: DirectoryListing.cpp,v 1.10 2002/12/28 01:31:49 arnetheduck Exp $
 */
