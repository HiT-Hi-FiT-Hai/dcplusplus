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
#include "SearchManager.h"
#include "ClientManager.h"

STANDARD_EXCEPTION(QueueException);

class QueueManager;
class Download;
class UserConnection;

class QueueItem : public Flags {
public:
	typedef QueueItem* Ptr;
	typedef HASH_MAP<string, Ptr> StringMap;
	typedef StringMap::iterator StringIter;
	typedef HASH_MULTIMAP<User::Ptr, Ptr> UserMap;
	typedef UserMap::iterator UserIter;
	typedef pair<UserIter, UserIter> UserPair;

	enum Status {
		WAITING,
		RUNNING
	};

	enum Priority {
		DEFAULT = -1,
		PAUSED = 0,
		LOWEST,
		LOW,
		NORMAL,
		HIGH,
		HIGHEST
	};
	enum {
		RESUME = 0x01,
		USER_LIST = 0x02
	};

	class Source : public Flags {
	public:
		typedef Source* Ptr;
		typedef vector<Ptr> List;
		typedef List::iterator Iter;
		enum {
			FLAG_FILE_NOT_FOUND,
			FLAG_ROLLBACK_INCONSISTENCY
		};

		Source(const User::Ptr& aUser, const string& aPath) : path(aPath), user(aUser) { };
		Source(const Source& aSource) : path(aSource.path), user(aSource.user) { }

		User::Ptr& getUser() { return user; };
		void setUser(const User::Ptr& aUser) {
			user = aUser;
		}
		
		string getFileName() {
			string::size_type i = path.rfind('\\');
			return (i != string::npos) ? path.substr(i + 1) : path;
		}

		GETSETREF(string, path, Path);
	private:
		User::Ptr user;
	};

	QueueItem(const string& aTarget, int64_t aSize, Priority aPriority, bool aResume) : target(aTarget), size(aSize),
		status(WAITING), priority(aPriority), current(NULL) { if(aResume) setFlag(RESUME); };
	
	QueueItem(const QueueItem& aQi) : target(aQi.target), size(aQi.size), status(aQi.status), priority(aQi.priority),
		current(aQi.current) {
		
		for(Source::List::const_iterator i = aQi.sources.begin(); i != aQi.sources.end(); ++i) {
			sources.push_back(new Source(*(*i)));
		}
	}

	~QueueItem() { 
		for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
			delete *i;
		}
	};
	
	const string& getSourcePath(const User::Ptr& aUser) {
		for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
			if((*i)->getUser() == aUser)
				return (*i)->getPath();
		}

		dcassert(0);
		return Util::emptyString;
	}

	Source::List& getSources() { return sources; };

	string getTargetFileName() {
		string::size_type i = getTarget().rfind('\\');
		return (i != string::npos) ? getTarget().substr(i + 1) : getTarget();
	}
	
	GETSETREF(string, target, Target);
	GETSET(int64_t, size, Size);
	GETSET(Status, status, Status);
	GETSET(Priority, priority, Priority);
private:
	friend class QueueManager;
	Source::List sources;
	
	Source* current;
	
	Source* addSource(const User::Ptr& aUser, const string& aPath) {
		Source* s = getSource(aUser);
		if(s == NULL) {
			Source* s = new Source(aUser, aPath);
			sources.push_back(s);
		}
		return s;
	}
	void removeSource(const User::Ptr& aUser) {
		for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
			if((*i)->getUser() == aUser) {
				// Bingo!
				delete *i;
				sources.erase(i);
				
				return;
			}
		}
	}
	
	void setCurrent(const User::Ptr& aUser) {
		for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
			if((*i)->getUser() == aUser) {
				// Bingo!
				current = *i;
			}
		}
	}
	
	void setCurrent(Source* aSource) {
		current = aSource;
	}

	Source* getCurrent() { return current; };

	Source* getSource(const User::Ptr& aUser) {
		for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
			if((*i)->getUser() == aUser) {
				return *i;
			}
		}
		return NULL;
	}
	bool isSource(const User::Ptr& aUser) {
		for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
			if((*i)->getUser() == aUser) {
				return true;
			}
		}
		return false;
	}
	
};

class QueueManagerListener {
public:
	typedef QueueManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	enum Types {
		ADDED,
		REMOVED,
		SOURCES_UPDATED,
		STATUS_UPDATED,
		QUEUE_ITEM,
		QUEUE
	};

