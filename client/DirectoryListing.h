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

#if !defined(AFX_DIRECTORYLISTING_H__D2AF61C5_DEDE_42E0_8257_71D5AB567D39__INCLUDED_)
#define AFX_DIRECTORYLISTING_H__D2AF61C5_DEDE_42E0_8257_71D5AB567D39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "User.h"
#include "FastAlloc.h"

class DirectoryListing  
{
public:
	class Directory;

	/** Auxiliary class to ease searching in File::List and Directory::List */
	struct Name {
		Name(const string& aName) : name(aName) { };
		virtual ~Name() { };

		GETSETREF(string, name, Name);
	};

	class File : public Name, public FastAlloc<File> {
	public:
		typedef File* Ptr;
		struct FileSort {
			bool operator()(const Ptr& a, const Ptr& b) const {
				return Util::stricmp(a->getName().c_str(), b->getName().c_str()) == -1;
			}
		};
		typedef set<Ptr, FileSort> List;
		typedef List::iterator Iter;
		
		File(Directory* aDir = NULL, const string& aName = Util::emptyString, int64_t aSize = -1) throw() : Name(aName), size(aSize), parent(aDir), adls(false) { };

		GETSET(int64_t, size, Size);
		GETSET(Directory*, parent, Parent);
		GETSET(bool, adls, Adls);
	};

	class Directory : public Name, public FastAlloc<Directory> {
	public:
		typedef Directory* Ptr;
		struct DirSort {
			bool operator()(const Ptr& a, const Ptr& b) const {
				return Util::stricmp(a->getName().c_str(), b->getName().c_str()) == -1;
			}
		};
		typedef set<Ptr, DirSort> List;
		typedef List::iterator Iter;
		
		List directories;
		File::List files;
		
		Directory(Directory* aParent = NULL, const string& aName = Util::emptyString, bool _adls = false) 
			: Name(aName), parent(aParent), adls(_adls) { };
		
		virtual ~Directory() {
			for_each(directories.begin(), directories.end(), DeleteFunction<Directory*>());
			for_each(files.begin(), files.end(), DeleteFunction<File*>());
		}

		int getTotalFileCount(bool adls = false);		
		int64_t getTotalSize(bool adls = false);
		
		int getFileCount() { return files.size(); };
		
		int64_t getSize() {
			int64_t x = 0;
			for(File::Iter i = files.begin(); i != files.end(); ++i) {
				x+=(*i)->getSize();
			}
			return x;
		}
		
		GETSET(Directory*, parent, Parent);		
		GETSET(bool, adls, Adls);
	private:
		Directory(const Directory&);
		Directory& operator=(const Directory&);
	};
	class AdlDirectory : public Directory {
	public:
		AdlDirectory(const string& aFullPath, Directory* aParent, const string& aName) : Directory(aParent, aName, true), fullPath(aFullPath) { };

		GETSETREF(string, fullPath, FullPath);
	};

	DirectoryListing(const User::Ptr& aUser) : user(aUser), root(new Directory()) {
	};
	
	~DirectoryListing() {
		delete root;
	};

	void download(const string& aDir, const string& aTarget);
	void download(Directory* aDir, const string& aTarget);
	void load(const string& i);
	string getPath(Directory* d);
	
	string getPath(File* f) { return getPath(f->getParent()); };
	int64_t getTotalSize(bool adls = false) { return root->getTotalSize(adls); };
	int getTotalFileCount(bool adls = false) { return root->getTotalFileCount(adls); };
	Directory* getRoot() { return root; };
	
	void download(File* aFile, const string& aTarget, bool view = false);

	GETSETREF(User::Ptr, user, User);

private:
	DirectoryListing(const DirectoryListing&);
	DirectoryListing& operator=(const DirectoryListing&);

	Directory* root;
		
	Directory* find(const string& aName, Directory* current);
		
};

#endif // !defined(AFX_DIRECTORYLISTING_H__D2AF61C5_DEDE_42E0_8257_71D5AB567D39__INCLUDED_)

/**
 * @file
 * $Id: DirectoryListing.h,v 1.21 2004/01/28 19:37:54 arnetheduck Exp $
 */
