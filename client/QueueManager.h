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

#if !defined(AFX_QUEUEMANAGER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)
#define AFX_QUEUEMANAGER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "CriticalSection.h"
#include "Exception.h"
#include "User.h"
#include "File.h"
#include "TimerManager.h"
#include "SearchManagerListener.h"
#include "ClientManagerListener.h"

#include "QueueManagerListener.h"

STANDARD_EXCEPTION(QueueException);

class QueueManager;
class Download;
class UserConnection;

class QueueItem : public Flags {
public:
#if 0
	// These are for the hash map type of indexing in the queue manager, resulting
	// in more efficient lookups of names but no sorting on save...
#endif

	typedef QueueItem* Ptr;
	// Strange, the vc7 optimizer won't take a deque here...
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	typedef map<string, Ptr, noCaseStringLess> StringMap;
//	typedef HASH_MAP<string, Ptr, noCaseStringHash, noCaseStringEq> StringMap;
	typedef StringMap::iterator StringIter;
	typedef HASH_MAP<User::Ptr, Ptr, User::HashFunction> UserMap;
	typedef UserMap::iterator UserIter;
	typedef HASH_MAP<User::Ptr, List, User::HashFunction> UserListMap;
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
	enum {
		/** This download should be resumed if possible */
		FLAG_RESUME = 0x01,
		/** This is a user file listing download */
		FLAG_USER_LIST = 0x02,
		/** The file list is downloaded to use for directory download (used with USER_LIST) */
		FLAG_DIRECTORY_DOWNLOAD = 0x04,
		/** The file list is downloaded to be viewed in the gui (used with USER_LIST) */
		FLAG_CLIENT_VIEW = 0x08,
		/** The file list downloaded was actually BZ compressed (MyList.bz2, only available in FINISHED message) */
		FLAG_BZLIST = 0x10,
	};

	class Source : public Flags {
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

	QueueItem(const string& aTarget, int64_t aSize, Priority aPriority, bool aResume) : target(aTarget), size(aSize),
		status(STATUS_WAITING), priority(aPriority), current(NULL) { if(aResume) setFlag(FLAG_RESUME); };
	
	QueueItem(const QueueItem& aQi) : Flags(aQi), target(aQi.target), size(aQi.size), status(aQi.status), priority(aQi.priority),
		current(aQi.current) {
		Source::List::const_iterator i;
		for(i = aQi.sources.begin(); i != aQi.sources.end(); ++i) {
			sources.push_back(new Source(*(*i)));
		}
		for(i = aQi.badSources.begin(); i != aQi.badSources.end(); ++i) {
			badSources.push_back(new Source(*(*i)));
		}
		
	}

	~QueueItem() { 
		for_each(sources.begin(), sources.end(), DeleteFunction<Source*>());
		for_each(badSources.begin(), badSources.end(), DeleteFunction<Source*>());
	};
	
	bool hasOnlineUsers() {
		Source::Iter i = sources.begin();
		for(; i != sources.end(); ++i) {
			if((*i)->getUser()->isOnline())
				break;
		}
		return (i != sources.end());
	}

	const string& getSourcePath(const User::Ptr& aUser) { 
		dcassert(isSource(aUser)); 
		return (*getSource(aUser))->getPath();
	}

	Source::List& getSources() { return sources; };
	Source::List& getBadSources() { return badSources; };
	string getTargetFileName() { return Util::getFileName(getTarget()); };

	bool isSource(const User::Ptr& aUser) { return (getSource(aUser) != sources.end()); };

	void setCurrent(const User::Ptr& aUser) {
		dcassert(isSource(aUser));
		current = *getSource(aUser);
	}
	
	GETSETREF(string, tempTarget, TempTarget);
	GETSETREF(string, target, Target);
	GETSET(int64_t, size, Size);
	GETSET(Status, status, Status);
	GETSET(Priority, priority, Priority);
	GETSET(Source*, current, Current);
private:
	friend class QueueManager;
	Source::List sources;
	Source::List badSources;
	
