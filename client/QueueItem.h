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

#if !defined(AFX_QUEUEITEM_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)
#define AFX_QUEUEITEM_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class QueueManager;
class Download;

#include "User.h"
#include "FastAlloc.h"
#include "MerkleTree.h"

class QueueItem : public Flags, public FastAlloc<QueueItem> {
public:
	typedef QueueItem* Ptr;
	// Strange, the vc7 optimizer won't take a deque here...
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	typedef map<string, Ptr, noCaseStringLess> StringMap;
	//	typedef HASH_MAP<string, Ptr, noCaseStringHash, noCaseStringEq> StringMap;
	typedef StringMap::iterator StringIter;
	typedef HASH_MAP_X(User::Ptr, Ptr, User::HashFunction, equal_to<User::Ptr>, less<User::Ptr>) UserMap;
	typedef UserMap::iterator UserIter;
	typedef HASH_MAP_X(User::Ptr, List, User::HashFunction, equal_to<User::Ptr>, less<User::Ptr>) UserListMap;
	typedef UserListMap::iterator UserListIter;

	enum Status {
		/** The queue item is waiting to be downloaded and can be found in userQueue */
		STATUS_WAITING,
		/** This item is being downloaded and can be found in running */
		STATUS_RUNNING
	};

	enum Priority {
		DEFAULT = -1,
		PAUSED = 0,
		LOWEST,
		LOW,
		NORMAL,
		HIGH,
		HIGHEST,
		LAST
	};

	enum FileFlags {
		/** Normal download, no flags set */
		FLAG_NORMAL = 0x00, 
		/** This download should be resumed if possible */
		FLAG_RESUME = 0x01,
		/** This is a user file listing download */
		FLAG_USER_LIST = 0x02,
		/** The file list is downloaded to use for directory download (used with USER_LIST) */
		FLAG_DIRECTORY_DOWNLOAD = 0x04,
		/** The file is downloaded to be viewed in the gui */
		FLAG_CLIENT_VIEW = 0x08,
		/** The file list downloaded was actually BZ compressed (MyList.bz2, only available in FINISHED message) */
		FLAG_BZLIST = 0x10,
		/** Flag to indicate that file should be viewed as a text file */
		FLAG_TEXT = 0x20,
		/** This file exists on the hard disk and should be prioritised */
		FLAG_EXISTS = 0x40,
		/** Match the queue against this list */
		FLAG_MATCH_QUEUE = 0x80,
		/** The source being added has its filename in utf-8 */
		FLAG_SOURCE_UTF8 = 0x100,
		/** The file list downloaded was actually an .xml.bz2 list */
		FLAG_XML_BZLIST = 0x200,
	};

	class Source : public Flags, public FastAlloc<Source> {
	public:
		typedef Source* Ptr;
		typedef vector<Ptr> List;
		typedef List::iterator Iter;
		enum {
			FLAG_FILE_NOT_AVAILABLE = 0x01,
			FLAG_ROLLBACK_INCONSISTENCY = 0x02,
			FLAG_PASSIVE = 0x04,
			FLAG_REMOVED = 0x08,
			FLAG_CRC_FAILED = 0x10,
			FLAG_CRC_WARN = 0x20,
			FLAG_UTF8 = 0x40,
		};

		Source(const User::Ptr& aUser, const string& aPath) : path(aPath), user(aUser) { };
		Source(const Source& aSource) : Flags(aSource), path(aSource.path), user(aSource.user) { }

		User::Ptr& getUser() { return user; };
		const User::Ptr& getUser() const { return user; };
		void setUser(const User::Ptr& aUser) { user = aUser; };
		string getFileName() { return Util::getFileName(path); };

		GETSETREF(string, path, Path);
	private:
		User::Ptr user;
	};

	QueueItem(const string& aTarget, int64_t aSize, const string& aSearchString, 
		Priority aPriority, int aFlag, int64_t aDownloadedBytes, u_int32_t aAdded, const TTHValue* tth) : 
	Flags(aFlag), target(aTarget), searchString(aSearchString), 
		size(aSize), downloadedBytes(aDownloadedBytes), status(STATUS_WAITING), 
		priority(aPriority), current(NULL), currentDownload(NULL), added(aAdded),
		tthRoot(tth == NULL ? NULL : new TTHValue(*tth))
	{ };

	QueueItem(const QueueItem& rhs) : 
	Flags(rhs), target(rhs.target), tempTarget(rhs.tempTarget), searchString(rhs.searchString),
		size(rhs.size), downloadedBytes(rhs.downloadedBytes), status(rhs.status), priority(rhs.priority), 
		current(rhs.current), currentDownload(rhs.currentDownload), added(rhs.added), tthRoot(rhs.tthRoot == NULL ? NULL : new TTHValue(*rhs.tthRoot))
	{
		// Deep copy the source lists
		Source::List::const_iterator i;
		for(i = rhs.sources.begin(); i != rhs.sources.end(); ++i) {
			sources.push_back(new Source(*(*i)));
		}
		for(i = rhs.badSources.begin(); i != rhs.badSources.end(); ++i) {
			badSources.push_back(new Source(*(*i)));
		}
	}

