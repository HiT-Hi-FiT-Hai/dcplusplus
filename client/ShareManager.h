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

#if !defined(AFX_SHAREMANAGER_H__6CD5D87C_D13F_46E2_8C1E_5F116107C118__INCLUDED_)
#define AFX_SHAREMANAGER_H__6CD5D87C_D13F_46E2_8C1E_5F116107C118__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Exception.h"
#include "DirectoryListing.h"

STANDARD_EXCEPTION(ShareException);

class SimpleXML;

class ShareManager  
{
public:
	StringList getDirectories();
	void load(SimpleXML* aXml);	
	void save(SimpleXML* aXml);
	void addDirectory(const string& aDirectory) throw(ShareException);
	void removeDirectory(const string& aDirectory);	
	string translateFileName(const string& aFile) throw(ShareException);
	void refresh() throw(ShareException);
	LONGLONG getShareSize() {
		LONGLONG tmp = 0;
		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			tmp += i->second->getSize();
		}
		return tmp;
	}
	LONGLONG getShareSize(const string& aDir) {
		Directory::MapIter i = directories.find(aDir);
		if(i != directories.end()) {
			return i->second->getSize();
		}
		
		return 0;
	}
	string getShareSizeString() {
		char buf[24];
		return _i64toa(getShareSize(), buf, 10);
	}
	string getShareSizeString(const string& aDir) {
		char buf[24];
		return _i64toa(getShareSize(aDir), buf, 10);
	}
	
	LONGLONG getListLen() { return listLen; };
	string getListLenString() {
		char buf[24];
		return _i64toa(getListLen(), buf, 10);
	}
	
	static ShareManager* getInstance() {
		dcassert(instance);
		return instance;
	}

	static void newInstance() {
		if(instance)
			delete instance;
		
		instance = new ShareManager();
	}
	static void deleteInstance() {
		delete instance;
		instance = NULL;
	}
private:
	class Directory {
	public:
		typedef Directory* Ptr;
		typedef map<string, Ptr> Map;
		typedef Map::iterator MapIter;
		
		Directory(const string& aName = "", Directory* aParent = NULL) : name(aName), parent(aParent), size(0) { };
		~Directory() {
			for(MapIter i = directories.begin(); i!= directories.end(); ++i) {
				delete i->second;
			}
		}
		const string& getName() { return name; };
		void setName(const string& aName) { name = aName; };

		Directory* getParent() { return parent; };
		void setParent(Directory* aParent) { parent = aParent; };

		LONGLONG getSize() {
			LONGLONG tmp = size;
			for(MapIter i = directories.begin(); i != directories.end(); ++i) {
				tmp+=i->second->getSize();
			}
			return tmp;
		}

		Map directories;
		map<string, LONGLONG> files;

		LONGLONG size;
		
		string toString(int ident = 0);
	private:
		string name; 
		Directory* parent;
	};
		
	Directory* buildTree(const string& aName, Directory* aParent);
	
	LONGLONG listLen;

	ShareManager() : listLen(0) { };
	virtual ~ShareManager() { };
	static ShareManager* instance;
	
	Directory::Map directories;

};

#endif // !defined(AFX_SHAREMANAGER_H__6CD5D87C_D13F_46E2_8C1E_5F116107C118__INCLUDED_)

/**
 * @file ShareManager.h
 * $Id: ShareManager.h,v 1.1 2001/12/02 23:51:22 arnetheduck Exp $
 * @if LOG
 * $Log: ShareManager.h,v $
 * Revision 1.1  2001/12/02 23:51:22  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * @endif
 */