	Source* addSource(const User::Ptr& aUser, const string& aPath) {
		dcassert(!isSource(aUser));
		Source* s = NULL;
		Source::Iter i = getBadSource(aUser);
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
		Source::Iter i = getSource(aUser);
		dcassert(i != sources.end());
		(*i)->setFlag(reason);
		badSources.push_back(*i);
		sources.erase(i);
	}
	
	bool isBadSource(const User::Ptr& aUser) { return (getBadSource(aUser) != badSources.end()); };

	Source::Iter getSource(const User::Ptr& aUser) { return find(sources.begin(), sources.end(), aUser); };
	Source::Iter getBadSource(const User::Ptr& aUser) { return find(badSources.begin(), badSources.end(), aUser); };
};

inline bool operator==(QueueItem::Source* s, const User::Ptr& u) { return s->getUser() == u; };

class DirectoryItem {
public:
	typedef DirectoryItem* Ptr;
	typedef HASH_MULTIMAP<User::Ptr, Ptr, User::HashFunction> DirectoryMap;
	typedef DirectoryMap::iterator DirectoryIter;
	typedef pair<DirectoryIter, DirectoryIter> DirectoryPair;
	
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	DirectoryItem() : priority(QueueItem::DEFAULT) { };
	DirectoryItem(const User::Ptr& aUser, const string& aName, const string& aTarget, 
		QueueItem::Priority p) : name(aName), target(aTarget), priority(p), user(aUser) { };
	~DirectoryItem() { };
	
	User::Ptr& getUser() { return user; };
	void setUser(const User::Ptr& aUser) { user = aUser; };
	
	GETSETREF(string, name, Name);
	GETSETREF(string, target, Target);
	GETSET(QueueItem::Priority, priority, Priority);
private:
	User::Ptr user;
};

class ConnectionQueueItem;

class QueueManager : public Singleton<QueueManager>, public Speaker<QueueManagerListener>, private TimerManagerListener, 
	private SearchManagerListener, private ClientManagerListener
{
public:
	
	void add(const string& aFile, const string& aSize, const User::Ptr& aUser, const string& aTarget, 
		bool aResume = true, QueueItem::Priority p = QueueItem::DEFAULT, 
		const string& aTempTarget = Util::emptyString, bool addBad = true, 
		bool isDirectory = false) throw(QueueException, FileException) {
		add(aFile, aSize.length() > 0 ? Util::toInt64(aSize.c_str()) : -1, aUser, 
			aTarget, aResume, p, aTempTarget, addBad, isDirectory);
	}
	void add(const string& aFile, int64_t aSize, User::Ptr aUser, const string& aTarget, 
		bool aResume = true, QueueItem::Priority p = QueueItem::DEFAULT, 
		const string& aTempTarget = Util::emptyString, bool addBad = true, 
		bool isDirectory = false) throw(QueueException, FileException);
	
	void addList(const User::Ptr& aUser, bool isDirectory = false) throw(QueueException, FileException) {
		string file = Util::getAppPath() + "FileLists\\" + aUser->getNick() + ".DcLst";
		add(USER_LIST_NAME, -1, aUser, file, false, QueueItem::DEFAULT, Util::emptyString, true, isDirectory);
	}

	void addDirectory(const string& aDir, User::Ptr& aUser, const string& aTarget, QueueItem::Priority p = QueueItem::DEFAULT) throw();
	
	/** Move the target location of a queued item. Running items are silently ignored */
	void move(const string& aSource, const string& aTarget) throw();

	void remove(const string& aTarget) throw();
	void removeSource(const string& aTarget, User::Ptr& aUser, int reason, bool removeConn = true) throw();
	void removeSources(User::Ptr& aUser, int reason) throw();
	
	void setPriority(const string& aTarget, QueueItem::Priority p) throw();
	
	StringList getTargetsBySize(int64_t aSize, const string& suffix) throw() {
		Lock l(cs);
		StringList sl;
		
		for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
			if(i->second->getSize() == aSize) {
				const string& t = i->second->getTarget();
				if(suffix.empty() || (suffix.length() < t.length() &&
					Util::stricmp(suffix.c_str(), t.c_str() + (t.length() - suffix.length())) == 0) )
					sl.push_back(t);
			}
		}
		return sl;
	}

