// QueueManager.h: interface for the QueueManager class.
//
//////////////////////////////////////////////////////////////////////

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
		
		Source(const string& aNick, const string& aPath) : nick(aNick), path(aPath) { };
		Source(const User::Ptr& aUser, const string& aPath) : user(aUser), path(aPath), nick(aUser->getNick()) { };
		Source(const Source& aSource) : nick(aSource.nick), path(aSource.path), user(aSource.user) { }

		User::Ptr& getUser() { return user; };
		void setUser(const User::Ptr& aUser) {
			user = aUser;
			if(user)
				nick = user->getNick();
		}
		
		string getFileName() {
			int i = path.rfind('\\');
			if(i != string::npos) {
				return path.substr(i + 1);
			} else {
				return path;
			}
		}

		GETSETREF(string, nick, Nick);
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
		Source* s = new Source(aUser, aPath);
		sources.push_back(s);
		return s;
	}
	Source* addSource(const string& aNick, const string& aPath) {
		Source* s = new Source(aNick, aPath);
		sources.push_back(s);
		return s;
	}
	
	void removeSource(const User::Ptr& aUser) {
		removeSource(aUser->getNick());
	}
	
	void removeSource(const string& aNick) {
		for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
			if((*i)->getNick() == aNick) {
				// Bingo!
				delete *i;
				sources.erase(i);
				return;
			}
		}
	}
	
	void setCurrent(const string& aNick) {
		for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
			if((*i)->getNick() == aNick) {
				// Bingo!
				current = *i;
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

	bool isSource(const User::Ptr& aUser) {
		for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
			if((*i)->getUser() == aUser) {
				return true;
			}
		}
		return false;
	}

	bool updateUsers(bool reconnect);
	
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

	virtual void onAction(Types type, QueueItem* aQI) { };
};

class QueueManager : public Singleton<QueueManager>, public Speaker<QueueManagerListener>, private TimerManagerListener
{
public:
	
	void add(const string& aFile, const string& aSize, const User::Ptr& aUser, const string& aTarget, 
			 bool aResume = true) throw(QueueException, FileException) {

		add(aFile, aSize.length() > 0 ? Util::toInt64(aSize.c_str()) : -1, aUser, aTarget, aResume);
	}
	void add(const string& aFile, LONGLONG aSize, const User::Ptr& aUser, const string& aTarget, 
			 bool aResume = true) throw(QueueException, FileException);
	
	void add(const string& aFile, const string& aSize, const string& aUser, const string& aTarget, 
			 bool aResume = true) throw(QueueException, FileException) {

		add(aFile, aSize.length() > 0 ? Util::toInt64(aSize.c_str()) : -1, aUser, aTarget, aResume);
	}
	void add(const string& aFile, LONGLONG aSize, const string& aUser, const string& aTarget, 
			 bool aResume = true) throw(QueueException, FileException);

	void addList(const User::Ptr& aUser) throw(QueueException, FileException) {
		string file = Util::getAppPath() + aUser->getNick() + ".DcLst";
		add(USER_LIST_NAME, -1, aUser, file, false);
		userLists.push_back(file);
	}
	
	void addList(const string& aUser) throw(QueueException, FileException) {
		string file = Util::getAppPath() + aUser + ".DcLst";
		add(USER_LIST_NAME, -1, aUser, file, false);
		userLists.push_back(file);
	}

	void remove(const string& aTarget) throw(QueueException);
	void removeSource(const string& aTarget, const User::Ptr& aUser) {
		removeSource(aTarget, aUser->getNick());
	}

	void removeSource(const string& aTarget, const string& aUser);
	
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
	
	QueueManager() : dirty(false) { TimerManager::getInstance()->addListener(this); };
	
	virtual ~QueueManager() { 
		TimerManager::getInstance()->removeListener(this); 

		for_each(userLists.begin(), userLists.end(), File::deleteFile);

		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			delete *i;
		}
	};
	
	CriticalSection cs;
	QueueItem::List queue;
	StringList userLists;
	static const string USER_LIST_NAME;
	
	void remove(QueueItem* aQI) throw(QueueException);

	QueueItem* findByTarget(const string& aTarget) {
		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			if((*i)->getTarget() == aTarget)
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
	
};

#endif // !defined(AFX_QUEUEMANAGER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)
