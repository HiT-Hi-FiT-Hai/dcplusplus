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

#include "TimerManager.h"

#include "CriticalSection.h"
#include "Exception.h"
#include "User.h"
#include "File.h"
#include "DirectoryListing.h"
#include "QueueItem.h"
#include "Singleton.h"

#include "QueueManagerListener.h"
#include "SearchManagerListener.h"
#include "ClientManagerListener.h"

STANDARD_EXCEPTION(QueueException);

class UserConnection;
class DirectoryListing;

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
	
	/** Add a file to the queue. */
	void add(const string& aFile, int64_t aSize, User::Ptr aUser, 
		const string& aTarget, const string& aSearchString = Util::emptyString, 
		int aFlags = QueueItem::FLAG_RESUME, QueueItem::Priority p = QueueItem::DEFAULT, 
		const string& aTempTarget = Util::emptyString, bool addBad = true) throw(QueueException, FileException);
	
	/** Add a user's filelist to the queue. */
	void addList(const User::Ptr& aUser, int aFlags) throw(QueueException, FileException) {
		string x = aUser->getNick();
		string::size_type i = 0;
		while((i = x.find('\\'), i) != string::npos)
			x[i] = '_';
		string file = Util::getAppPath() + "FileLists\\" + x + ".DcLst";
		add(USER_LIST_NAME, -1, aUser, file, Util::emptyString,
			QueueItem::FLAG_USER_LIST | aFlags,  QueueItem::DEFAULT, 
			Util::emptyString, true);
	}

	/** Add a directory to the queue (downloads filelist and matches the directory). */
	void addDirectory(const string& aDir, const User::Ptr& aUser, const string& aTarget, QueueItem::Priority p = QueueItem::DEFAULT) throw();
	
	int matchListing(DirectoryListing* dl) throw();

	/** Move the target location of a queued item. Running items are silently ignored */
	void move(const string& aSource, const string& aTarget) throw();

	void remove(const string& aTarget) throw();
	void removeSource(const string& aTarget, User::Ptr& aUser, int reason, bool removeConn = true) throw();
	void removeSources(User::Ptr& aUser, int reason) throw();
	
	void setPriority(const string& aTarget, QueueItem::Priority p) throw();
	
	void setSearchString(const string& aTarget, const string& searchString) throw();

	void getTargetsBySize(StringList& sl, int64_t aSize, const string& suffix) throw() {
		Lock l(cs);
		fileQueue.find(sl, aSize, suffix);
	}

	QueueItem::StringMap& lockQueue() throw() { cs.enter(); return fileQueue.getQueue(); } ;
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

	/** All queue items by target */
	class FileQueue {
	public:
		FileQueue() : lastInsert(queue.end()) { };
		~FileQueue() {
			for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i)
				delete i->second;
		}
		void add(QueueItem* qi);
		QueueItem* add(const string& aTarget, int64_t aSize, const string& aSearchString, 
			int aFlags, QueueItem::Priority p, const string& aTempTarget, int64_t aDownloaded,
			u_int32_t aAdded) throw(QueueException, FileException);
		QueueItem* find(const string& target);
		void find(StringList& sl, int64_t aSize, const string& ext);
		QueueItem* findAutoSearch(StringList& recent);
		size_t getSize() { return queue.size(); };
		QueueItem::StringMap& getQueue() { return queue; };
		void move(QueueItem* qi, const string& aTarget);
		void remove(QueueItem* qi) {
			if(lastInsert != queue.end() && lastInsert->first == qi->getTarget())
				lastInsert = queue.end();
			queue.erase(qi->getTarget());
			delete qi;
		}

	private:
		QueueItem::StringMap queue;
		/** A hint where to insert an item... */
		QueueItem::StringIter lastInsert;
	};

	/** All queue items indexed by user (this is a cache for the FileQueue really...) */
	class UserQueue {
	public:
		void add(QueueItem* qi);
		void add(QueueItem* qi, const User::Ptr& aUser);
		QueueItem* getNext(const User::Ptr& aUser, QueueItem::Priority minPrio = QueueItem::LOWEST);
		QueueItem* getRunning(const User::Ptr& aUser);
		void setRunning(QueueItem* qi, const User::Ptr& aUser);
		void setWaiting(QueueItem* qi);
		QueueItem::UserListMap& getList(int p) { return userQueue[p]; };
		void remove(QueueItem* qi);
		void remove(QueueItem* qi, const User::Ptr& aUser);

		QueueItem::UserMap& getRunning() { return running; };
		bool isRunning(const User::Ptr& aUser) const { 
			return (running.find(aUser) != running.end());
		};
	private:
		/** QueueItems by priority and user (this is where the download order is determined) */
		QueueItem::UserListMap userQueue[QueueItem::LAST];
		/** Currently running downloads, a QueueItem is always either here or in the userQueue */
		QueueItem::UserMap running;
	};

	friend class Singleton<QueueManager>;
	
	QueueManager();
	virtual ~QueueManager();
	
	CriticalSection cs;
	
	/** QueueItems by target */
	FileQueue fileQueue;
	/** QueueItems by user */
	UserQueue userQueue;
	/** Directories queued for downloading */
	DirectoryItem::DirectoryMap directories;
	/** Recent searches list, to avoid searching for the same thing too often */
	StringList recent;
	/** The queue needs to be saved */
	bool dirty;
	/** Next search */
	int32_t nextSearch;
	
	static const string USER_LIST_NAME;
	static string getTempName(const string& aFileName);

	/** Sanity check for the target filename */
	string checkTarget(const string& aTarget, int64_t aSize) throw(QueueException, FileException);
	/** Add a source to an existing queue item */
	bool addSource(QueueItem* qi, const string& aFile, User::Ptr aUser, bool addBad) throw(QueueException, FileException);

	void QueueManager::matchFiles(DirectoryListing::Directory* dir) throw();

	void removeAll(QueueItem* q);
	void load(SimpleXML* aXml);

	void setDirty() {
		if(!dirty) {
			dirty = true;
			lastSave = GET_TICK();
		}
	}

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
 * @file
 * $Id: QueueManager.h,v 1.46 2003/11/11 13:16:09 arnetheduck Exp $
 */

