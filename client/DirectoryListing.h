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

#if !defined(AFX_DIRECTORYLISTING_H__D2AF61C5_DEDE_42E0_8257_71D5AB567D39__INCLUDED_)
#define AFX_DIRECTORYLISTING_H__D2AF61C5_DEDE_42E0_8257_71D5AB567D39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DownloadManager.h"

class DirectoryListing  
{
public:
	class Directory;

	class File {
	public:
		typedef File* Ptr;
		typedef vector<Ptr> List;
		typedef List::iterator Iter;
		
		File(Directory* aDir = NULL, const string& aName = "", LONGLONG aSize = -1) throw() : parent(aDir), name(aName), size(aSize) { };

		GETSETREF(string, name, Name);
		GETSET(LONGLONG, size, Size);
		GETSET(Directory*, parent, Parent);
	};

	class Directory {
	public:
		typedef Directory* Ptr;
		typedef vector<Ptr> List;
		typedef List::iterator Iter;
		
		List directories;
		File::List files;
		
		Directory(Directory* aParent = NULL, const string& aName = "") : parent(aParent), name(aName) { };
		
		~Directory() {
			for(Iter i = directories.begin(); i!=directories.end(); ++i) {
				delete *i;
			}
			for(File::Iter j = files.begin(); j!= files.end(); ++j) {
				delete *j;
			}
		}

		LONGLONG getSize() {
			LONGLONG x = 0;
			for(File::Iter i = files.begin(); i != files.end(); ++i) {
				x+=(*i)->getSize();
			}
			return x;
		}
		LONGLONG getTotalSize() {
			LONGLONG x = getSize();
			for(Iter i = directories.begin(); i != directories.end(); ++i) {
				x += (*i)->getTotalSize();
			}
			return x;
		}

		int getFileCount() {
			return files.size();
		}
		int getTotalFileCount() {
			int x = getFileCount();
			for(Iter i = directories.begin(); i != directories.end(); ++i) {
				x += (*i)->getTotalFileCount();
			}
			return x;
		}
		
		
		GETSETREF(string, name, Name);
		GETSET(Directory*, parent, Parent);		
	};

	LONGLONG getTotalSize() {
		return root->getTotalSize();
	}
	int getTotalFileCount() {
		return root->getTotalFileCount();
	}

	void download(Directory* aDir, const string& aUser, const string& aTarget) {
		string target = aTarget + aDir->getName() + '\\';
		// First, recurse over the directories
		for(Directory::Iter j = aDir->directories.begin(); j != aDir->directories.end(); ++j) {
			download(*j, aUser, target);
		}
		// Then add the files
		for(File::Iter i = aDir->files.begin(); i != aDir->files.end(); ++i) {
			File* file = *i;
			download(file, aUser, target + file->getName());
		}
	}

	void download(File* aFile, const string& aUser, const string& aTarget) {
		DownloadManager::getInstance()->download(getPath(aFile) + aFile->getName(), aFile->getSize(), aUser, aTarget);
	}
	
	void load(string& i);
	string getPath(Directory* d);
	string getPath(File* f) { return getPath(f->getParent()); };

	Directory* getRoot() { return root; };
	DirectoryListing() {
		root = new Directory();
	};
	
	~DirectoryListing() {
		delete root;
	};
	private:
	Directory* root;
		
		
};

#endif // !defined(AFX_DIRECTORYLISTING_H__D2AF61C5_DEDE_42E0_8257_71D5AB567D39__INCLUDED_)

/**
 * @file DirectoryListing.h
 * $Id: DirectoryListing.h,v 1.5 2002/01/16 20:56:26 arnetheduck Exp $
 * @if LOG
 * $Log: DirectoryListing.h,v $
 * Revision 1.5  2002/01/16 20:56:26  arnetheduck
 * Bug fixes, file listing sort and some other small changes
 *
 * Revision 1.4  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.3  2001/12/12 00:06:04  arnetheduck
 * Updated the public hub listings, fixed some minor transfer bugs, reworked the
 * sockets to use only one thread (instead of an extra thread for sending files),
 * and fixed a major bug in the client command decoding (still have to fix this
 * one for the userconnections...)
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
