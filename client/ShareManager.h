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

class ShareManager : public Singleton<ShareManager>
{
public:
	StringList getDirectories();
	void load(SimpleXML* aXml);	
	void save(SimpleXML* aXml);
	void addDirectory(const string& aDirectory) throw(ShareException);
	void removeDirectory(const string& aDirectory);	
	string translateFileName(const string& aFile) throw(ShareException);
	void refresh(bool dirs = false, bool aUpdate = true) throw(ShareException);
	void setDirty() { dirty = true; };
	
	SearchResult::List search(const string& aString, int aSearchType, const string& aSize, int aFileType, Client* aClient) {
		return search(aString, aSearchType, Util::toInt64(aSize), aFileType, aClient);
	}
	SearchResult::List search(const string& aString, int aSearchType, LONGLONG aSize, int aFileType, Client* aClient);

	LONGLONG getShareSize() {
		Lock l(cs);
		LONGLONG tmp = 0;
		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			tmp += i->second->getSize();
		}
		return tmp;
	}
	LONGLONG getShareSize(const string& aDir) {
		Lock l(cs);
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
	string getShareSizeString() {
		return Util::toString(getShareSize());
	}
	string getShareSizeString(const string& aDir) {
		return Util::toString(getShareSize(aDir));
	}
	
	LONGLONG getListLen() { return listLen; };

	string getListLenString() {
		return Util::toString(getListLen());
	}
	
	const string& getListFile() {
		if(listFile.empty())
			listFile = Util::getAppPath() + "\\MyList.DcLst";

		return listFile;
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

		LONGLONG getSize() {
			LONGLONG tmp = size;
			for(MapIter i = directories.begin(); i != directories.end(); ++i) {
				tmp+=i->second->getSize();
			}
			return tmp;
		}

		void search(SearchResult::List& aResults, StringList& aStrings, int aSearchType, LONGLONG aSize, int aFileType, Client* aClient);
		
		Map directories;
		map<string, LONGLONG> files;

		LONGLONG size;
		
		string toString(int ident = 0);
	private:
		string name; 
		Directory* parent;
	};
		
	friend class Singleton<ShareManager>;
	ShareManager() : update(false), refreshDirs(false), listLen(0), dirty(false), refreshThread(NULL) { 
		
	};
	
	virtual ~ShareManager() {
		if(refreshThread) {
			WaitForSingleObject(refreshThread, INFINITE);
			CloseHandle(refreshThread);
		}
		
		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			delete i->second;
		}
	}
	
	Directory* buildTree(const string& aName, Directory* aParent);
	
	LONGLONG listLen;
	bool dirty;
	bool refreshDirs;
	bool update;
	
	string listFile;

	CriticalSection cs;
	HANDLE refreshThread;

	StringList files;
	bool checkFile(const string& aDir, const string& aFile);

	static DWORD WINAPI refresher(void* p);
	Directory::Map directories;
	StringMap dirs;

};

#endif // !defined(AFX_SHAREMANAGER_H__6CD5D87C_D13F_46E2_8C1E_5F116107C118__INCLUDED_)

/**
 * @file ShareManager.h
 * $Id: ShareManager.h,v 1.19 2002/02/12 00:35:37 arnetheduck Exp $
 * @if LOG
 * $Log: ShareManager.h,v $
 * Revision 1.19  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.18  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.17  2002/02/01 02:00:45  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.16  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.15  2002/01/26 14:59:24  arnetheduck
 * Fixed disconnect crash
 *
 * Revision 1.14  2002/01/26 12:06:40  arnetheduck
 * Småsaker
 *
 * Revision 1.13  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.12  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.11  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.10  2002/01/06 11:13:07  arnetheduck
 * Last fixes before 0.10
 *
 * Revision 1.9  2002/01/06 00:14:54  arnetheduck
 * Incoming searches almost done, just need some testing...
 *
 * Revision 1.8  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.7  2001/12/30 17:41:16  arnetheduck
 * Fixed some XML parsing bugs
 *
 * Revision 1.6  2001/12/30 15:03:45  arnetheduck
 * Added framework to handle incoming searches
 *
 * Revision 1.5  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.4  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.3  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.2  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.1  2001/12/02 23:51:22  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * @endif
 */

