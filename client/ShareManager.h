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

#if !defined(AFX_SHAREMANAGER_H__6CD5D87C_D13F_46E2_8C1E_5F116107C118__INCLUDED_)
#define AFX_SHAREMANAGER_H__6CD5D87C_D13F_46E2_8C1E_5F116107C118__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TimerManager.h"
#include "SearchManager.h"
#include "SettingsManager.h"

#include "Exception.h"
#include "CriticalSection.h"
#include "StringSearch.h"
#include "Singleton.h"

STANDARD_EXCEPTION(ShareException);

class SimpleXML;
class Client;
class File;

class ShareManager : public Singleton<ShareManager>, private SettingsManagerListener, private Thread, private TimerManagerListener
{
public:
	StringList getDirectories();
	void addDirectory(const string& aDirectory) throw(ShareException);
	void removeDirectory(const string& aDirectory);	
	string translateFileName(const string& aFile) throw(ShareException);
	void refresh(bool dirs = false, bool aUpdate = true, bool block = false) throw(ShareException);
	void setDirty() { dirty = true; };
	
	SearchResult::List search(const string& aString, int aSearchType, const string& aSize, int aFileType, Client* aClient, StringList::size_type maxResults) {
		return search(aString, aSearchType, Util::toInt64(aSize), aFileType, aClient, maxResults);
	}
	SearchResult::List search(const string& aString, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults);

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
	int64_t getBZListLen() { return bzListLen; };
	string getBZListLenString() { return Util::toString(getBZListLen()); };
	
	SearchManager::TypeModes getType(const string& fileName);
	u_int32_t getMask(const string& fileName);
	u_int32_t getMask(StringList& l);
	u_int32_t getMask(StringSearch::List& l);
	
	GETSET(u_int32_t, hits, Hits);
	GETSET(string, listFile, ListFile);
	GETSET(string, bzListFile, BZListFile);
private:
	class Directory {
	public:
		typedef Directory* Ptr;
		typedef HASH_MAP<string, Ptr> Map;
		typedef Map::iterator MapIter;
		typedef HASH_MULTIMAP<int64_t, string> DupeMap;
		typedef DupeMap::iterator DupeIter;
		typedef map<string, int64_t, noCaseStringLess> FileMap;
		typedef FileMap::iterator FileIter;

		int64_t size;
		Map directories;
		FileMap files;

		Directory(const string& aName = Util::emptyString, Directory* aParent = NULL) : 
			size(0), name(aName), parent(aParent), fileTypes(0), searchTypes(0) { 
		};

		~Directory() {
			for(MapIter i = directories.begin(); i != directories.end(); ++i)
				delete i->second;
		}

		bool hasType(u_int32_t type) throw() {
			return ( (type == SearchManager::TYPE_ANY) || (fileTypes & (1 << type)) );
		}
		void addType(u_int32_t type) throw() {
			Directory* cur = this;
			while(cur && !cur->hasType(type)) {
				cur->fileTypes |= (1 << type);
				cur = cur->getParent();
			}
		}

		bool hasSearchType(u_int32_t mask) throw() {
			return (searchTypes & mask) == mask;
		}
		void addSearchType(u_int32_t mask) throw() {
			Directory* cur = this;
			if(cur && !cur->hasSearchType(mask)) {
				searchTypes |= mask;
				cur = cur->getParent();
			}
		}
		u_int32_t getSearchTypes() throw() {
			return searchTypes;
		} 

		string getFullName() throw() {
			Directory* x = this;
			string str;
			while(x) {
				str = x->getName() + '\\' + str;
				x = x->getParent();
			}
			return str;
		}

		int64_t getSize() {
			int64_t tmp = size;
			for(MapIter i = directories.begin(); i != directories.end(); ++i) {
				tmp+=i->second->getSize();
			}
			return tmp;
		}

		void search(SearchResult::List& aResults, StringSearch::List& aStrings, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults, u_int32_t mask);
		
		void toString(string& tmp, DupeMap& dupes, int ident = 0);
		
		GETSETREF(string, name, Name);
		GETSET(Directory*, parent, Parent);
	private:
		/** Set of flags that say which SearchManager::TYPE_* a directory contains */
		u_int32_t fileTypes;
		/** Set of flags that say which common search phrases a directory contains */
		u_int32_t searchTypes;
	};
		
	friend class Singleton<ShareManager>;
	ShareManager();
	
	virtual ~ShareManager();
	
	int64_t listLen;
	int64_t bzListLen;
	bool dirty;
	bool refreshDirs;
	bool update;
	
	int listN;

	File* lFile;
	File* bFile;

	u_int32_t lastUpdate;

	/** Words that are commonly searched for. */	 
	StringList words;

	RWLock cs;

	Directory::Map directories;
	StringMap dirs;
	
	bool checkFile(const string& aDir, const string& aFile);
	Directory* buildTree(const string& aName, Directory* aParent);

	virtual int run();

	// SettingsManagerListener
	virtual void onAction(SettingsManagerListener::Types type, SimpleXML* xml) throw();
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t tick) throw();
	void load(SimpleXML* aXml);
	void save(SimpleXML* aXml);
	
};

#endif // !defined(AFX_SHAREMANAGER_H__6CD5D87C_D13F_46E2_8C1E_5F116107C118__INCLUDED_)

/**
 * @file
 * $Id: ShareManager.h,v 1.38 2003/11/10 22:42:12 arnetheduck Exp $
 */

