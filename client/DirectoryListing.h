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

#if !defined(AFX_DIRECTORYLISTING_H__D2AF61C5_DEDE_42E0_8257_71D5AB567D39__INCLUDED_)
#define AFX_DIRECTORYLISTING_H__D2AF61C5_DEDE_42E0_8257_71D5AB567D39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "QueueManager.h"

class DirectoryListing  
{
public:
	class Directory;

	class File {
	public:
		typedef File* Ptr;
		typedef vector<Ptr> List;
		typedef List::iterator Iter;
		
		File(Directory* aDir = NULL, const string& aName = Util::emptyString, int64_t aSize = -1) throw() : name(aName), size(aSize), parent(aDir) { };

		GETSETREF(string, name, Name);
		GETSET(int64_t, size, Size);
		GETSET(Directory*, parent, Parent);
	};

	class Directory {
	public:
		typedef Directory* Ptr;
		typedef vector<Ptr> List;
		typedef List::iterator Iter;
		
		List directories;
		File::List files;
		
		Directory(Directory* aParent = NULL, const string& aName = Util::emptyString) : name(aName), parent(aParent) { };
		
		~Directory() {
			for(Iter i = directories.begin(); i!=directories.end(); ++i) {
				delete *i;
			}
			for(File::Iter j = files.begin(); j!= files.end(); ++j) {
				delete *j;
			}
		}

		int64_t getSize() {
			int64_t x = 0;
			for(File::Iter i = files.begin(); i != files.end(); ++i) {
				x+=(*i)->getSize();
			}
			return x;
		}
		int64_t getTotalSize() {
			int64_t x = getSize();
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

	int64_t getTotalSize() {
		return root->getTotalSize();
	}
	int getTotalFileCount() {
		return root->getTotalFileCount();
	}

	void download(Directory* aDir, const User::Ptr& aUser, const string& aTarget) {
		string target = aTarget + aDir->getName() + '\\';
		// First, recurse over the directories
		for(Directory::Iter j = aDir->directories.begin(); j != aDir->directories.end(); ++j) {
				download(*j, aUser, target);
		}
		// Then add the files
		for(File::Iter i = aDir->files.begin(); i != aDir->files.end(); ++i) {
			File* file = *i;
			try {
				download(file, aUser, target + file->getName());
			} catch(QueueException e) {
				// Catch it here to allow parts of directories to be added...
			} catch(FileException e) {
				//..
			}
		}
	}
	
	void download(File* aFile, const User::Ptr& aUser, const string& aTarget) {
		QueueManager::getInstance()->add(getPath(aFile) + aFile->getName(), aFile->getSize(), aUser, aTarget);
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
 * $Id: DirectoryListing.h,v 1.11 2002/04/13 12:57:22 arnetheduck Exp $
 */
