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

STANDARD_EXCEPTION(QueueException);

class QueueManager;
class Download;
class UserConnection;

class QueueItem : public Flags {
public:
	typedef QueueItem* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;

	enum Status {
		WAITING,
		RUNNING
	};
	enum Priority {
		DEFAULT = -1,
		PAUSED = 0,
		LOW,
		NORMAL,
		HIGH
	};
	enum Flags {
		RESUME = 0x01,
		USER_LIST = 0x02
	};

	class Source {
	public:
		typedef Source* Ptr;
		typedef vector<Ptr> List;
		typedef List::iterator Iter;
		
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
	virtual void onAction(Types, const QueueItem::List&) { };
};
class ConnectionQueueItem;

class QueueManager : public Singleton<QueueManager>, public Speaker<QueueManagerListener>, private TimerManagerListener, 
	private SearchManagerListener, private SettingsManagerListener
{
public:
	
	void add(const string& aFile, const string& aSize, const User::Ptr& aUser, const string& aTarget, 
		bool aResume = true, QueueItem::Priority p = QueueItem::DEFAULT) throw(QueueException, FileException) {

		add(aFile, aSize.length() > 0 ? Util::toInt64(aSize.c_str()) : -1, aUser, aTarget, aResume, p);
	}
	void add(const string& aFile, int64_t aSize, const User::Ptr& aUser, const string& aTarget, 
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
		
		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			if((*i)->getSize() == aSize) {
				sl.push_back((*i)->getTarget());
			}
		}
		return sl;
	}

	Download* getDownload(User::Ptr& aUser);
	bool hasDownload(const User::Ptr& aUser) {
		Lock l(cs);
		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			if(((*i)->getStatus() != QueueItem::RUNNING) && 
				((*i)->getPriority() != QueueItem::PAUSED) && 
				((*i)->isSource(aUser)) ) {
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
	
	GETSET(bool, dirty, Dirty);
private:

	friend class Singleton<QueueManager>;
	
	QueueManager() : dirty(false) { 
		SettingsManager::getInstance()->addListener(this);
		TimerManager::getInstance()->addListener(this); 
		SearchManager::getInstance()->addListener(this);
		Util::ensureDirectory(Util::getAppPath() + "FileLists\\");
	};
	
	virtual ~QueueManager() { 
		SettingsManager::getInstance()->removeListener(this);
		SearchManager::getInstance()->removeListener(this);
		TimerManager::getInstance()->removeListener(this); 
#ifdef WIN32
		string path = Util::getAppPath() + "FileLists\\";
		WIN32_FIND_DATA data;
		HANDLE hFind;
		
		hFind = FindFirstFile((path + "\\*.bz2").c_str(), &data);
		if(hFind != INVALID_HANDLE_VALUE) {
			do {
				File::deleteFile(path + data.cFileName);			
			} while(FindNextFile(hFind, &data));

			FindClose(hFind);
		}

		hFind = FindFirstFile((path + "\\*.DcLst").c_str(), &data);
		if(hFind != INVALID_HANDLE_VALUE) {
			do {
				File::deleteFile(path + data.cFileName);			
			} while(FindNextFile(hFind, &data));
			
			FindClose(hFind);
		}
#endif
		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			delete *i;
		}
	};
	
	CriticalSection cs;
	QueueItem::List queue;
	typedef hash_map<string, u_int32_t> SearchMap;
	SearchMap search;

	static const string USER_LIST_NAME;
	
	QueueItem* findByTarget(const string& aTarget) {
		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			if(stricmp((*i)->getTarget().c_str(), aTarget.c_str()) == 0)
				return *i;
		}
		return NULL;
	}
	QueueItem* getQueueItem(const string& aFile, const string& aTarget, int64_t aSize, bool aResume, bool& newItem) throw(QueueException, FileException);
	
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t aTick) {
		switch(type) {
		case TimerManagerListener::MINUTE:
			onTimerMinute(aTick); break;
		}
	}
	void onTimerMinute(u_int32_t aTick);
	
	// SearchManagerListener
	virtual void onAction(SearchManagerListener::Types, SearchResult*);

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

#endif // !defined(AFX_QUEUEMANAGER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)

/**
 * @file QueueManager.h
 * $Id: QueueManager.h,v 1.19 2002/04/28 08:25:50 arnetheduck Exp $
 */