	virtual void onAction(Types, QueueItem*) { };
	virtual void onAction(Types, const QueueItem::StringMap&) { };
};
class ConnectionQueueItem;

class QueueManager : public Singleton<QueueManager>, public Speaker<QueueManagerListener>, private TimerManagerListener, 
	private SearchManagerListener, private SettingsManagerListener, private ClientManagerListener
{
public:
	
	void add(const string& aFile, const string& aSize, const User::Ptr& aUser, const string& aTarget, 
		bool aResume = true, QueueItem::Priority p = QueueItem::DEFAULT) throw(QueueException, FileException) {

		add(aFile, aSize.length() > 0 ? Util::toInt64(aSize.c_str()) : -1, aUser, aTarget, aResume, p);
	}
	void add(const string& aFile, int64_t aSize, User::Ptr aUser, const string& aTarget, 
		bool aResume = true, QueueItem::Priority p = QueueItem::DEFAULT) throw(QueueException, FileException);
	
	void addList(const User::Ptr& aUser) throw(QueueException, FileException) {
		string file = Util::getAppPath() + "FileLists\\" + aUser->getNick() + ".DcLst";
		add(USER_LIST_NAME, -1, aUser, file, false);
	}
	
	void remove(const string& aTarget) throw(QueueException);
	void removeSource(const string& aTarget, User::Ptr& aUser, bool removeConn = true);
	
	void setPriority(const string& aTarget, QueueItem::Priority p) throw();
	
	StringList getTargetsBySize(int64_t aSize) {
		Lock l(cs);
		StringList sl;
		
		for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
			if(i->second->getSize() == aSize) {
				sl.push_back(i->second->getTarget());
			}
		}
		return sl;
	}

	Download* getDownload(User::Ptr& aUser);
	bool hasDownload(const User::Ptr& aUser) {
		Lock l(cs);
		QueueItem::UserPair up = userQueue.equal_range(aUser);
		
		for(QueueItem::UserIter i = up.first; i != up.second; ++i) {
			dcassert(i->second->isSource(aUser));
			if( (i->second->getStatus() != QueueItem::RUNNING) &&
				(i->second->getPriority() != QueueItem::PAUSED) ) {
				return true;
			}
		}
		return false;
	}
	
	void putDownload(Download* aDownload, bool finished = false);

	void getQueue() {
		Lock l(cs);
		fire(QueueManagerListener::QUEUE, queue);
	}

	void importNMQueue(const string& aFile) throw(FileException);
	void loadQueue();
	
	GETSET(bool, dirty, Dirty);
	GETSET(u_int32_t, lastSave, LastSave);
	GETSETREF(string, queueFile, QueueFile);
private:

	friend class Singleton<QueueManager>;
	
	QueueManager() : dirty(false), lastSave(0), queueFile(Util::getAppPath() + "Queue.xml") { 
		SettingsManager::getInstance()->addListener(this);
		TimerManager::getInstance()->addListener(this); 
		SearchManager::getInstance()->addListener(this);
		ClientManager::getInstance()->addListener(this);
		Util::ensureDirectory(Util::getAppPath() + "FileLists\\");
	};
	virtual ~QueueManager();
	
	CriticalSection cs;
	
	QueueItem::StringMap queue;
	QueueItem::UserMap userQueue;

	typedef deque<pair<string, u_int32_t> > SearchList;
	typedef SearchList::iterator SearchIter;
	SearchList search;

	static const string USER_LIST_NAME;
	
	QueueItem* getQueueItem(const string& aFile, const string& aTarget, int64_t aSize, bool aResume, bool& newItem) throw(QueueException, FileException);
	void remove(QueueItem* q);

	void load(SimpleXML* aXml);
	void saveQueue();

	QueueItem* findByTarget(const string& aTarget);

	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t aTick);
	void onTimerMinute(u_int32_t aTick);
	
	// SearchManagerListener
	virtual void onAction(SearchManagerListener::Types, SearchResult*);

	// SettingsManagerListener
	virtual void onAction(SettingsManagerListener::Types type, SimpleXML* xml);

	// ClientManagerListener
	virtual void onAction(ClientManagerListener::Types type, const User::Ptr& aUser);
};

#endif // !defined(AFX_QUEUEMANAGER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)

/**
 * @file QueueManager.h
 * $Id: QueueManager.h,v 1.24 2002/05/26 20:28:11 arnetheduck Exp $
 */

