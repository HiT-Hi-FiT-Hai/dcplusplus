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
		RUNNING,
		FINISHED
	};
	enum Priority {
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
		
		Source(const User::Ptr& aUser, const string& aPath) : user(aUser), path(aPath) { };
		Source(const Source& aSource) : path(aSource.path), user(aSource.user) { }

		User::Ptr& getUser() { return user; };
		void setUser(const User::Ptr& aUser) {
			user = aUser;
		}
		
		string getFileName() {
			int i = path.rfind('\\');
			if(i != string::npos) {
				return path.substr(i + 1);
			} else {
				return path;
			}
		}
		GETSETREF(string, path, Path);
		
	private:
		User::Ptr user;
	};

	QueueItem(const string& aTarget, LONGLONG aSize, Priority aPriority, bool aResume) : target(aTarget), size(aSize),
		status(WAITING), priority(aPriority), current(NULL) { if(aResume) setFlag(RESUME); };
	
	QueueItem(const QueueItem& aQi) : target(aQi.target), status(aQi.status), priority(aQi.priority),
		size(aQi.size), current(aQi.current) {
		
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
		int i = getTarget().rfind('\\');
		if(i != string::npos) {
			return getTarget().substr(i + 1);
		} else {
			return getTarget();
		}
	}
	
	
	GETSETREF(string, target, Target);
	GETSET(Status, status, Status);
	GETSET(Priority, priority, Priority);
	GETSET(LONGLONG, size, Size);
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
		QUEUE_ITEM
	};

	virtual void onAction(Types, QueueItem*) { };
};

class QueueManager : public Singleton<QueueManager>, public Speaker<QueueManagerListener>, private TimerManagerListener, private SearchManagerListener
{
public:
	
	void add(const string& aFile, const string& aSize, const User::Ptr& aUser, const string& aTarget, 
			 bool aResume = true) throw(QueueException, FileException) {

		add(aFile, aSize.length() > 0 ? Util::toInt64(aSize.c_str()) : -1, aUser, aTarget, aResume);
	}
	void add(const string& aFile, LONGLONG aSize, const User::Ptr& aUser, const string& aTarget, 
			 bool aResume = true) throw(QueueException, FileException);
	
	void addList(const User::Ptr& aUser) throw(QueueException, FileException) {
		string file = Util::getAppPath() + aUser->getNick() + ".DcLst";
		add(USER_LIST_NAME, -1, aUser, file, false);
		userLists.push_back(file);
	}
	
	void remove(const string& aTarget) throw(QueueException);
	void removeSource(const string& aTarget, User::Ptr& aUser, bool removeConn = true);
	
	void setPriority(const string& aTarget, QueueItem::Priority p) throw();
	
	StringList getTargetsBySize(LONGLONG aSize) {
		Lock l(cs);
		StringList sl;
		
		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			if((*i)->getSize() == aSize) {
				sl.push_back((*i)->getTarget());
			}
		}
		return sl;
	}

	Download* getDownload(UserConnection* aUserConnection);
	bool hasDownload(const User::Ptr& aUser);
	void putDownload(Download* aDownload, bool finished = false);
	
	void getQueue() {
		Lock l(cs);
		for(QueueItem::Iter i = queue.begin(); i!= queue.end(); ++i) {
			fire(QueueManagerListener::QUEUE_ITEM, *i);
		}
	}

	void load(SimpleXML* aXml);
	void save(SimpleXML* aXml);
	
	GETSET(bool, dirty, Dirty);
private:

	friend class Singleton<QueueManager>;
	
	QueueManager() : dirty(false) { 
		TimerManager::getInstance()->addListener(this); 
		SearchManager::getInstance()->addListener(this); 
	};
	
	virtual ~QueueManager() { 
		SearchManager::getInstance()->removeListener(this);
		TimerManager::getInstance()->removeListener(this); 

		for_each(userLists.begin(), userLists.end(), File::deleteFile);

		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			delete *i;
		}
	};
	
	CriticalSection cs;
	QueueItem::List queue;
	StringList userLists;
	map<string, DWORD> search;
	
	static const string USER_LIST_NAME;
	
	void remove(QueueItem* aQI) throw(QueueException);

	QueueItem* findByTarget(const string& aTarget) {
		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			if(stricmp((*i)->getTarget().c_str(), aTarget.c_str()) == 0)
				return *i;
		}
		return NULL;
	}
	QueueItem* getQueueItem(const string& aFile, const string& aTarget, LONGLONG aSize, bool aResume, bool& newItem) throw(QueueException, FileException);
	
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD aTick) {
		switch(type) {
		case TimerManagerListener::MINUTE:
			onTimerMinute(aTick); break;
		}
	}
	void onTimerMinute(DWORD aTick);
	
	// SearchManagerListener
	virtual void onAction(SearchManagerListener::Types, SearchResult*);
};

#endif // !defined(AFX_QUEUEMANAGER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)

/**
 * @file QueueManager.h
 * $Id: QueueManager.h,v 1.10 2002/02/27 12:02:09 arnetheduck Exp $
 * @if LOG
 * $Log: QueueManager.h,v $
 * Revision 1.10  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.9  2002/02/25 15:39:29  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.8  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * @endif
 */

