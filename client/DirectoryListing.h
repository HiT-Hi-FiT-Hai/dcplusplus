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
			for_each(directories.begin(), directories.end(), DeleteFunction<Directory*>());
			for_each(files.begin(), files.end(), DeleteFunction<File*>());
		}

		int getTotalFileCount();		
		int64_t getTotalSize();
		
		int getFileCount() { return files.size(); };
		
		int64_t getSize() {
			int64_t x = 0;
			for(File::Iter i = files.begin(); i != files.end(); ++i) {
				x+=(*i)->getSize();
			}
			return x;
		}
		
		GETSETREF(string, name, Name);
		GETSET(Directory*, parent, Parent);		
	};

	DirectoryListing() {
		root = new Directory();
	};
	
	~DirectoryListing() {
		delete root;
	};

	void download(const string& aDir, const User::Ptr& aUser, const string& aTarget, QueueItem::Priority p = QueueItem::DEFAULT);
	void download(Directory* aDir, const User::Ptr& aUser, const string& aTarget, QueueItem::Priority p = QueueItem::DEFAULT);
	void load(const string& i);
	string getPath(Directory* d);
	
	string getPath(File* f) { return getPath(f->getParent()); };
	int64_t getTotalSize() { return root->getTotalSize(); };
	int getTotalFileCount() { return root->getTotalFileCount(); };
	Directory* getRoot() { return root; };
	
	void download(File* aFile, const User::Ptr& aUser, const string& aTarget, QueueItem::Priority p = QueueItem::DEFAULT) {
		QueueManager::getInstance()->add(getPath(aFile) + aFile->getName(), aFile->getSize(), aUser, aTarget, true, p);
	}

private:
	Directory* root;
		
	Directory* find(const string& aName, Directory* current);
		
};

#endif // !defined(AFX_DIRECTORYLISTING_H__D2AF61C5_DEDE_42E0_8257_71D5AB567D39__INCLUDED_)

/**
 * @file DirectoryListing.h
 * $Id: DirectoryListing.h,v 1.13 2002/12/28 01:31:49 arnetheduck Exp $
 */