	QueueItem::StringMap& lockQueue() throw() { cs.enter(); return queue; } ;
	void unlockQueue() throw() { cs.leave(); };

	Download* getDownload(User::Ptr& aUser) throw();
	void putDownload(Download* aDownload, bool finished = false) throw();

	bool hasDownload(const User::Ptr& aUser, QueueItem::Priority minPrio = QueueItem::LOWEST) throw() {
		Lock l(cs);
		return (userQueue.getNext(aUser, minPrio) != NULL);
	}
	
	void importNMQueue(const string& aFile) throw(FileException);
	void loadQueue() throw();
	void saveQueue() throw();
	
	GETSET(u_int32_t, lastSave, LastSave);
	GETSETREF(string, queueFile, QueueFile);
private:

	class UserQueue {
	public:
		void add(QueueItem* qi, bool inFront = false);
		void add(QueueItem* qi, const User::Ptr& aUser, bool inFront = false);
		QueueItem* getNext(const User::Ptr& aUser, QueueItem::Priority minPrio = QueueItem::LOWEST);
		QueueItem* getRunning(const User::Ptr& aUser);
		void setRunning(QueueItem* qi, const User::Ptr& aUser);
		void setWaiting(QueueItem* qi);
		void getUserList(User::List& l, QueueItem::Priority p);
		QueueItem::UserListMap& getList(int p) { return userQueue[p]; };
		void remove(QueueItem* qi);
		void remove(QueueItem* qi, const User::Ptr& aUser);
	private:
		/** QueueItems by priority and user (this is where the download order is determined) */
		QueueItem::UserListMap userQueue[QueueItem::LAST];
		/** Currently running downloads, a QueueItem is always either here or in the queue */
		QueueItem::UserMap running;
	};

	friend class Singleton<QueueManager>;
	
	QueueManager();
	virtual ~QueueManager();
	
	CriticalSection cs;
	
	/** QueueItems by target */
	QueueItem::StringMap queue;
	
	/** QueueItems by user */
	UserQueue userQueue;

	/** Directories queued for downloading */
	DirectoryItem::DirectoryMap directories;

	/** Recent searches list, to avoid searching for the same thing too often */
	StringList recent;

	/** A hint where to insert an item... */
	QueueItem::StringIter lastInsert;

	/** The queue needs to be saved */
	bool dirty;

	/** Searched last minute (ugly */
	bool searched;
	
	static const string USER_LIST_NAME;
	static string getTempName(const string& aFileName);
	QueueItem* getQueueItem(const string& aFile, const string& aTarget, int64_t aSize, bool aResume, bool& newItem) throw(QueueException, FileException);
	
	void removeAll(QueueItem* q);
	void load(SimpleXML* aXml);

	void setDirty() {
		if(!dirty) {
			dirty = true;
			lastSave = GET_TICK();
		}
	}

	QueueItem* findByTarget(const string& aTarget);

	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t aTick) throw();
	void onTimerMinute(u_int32_t aTick);
	
	// SearchManagerListener
	virtual void onAction(SearchManagerListener::Types, SearchResult*) throw();

	// ClientManagerListener
	virtual void onAction(ClientManagerListener::Types type, const User::Ptr& aUser) throw();
};

#endif // !defined(AFX_QUEUEMANAGER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)

/**
 * @file QueueManager.h
 * $Id: QueueManager.h,v 1.33 2003/03/13 13:31:27 arnetheduck Exp $
 */

