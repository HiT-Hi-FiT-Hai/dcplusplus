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

class DirectoryListing  
{
public:
	class File {
	public:
		typedef File* Ptr;
		typedef vector<Ptr> List;
		typedef List::iterator Iter;

		string name;
		LONGLONG size;

		File(const string& aName = "", LONGLONG aSize = -1) : name(aName), size(aSize) { };
	};

	class Directory {
	public:
		typedef Directory* Ptr;
		typedef list<Ptr> List;
		typedef List::iterator Iter;
		
		LONGLONG getSize() {
			LONGLONG x = 0;
			for(File::Iter i = files.begin(); i != files.end(); ++i) {
				x+=(*i)->size;
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
		
		string name;
		List directories;
		File::List files;

		Directory* parent;
		Directory(Directory* aParent = NULL, const string& aName = "") : parent(aParent), name(aName) { };
		
		~Directory() {
			for(Iter i = directories.begin(); i!=directories.end(); ++i) {
				delete *i;
			}
			for(File::Iter j = files.begin(); j!= files.end(); ++j) {
				delete *j;
			}
		}
		
	};

	LONGLONG getTotalSize() {
		return root->getTotalSize();
	}
	int getTotalFileCount() {
		return root->getTotalFileCount();
	}

	void load(string& i);
	string getPath(Directory* d);
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
 * $Id: DirectoryListing.h,v 1.3 2001/12/12 00:06:04 arnetheduck Exp $
 * @if LOG
 * $Log: DirectoryListing.h,v $
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