	virtual ~QueueItem() { 
		for_each(sources.begin(), sources.end(), DeleteFunction<Source*>());
		for_each(badSources.begin(), badSources.end(), DeleteFunction<Source*>());
		delete tthRoot;
	};

	int countOnlineUsers() const {
		int n = 0;
		Source::List::const_iterator i = sources.begin();
		for(; i != sources.end(); ++i) {
			if((*i)->getUser()->isOnline())
				n++;
		}
		return n;
	}
	bool hasOnlineUsers() const { return countOnlineUsers() > 0; };

	const string& getSourcePath(const User::Ptr& aUser) { 
		dcassert(isSource(aUser)); 
		return (*getSource(aUser, sources))->getPath();
	}

	Source::List& getSources() { return sources; };
	Source::List& getBadSources() { return badSources; };

	void getOnlineUsers(User::List& l) const  {
		for(Source::List::const_iterator i = sources.begin(); i != sources.end(); ++i)
			if((*i)->getUser()->isOnline())
				l.push_back((*i)->getUser());
	}

	string getTargetFileName() const { return Util::getFileName(getTarget()); };

	Source::Iter getSource(const User::Ptr& aUser) { return getSource(aUser, sources); };
	Source::Iter getBadSource(const User::Ptr& aUser) { return getSource(aUser, badSources); };

	bool isSource(const User::Ptr& aUser) { return (getSource(aUser, sources) != sources.end()); };
	bool isBadSource(const User::Ptr& aUser) { return (getSource(aUser, badSources) != badSources.end()); };

	bool isSource(const User::Ptr& aUser, const string& aFile) const { return isSource(aUser, aFile, sources); };
	bool isBadSource(const User::Ptr& aUser, const string& aFile) const { return isSource(aUser, aFile, badSources); };
	
	void setCurrent(const User::Ptr& aUser) {
		dcassert(isSource(aUser));
		current = *getSource(aUser, sources);
	}

	string getListName() {
		dcassert(isSet(QueueItem::FLAG_USER_LIST));
		if(isSet(QueueItem::FLAG_XML_BZLIST)) {
			return getTarget() + ".xml.bz2";
		} else if(isSet(QueueItem::FLAG_BZLIST)) {
			return getTarget() + ".bz2";
		} else {
			return getTarget() + ".DcLst";
		}
	}

	GETSETREF(string, target, Target);
	GETSETREF(string, tempTarget, TempTarget);
	GETSETREF(string, searchString, SearchString);
	GETSET(int64_t, size, Size);
	GETSET(int64_t, downloadedBytes, DownloadedBytes);
	GETSET(Status, status, Status);
	GETSET(Priority, priority, Priority);
	GETSET(Source*, current, Current);
	GETSET(Download*, currentDownload, CurrentDownload);
	GETSET(u_int32_t, added, Added);
	GETSET(TTHValue*, tthRoot, TTHRoot);
private:
	friend class QueueManager;
	Source::List sources;
	Source::List badSources;	

	Source* addSource(const User::Ptr& aUser, const string& aPath) {
		dcassert(!isSource(aUser));
		Source* s = NULL;
		Source::Iter i = getSource(aUser, badSources);
		if(i != badSources.end()) {
			s = *i;
			badSources.erase(i);
			s->setPath(aPath);
		} else {
			s = new Source(aUser, aPath);
		}

		sources.push_back(s);
		return s;
	}

	void removeSource(const User::Ptr& aUser, int reason) {
		Source::Iter i = getSource(aUser, sources);
		dcassert(i != sources.end());
		(*i)->setFlag(reason);
		badSources.push_back(*i);
		sources.erase(i);
	}

	static Source::Iter getSource(const User::Ptr& aUser, Source::List& lst) { 
		for(Source::Iter i = lst.begin(); i != lst.end(); ++i)
			if((*i)->getUser() == aUser)
				return i;
		return lst.end();
	}
	static bool isSource(const User::Ptr& aUser, const string& aFile, const Source::List& lst) {
		for(Source::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
			Source* s = *i;
			if( (s->getUser() == aUser) ||
				((s->getUser()->getNick() == aUser->getNick()) && (s->getPath() == aFile)) )
				return true;
		}
		return false;
	}

};

#endif // !defined(AFX_QUEUEITEM_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)

/**
* @file
* $Id: QueueItem.h,v 1.6 2004/02/16 13:21:40 arnetheduck Exp $
*/
