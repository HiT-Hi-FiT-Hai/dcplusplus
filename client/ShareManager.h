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

#if !defined(AFX_SHAREMANAGER_H__6CD5D87C_D13F_46E2_8C1E_5F116107C118__INCLUDED_)
#define AFX_SHAREMANAGER_H__6CD5D87C_D13F_46E2_8C1E_5F116107C118__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Exception.h"
#include "CriticalSection.h"
#include "Util.h"
#include "SearchManager.h"

STANDARD_EXCEPTION(ShareException);

class SimpleXML;
class Client;

class ShareManager : public Singleton<ShareManager>, private SettingsManagerListener, private Thread
{
public:
	StringList getDirectories();
	void addDirectory(const string& aDirectory) throw(ShareException);
	void removeDirectory(const string& aDirectory);	
	string translateFileName(const string& aFile) throw(ShareException);
	void refresh(bool dirs = false, bool aUpdate = true) throw(ShareException);
	void setDirty() { dirty = true; };
	
	SearchResult::List search(const string& aString, int aSearchType, const string& aSize, int aFileType, Client* aClient) {
		return search(aString, aSearchType, Util::toInt64(aSize), aFileType, aClient);
	}
	SearchResult::List search(const string& aString, int aSearchType, int64_t aSize, int aFileType, Client* aClient);

	int64_t getShareSize() {
		RLock l(cs);
		int64_t tmp = 0;
		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			tmp += i->second->getSize();
		}
		return tmp;
	}
	int64_t getShareSize(const string& aDir) {
		RLock l(cs);
		dcassert(aDir.size()>0);
		Directory::MapIter i;
		if(aDir[aDir.size()-1] =='\\')
			i = directories.find(aDir.substr(0, aDir.size()-1));
		else
			i = directories.find(aDir);

		if(i != directories.end()) {
			return i->second->getSize();
		}
		
		return -1;
	}
	string getShareSizeString() { return Util::toString(getShareSize()); };
	string getShareSizeString(const string& aDir) { return Util::toString(getShareSize(aDir)); };
	
	int64_t getListLen() { return listLen; };
	string getListLenString() { return Util::toString(getListLen()); };
	
	const string& getListFile() {
		if(listFile.empty())
			listFile = Util::getAppPath() + "\\MyList.DcLst";

		return listFile;
	}
private:
	class Directory {
	public:
		typedef Directory* Ptr;
		typedef HASH_MAP<string, Ptr> Map;
		typedef Map::iterator MapIter;
		typedef HASH_MULTIMAP<int64_t, string> DupeMap;
		typedef DupeMap::iterator DupeIter;
		typedef HASH_MAP<string, int64_t> FileMap;
		typedef FileMap::iterator FileIter;

		int64_t size;
		Map directories;
		FileMap files;
		
		Directory(const string& aName = Util::emptyString, Directory* aParent = NULL) : size(0), name(aName), parent(aParent) { 
		};

		~Directory() {
			for(MapIter i = directories.begin(); i!= directories.end(); ++i) {
				delete i->second;
			}
		}
		string getFullName() {
			Directory* x = this;
			string str;
			while(x) {
				str = x->getName() + '\\' + str;
				x = x->getParent();
			}
			return str;
		}
		const string& getName() { return name; };
		void setName(const string& aName) { name = aName; };

		Directory* getParent() { return parent; };
		void setParent(Directory* aParent) { parent = aParent; };

		int64_t getSize() {
			int64_t tmp = size;
			for(MapIter i = directories.begin(); i != directories.end(); ++i) {
				tmp+=i->second->getSize();
			}
			return tmp;
		}

		void search(SearchResult::List& aResults, StringList& aStrings, int aSearchType, int64_t aSize, int aFileType, Client* aClient);
		
		string toString(DupeMap& dupes, int ident = 0);
	private:
		string name; 
		Directory* parent;
	};
		
	friend class Singleton<ShareManager>;
	ShareManager() : listLen(0), dirty(false), refreshDirs(false), update(false) { 
		SettingsManager::getInstance()->addListener(this);
	};
	
	virtual ~ShareManager() {
		SettingsManager::getInstance()->removeListener(this);
		
		join();

		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			delete i->second;
		}
	}
	
	int64_t listLen;
	bool dirty;
	bool refreshDirs;
	bool update;
	
	string listFile;

	RWLock cs;

	Directory::Map directories;
	StringMap dirs;
	
	bool checkFile(const string& aDir, const string& aFile);
	Directory* buildTree(const string& aName, Directory* aParent);

	virtual int run();

	// SettingsManagerListener
	virtual void onAction(SettingsManagerListener::Types type, SimpleXML* xml) {
		switch(type) {
		case SettingsManagerListener::LOAD: load(xml); break;
		case SettingsManagerListener::SAVE: save(xml); break;
		}
	}
	
	void load(SimpleXML* aXml);
	void save(SimpleXML* aXml);
	
};

#endif // !defined(AFX_SHAREMANAGER_H__6CD5D87C_D13F_46E2_8C1E_5F116107C118__INCLUDED_)

/**
 * @file ShareManager.h
 * $Id: ShareManager.h,v 1.24 2002/04/13 12:57:23 arnetheduck Exp $
 */

