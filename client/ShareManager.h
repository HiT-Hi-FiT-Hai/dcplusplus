/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#include "HashManager.h"
#include "DownloadManager.h"

#include "Exception.h"
#include "CriticalSection.h"
#include "StringSearch.h"
#include "Singleton.h"
#include "BloomFilter.h"
#include "FastAlloc.h"
#include "MerkleTree.h"

STANDARD_EXCEPTION(ShareException);

class SimpleXML;
class Client;
class File;
class OutputStream;

class ShareManager : public Singleton<ShareManager>, private SettingsManagerListener, private Thread, private TimerManagerListener,
	private HashManagerListener, private DownloadManagerListener
{
public:
	/**
	 * @param aDirectory Physical directory localtion
	 * @param aName Virtual name
	 */
	void addDirectory(const string& aDirectory, const string & aName) throw(ShareException);
	void removeDirectory(const string& aName);	
	string translateFileName(const string& aFile, bool adc, bool utf8) throw(ShareException);
	void refresh(bool dirs = false, bool aUpdate = true, bool block = false) throw(ShareException);
	void setDirty() { xmlDirty = nmdcDirty = true; };

	void search(SearchResult::List& l, const string& aString, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults);
	void search(SearchResult::List& l, const StringList& params, Client* aClient, StringList::size_type maxResults);

	StringPairList getDirectories() const { RLock l(cs); return virtualMap; }

	int64_t getShareSize() throw();
	int64_t getShareSize(const string& aDir) throw();

	string getShareSizeString() { return Util::toString(getShareSize()); };
	string getShareSizeString(const string& aDir) { return Util::toString(getShareSize(aDir)); };
	
	int64_t getListLen() { return listLen; };
	string getListLenString() { return Util::toString(getListLen()); };
	
	SearchManager::TypeModes getType(const string& fileName);
	u_int32_t getMask(const string& fileName);
	u_int32_t getMask(StringList& l);
	u_int32_t getMask(StringSearch::List& l);

	void addHits(u_int32_t aHits) {
		hits += aHits;
	}
	
	GETSET(u_int32_t, hits, Hits);
	GETSET(string, listFile, ListFile);
	GETSET(string, bzXmlFile, BZXmlFile);

private:
	struct AdcSearch;

	class Directory : public FastAlloc<Directory> {
	public:
		struct File {
			struct StringComp {
				StringComp(const string& s) : a(s) { }
				bool operator()(const File& b) const { return Util::stricmp(a, b.getName()) == 0; }
				const string& a;
			};
			struct FileLess {
				int operator()(const File& a, const File& b) const { return Util::stricmp(a.getName(), b.getName()); }
			};
			typedef set<File, FileLess> Set;
			typedef Set::iterator Iter;

			File() : size(0), parent(NULL), tth(NULL) { };
			File(const string& aName, int64_t aSize, Directory* aParent, TTHValue* aRoot) : 
			    name(aName), size(aSize), parent(aParent), tth(aRoot) { };

			string getADCPath() const { return parent->getADCPath() + name; }
			string getFullName() const { return parent->getFullName() + getName(); }

			GETSET(string, name, Name);
			GETSET(int64_t, size, Size);
			GETSET(Directory*, parent, Parent);
			GETSET(TTHValue*, tth, TTH);
		};

		typedef Directory* Ptr;
		typedef HASH_MAP<string, Ptr> Map;
		typedef Map::iterator MapIter;

		int64_t size;
		Map directories;
		File::Set files;

		Directory(const string& aName = Util::emptyString, Directory* aParent = NULL) : 
			size(0), name(aName), parent(aParent), fileTypes(0), searchTypes(0) { 
		};

		~Directory();

		bool hasType(u_int32_t type) const throw() {
			return ( (type == SearchManager::TYPE_ANY) || (fileTypes & (1 << type)) );
		}
		void addType(u_int32_t type) throw();
		bool hasSearchType(u_int32_t mask) const throw() {
			return (searchTypes & mask) == mask;
		}
		void addSearchType(u_int32_t mask) throw();
		u_int32_t getSearchTypes() throw() {
			return searchTypes;
		}

		string getADCPath() const throw();
		string getFullName() const throw(); 

		int64_t getSize() {
			int64_t tmp = size;
			for(MapIter i = directories.begin(); i != directories.end(); ++i) {
				tmp+=i->second->getSize();
			}
			return tmp;
		}

		void search(SearchResult::List& aResults, StringSearch::List& aStrings, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults, u_int32_t mask) throw();
		void search(SearchResult::List& aResults, AdcSearch& aStrings, Client* aClient, StringList::size_type maxResults, u_int32_t mask) throw();

		void toNmdc(string& nmdc, string& indent, string& tmp2);
		void toXml(OutputStream& xmlFile, string& indent, string& tmp2);

		GETSET(string, name, Name);
		GETSET(Directory*, parent, Parent);
	private:
		/** Set of flags that say which SearchManager::TYPE_* a directory contains */
		u_int32_t fileTypes;
		/** Set of flags that say which common search phrases a directory contains */
		u_int32_t searchTypes;
	};
	friend class Directory;

	friend class Singleton<ShareManager>;
	ShareManager();
	
	virtual ~ShareManager();
	
	StringPairList loadDirs;

	struct AdcSearch {
		AdcSearch(const StringList& params);

		bool isExcluded(const string& str) {
			for(StringSearch::Iter i = exclude.begin(); i != exclude.end(); ++i) {
				if(i->match(str))
					return true;
			}
			return false;
		}

		bool hasExt(const string& name) {
			for(StringIter i = ext.begin(); i != ext.end(); ++i) {
				if(name.length() >= i->length() && Util::stricmp(name.c_str() + name.length() - i->length(), i->c_str()) == 0)
					return true;
			}
			return false;
		}

		StringSearch::List* include;
		StringSearch::List includeX;
		StringSearch::List exclude;
		StringList ext;

		int64_t gt;
		int64_t lt;

		TTHValue root;
		bool hasRoot;

		bool isDirectory;
	};

	typedef HASH_MULTIMAP_X(TTHValue::Ptr, Directory::File::Iter, TTHValue::PtrHash, TTHValue::PtrHash, TTHValue::PtrLess) HashFileMap;
	typedef HashFileMap::iterator HashFileIter;

	HashFileMap tthIndex;

	int64_t listLen;
	int64_t bzXmlListLen;
	bool xmlDirty;
	bool nmdcDirty;
	bool refreshDirs;
	bool update;
	
	int listN;

	File* lFile;
	File* xFile;

	u_int32_t lastXmlUpdate;
	u_int32_t lastNmdcUpdate;
	u_int32_t lastFullUpdate;

	/** Words that are commonly searched for. */	 
	StringList words;

	mutable RWLock cs;

	// Map real name to directory structure
	Directory::Map directories;

	// Map virtual to real dir name
	StringPairList virtualMap;

	BloomFilter<5> bloom;
	
	StringPairIter findVirtual(const string& name);

	bool checkFile(const string& aDir, const string& aFile);
	Directory* buildTree(const string& aName, Directory* aParent);
	void addTree(const string& aName, Directory* aDirectory);
	void generateNmdcList();
	void generateXmlList();

	void removeTTH(TTHValue* tth, const Directory::File::Iter&);

	Directory* getDirectory(const string& fname);

	virtual int run();

	// DownloadManagerListener
	virtual void on(DownloadManagerListener::Complete, Download* d) throw();

	// HashManagerListener
	virtual void on(HashManagerListener::TTHDone, const string& fname, TTHValue* root) throw();

	// SettingsManagerListener
	virtual void on(SettingsManagerListener::Save, SimpleXML* xml) throw() {
		save(xml);
	}
	virtual void on(SettingsManagerListener::Load, SimpleXML* xml) throw() {
		load(xml);
	}
	
	// TimerManagerListener
	virtual void on(TimerManagerListener::Minute, u_int32_t tick) throw();
	void load(SimpleXML* aXml);
	void save(SimpleXML* aXml);
	
};

#endif // !defined(AFX_SHAREMANAGER_H__6CD5D87C_D13F_46E2_8C1E_5F116107C118__INCLUDED_)

/**
 * @file
 * $Id: ShareManager.h,v 1.53 2004/09/06 12:32:42 arnetheduck Exp $
 */

