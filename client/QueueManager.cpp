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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "QueueManager.h"
#include "ConnectionManager.h"
#include "ClientManager.h"
#include "DownloadManager.h"
#include "UserConnection.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "File.h"

QueueManager* Singleton<QueueManager>::instance = NULL;

const string QueueManager::USER_LIST_NAME = "MyList.DcLst";

QueueManager::~QueueManager() { 
	SettingsManager::getInstance()->removeListener(this);
	SearchManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this); 
	ClientManager::getInstance()->removeListener(this);

	saveQueue();

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
	for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
		delete i->second;
	}
};

void QueueManager::onTimerMinute(u_int32_t /*aTick*/) {
	if(BOOLSETTING(AUTO_SEARCH)) {
		{
			Lock l(cs);
			
			if(queue.size() > 0) {
				// No real need to keep more than 100 searches in the queue...
				for(QueueItem::StringIter i = queue.begin(); (search.size() < 100) && (i != queue.end()); ++i) {
					QueueItem* q = i->second;
					
					if(q->getStatus() == QueueItem::RUNNING) {
						continue;
					}
					
					if( q->getPriority() != QueueItem::PAUSED ) {
						bool online = false;
						for(QueueItem::Source::Iter j = q->getSources().begin(); j != q->getSources().end(); ++j) {
							if((*j)->getUser()->isOnline()) {
								online = true;
								break;
							}
						}

						if(!online) {
							SearchIter si;
							for(si = search.begin(); si != search.end(); ++si) {
								if(si->first == q->getTarget())
									break;
							}

							if(si == search.end()) {
								search.push_back(make_pair(q->getTarget(), GET_TICK()));
							}
						}
					}
				}
			}
		}
		
		while((search.size() > 0) && ((search.front().second + 3*60*1000) < GET_TICK()) ) {
			QueueItem* q = findByTarget(search.front().first);
			search.pop_front();
			if(q != NULL) {
				QueueItem::Source::Iter j;
				for(j = q->getSources().begin(); j != q->getSources().end(); ++j) {
					if((*j)->getUser()->isOnline()) {
						break;
					}
				}
				if(j == q->getSources().end()) {
					dcdebug("QueueManager::onTimerMinute Doing autosearch for %s\n", SearchManager::clean(q->getTargetFileName()).c_str());
					SearchManager::getInstance()->search(SearchManager::clean(q->getTargetFileName()), q->getSize() - 1, SearchManager::TYPE_ANY, SearchManager::SIZE_ATLEAST);
					break;
				} 
			} 
		} 
	}
}

void QueueManager::add(const string& aFile, int64_t aSize, User::Ptr aUser, const string& aTarget, 
					   bool aResume /* = true */, QueueItem::Priority p /* = QueueItem::DEFAULT */) throw(QueueException, FileException) {
	// Check that we're not downloading from ourselves...
						   
	if(aUser->getClientNick() == aUser->getNick()) {
		throw QueueException(STRING(NO_DOWNLOADS_FROM_SELF));
	}

	// Check that the file doesn't already exist...
	if( (aSize != -1) && (aSize <= File::getSize(aTarget)) )  {
		throw FileException(STRING(LARGER_TARGET_FILE_EXISTS));
	}
	
	// Alright, get a queue item, new or old...
	bool newItem = false;
	QueueItem* q = getQueueItem(aFile, aTarget, aSize, aResume, newItem);
	QueueItem::Source* s = NULL;

	if(p != QueueItem::DEFAULT) {
		q->setPriority(p);
	}
	bool newSource = false;

	{
		Lock l(cs);
		if(!q->isSource(aUser)) {
			s = q->addSource(aUser, aFile);
		} else {
			return;
		}
		if(newItem) {
            queue.insert(make_pair(q->getTarget(), q));
		}
		
		if(userQueue.find(aUser) == userQueue.end())
			newSource = true;

		userQueue.insert(make_pair(aUser, q));
	}

	// Good, now notify the listeners of the changes
	if(newItem)
		fire(QueueManagerListener::ADDED, q);

	fire(QueueManagerListener::SOURCES_UPDATED, q);
	dirty = true;

	if(newSource && aUser->isOnline() && (q->getStatus() != QueueItem::RUNNING) && (q->getPriority() != QueueItem::PAUSED))
		ConnectionManager::getInstance()->getDownloadConnection(aUser);
}

QueueItem* QueueManager::getQueueItem(const string& aFile, const string& aTarget, int64_t aSize, bool aResume, bool& newItem) throw(QueueException, FileException){
	QueueItem* q = findByTarget(aTarget);
	
	if(q == NULL) {
		newItem = true;
		if(aSize <= 16*1024)
			q = new QueueItem(aTarget, aSize, QueueItem::HIGHEST, aResume);
		else
			q = new QueueItem(aTarget, aSize, QueueItem::NORMAL, aResume);
		if(aFile == USER_LIST_NAME)
			q->setFlag(QueueItem::USER_LIST);
	} else {
		newItem = false;
		if(q->getSize() != aSize) {
			throw QueueException(STRING(FILE_WITH_DIFFERENT_SIZE));
		}

		if(aFile == USER_LIST_NAME) {
			throw QueueException(STRING(ALREADY_GETTING_THAT_LIST));
		}
	}

	return q;
}

Download* QueueManager::getDownload(User::Ptr& aUser) {

	QueueItem* q = NULL;

	{
		Lock l(cs);

		QueueItem::UserPair up = userQueue.equal_range(aUser);

		// Find a suitable download for this user
		for(QueueItem::UserIter i = up.first; i != up.second; ++i) {
			dcassert(i->second->isSource(aUser));

			if( (i->second->getStatus() == QueueItem::WAITING) &&
				(i->second->getPriority() != QueueItem::PAUSED) ) {
				if(q == NULL) {
					q = i->second;
					dcdebug("Found download for %s: %s\n", aUser->getNick().c_str(), q->getTarget().c_str());
					continue;
				} else if( (q->getPriority() < i->second->getPriority()) || 
					( (q->getPriority() == i->second->getPriority()) && (q->getSize() > i->second->getSize()) ) ) {
					q = i->second;
					dcdebug("Found better download for %s: %s\n", aUser->getNick().c_str(), q->getTarget().c_str());
				}
			}
		}
		if(q == NULL)
			return NULL;

		// Set the flag to running, so that we don't get the same download twice...
		q->setStatus(QueueItem::RUNNING);
		q->setCurrent(aUser);

		fire(QueueManagerListener::STATUS_UPDATED, q);
		
		Download* d = new Download(q->isSet(QueueItem::RESUME), q->isSet(QueueItem::USER_LIST));
		d->setSource(q->getCurrent()->getPath());
		d->setTarget(q->getTarget());
		d->setSize(q->getSize());
		return d;
	}
}

void QueueManager::putDownload(Download* aDownload, bool finished /* = false */) {
	{
		Lock l(cs);
		QueueItem* q = findByTarget(aDownload->getTarget());

		if(q != NULL) {
			if(finished) {
				remove(q);
				fire(QueueManagerListener::REMOVED, q);
				delete q;
			} else {
				q->setStatus(QueueItem::WAITING);
				q->setCurrent(NULL);
				fire(QueueManagerListener::STATUS_UPDATED, q);
				for(QueueItem::Source::Iter i = q->getSources().begin(); i != q->getSources().end(); ++i) {
					ConnectionManager::getInstance()->getDownloadConnection((*i)->getUser());
				}
			}
		}
 		aDownload->setUserConnection(NULL);
		delete aDownload;
		
	}
}

void QueueManager::remove(const string& aTarget) throw(QueueException) {
	Lock l(cs);

	QueueItem* q = findByTarget(aTarget);
	if(q != NULL) {
		if(q->getStatus() == QueueItem::RUNNING) {
			DownloadManager::getInstance()->abortDownload(q->getTarget());
		}
		remove(q);
		fire(QueueManagerListener::REMOVED, q);
		delete q;
	}
}

void QueueManager::remove(QueueItem* q) {
	// First, we have to remove all sources from active
	{
		for(QueueItem::Source::Iter i = q->getSources().begin(); i != q->getSources().end(); ++i) {
			QueueItem::UserPair up = userQueue.equal_range((*i)->getUser());
			for(QueueItem::UserIter j = up.first; j != up.second; ++j) {
				if(j->second == q) {
					userQueue.erase(j);
					break;
				}
			}
		}
	}
	// Then, the easy part...
	dcassert(queue.find(q->getTarget()) != queue.end());
	queue.erase(q->getTarget());
	dirty = true;
}

void QueueManager::removeSource(const string& aTarget, User::Ptr& aUser, bool removeConn /* = true */)  {
	Lock l(cs);
	QueueItem* q = findByTarget(aTarget);
	if(q != NULL) {

		if(removeConn && q->getStatus() == QueueItem::RUNNING) {
			dcassert(q->getCurrent());
			if(q->getCurrent()->getUser() == aUser) {
				// Oops...
				DownloadManager::getInstance()->abortDownload(q->getTarget());
			}
		}
		if(q->isSet(QueueItem::USER_LIST)) {
			remove(q);
		} else {
			q->removeSource(aUser);
			QueueItem::UserPair up = userQueue.equal_range(aUser);
			for(QueueItem::UserIter j = up.first; j != up.second; ++j) {
				if(j->second == q) {
					userQueue.erase(j);
					break;
				}
			}

			fire(QueueManagerListener::SOURCES_UPDATED, q);

			dirty = true;
		}
	}
}

void QueueManager::setPriority(const string& aTarget, QueueItem::Priority p) throw() {
	{
		Lock l(cs);
	
		QueueItem* q = findByTarget(aTarget);
		if(q != NULL) {
			q->setPriority(p);
			fire(QueueManagerListener::STATUS_UPDATED, q);
		}
	}
}

void QueueManager::saveQueue() {
	Lock l(cs);
	try {
		SimpleXML xml;

		xml.addTag("Downloads");
		xml.stepIn();
		for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
			QueueItem::Ptr d = i->second;
			if(!d->isSet(QueueItem::USER_LIST)) {
				xml.addTag("Download");
				xml.addChildAttrib("Target", d->getTarget());
				xml.addChildAttrib("Resume", d->isSet(QueueItem::RESUME));
				xml.addChildAttrib("Size", d->getSize());
				xml.addChildAttrib("Priority", (int)d->getPriority());

				xml.stepIn();
				for(QueueItem::Source::List::const_iterator j = d->sources.begin(); j != d->sources.end(); ++j) {
					QueueItem::Source* s = *j;
					xml.addTag("Source");
					xml.addChildAttrib("Nick", s->getUser()->getNick());
					xml.addChildAttrib("Path", s->getPath());
				}
				xml.stepOut();
			}
		}

		File f(getQueueFile() + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write("<?xml version=\"1.0\" encoding=\"windows-1252\"?>\r\n");
		f.write(xml.toXML());
		f.close();
		File::deleteFile(getQueueFile());
		File::renameFile(getQueueFile() + ".tmp", getQueueFile());

		dirty = false;
		lastSave = GET_TICK();
	} catch(Exception e) {
		// ...
	}
}

void QueueManager::loadQueue() {
	try {
		File f(getQueueFile(), File::READ, File::OPEN);
		SimpleXML xml;
		xml.fromXML(f.read());

		load(&xml);
	} catch(Exception e) {
		// ...
	}
}

void QueueManager::load(SimpleXML* aXml) {
	Lock l(cs);

	aXml->resetCurrentChild();
	if(aXml->findChild("Downloads")) {
		aXml->stepIn();
		
		while(aXml->findChild("Download")) {
			const string& target = aXml->getChildAttrib("Target");
			bool resume = aXml->getBoolChildAttrib("Resume");
			int64_t size = aXml->getLongLongChildAttrib("Size");
			QueueItem::Priority p = (QueueItem::Priority)aXml->getIntChildAttrib("Priority");
			aXml->stepIn();
			while(aXml->findChild("Source")) {
				const string& nick = aXml->getChildAttrib("Nick");
				const string& path = aXml->getChildAttrib("Path");
				
				try {
					add(path, size, ClientManager::getInstance()->getUser(nick), target, resume, p);
				} catch(Exception e) {
					// ...
				}
			}
			aXml->stepOut();
		}
		aXml->stepOut();
	}

	// We don't need to save the queue when we've just loaded it...
	setDirty(false);
}

void QueueManager::onAction(SearchManagerListener::Types type, SearchResult* sr) {
	
	if(type == SearchManagerListener::SEARCH_RESULT && BOOLSETTING(AUTO_SEARCH)) {
		StringList l = getTargetsBySize(sr->getSize());
		StringList tok = StringTokenizer(SearchManager::clean(sr->getFileName()), ' ').getTokens();

		for(StringIter i = l.begin(); i != l.end(); ++i) {
			bool found = true;

			for(StringIter j = tok.begin(); j != tok.end(); ++j) {
				if(Util::findSubString(*i, *j) == string::npos) {
					found = false;
					break;
				}
			}

			if(found) {
				// Wow! found a new source that seems to match...add it...
				dcdebug("QueueManager::onAction New source %s for target %s found\n", sr->getUser()->getNick().c_str(), i->c_str());
				try {
					add(sr->getFile(), sr->getSize(), sr->getUser(), *i);
				} catch(Exception e) {
					// ...
				}
			}
		}
	}
}

QueueItem* QueueManager::findByTarget(const string& aTarget) {
	// First, try an exact match (fast, since we're using a map...)
	QueueItem::StringIter i = queue.find(aTarget);
	if(i != queue.end())
		return i->second;

	// Then case-insensitive...
	for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
		if(stricmp(i->second->getTarget().c_str(), aTarget.c_str()) == 0)
			return i->second;
	}
	return NULL;
}

void QueueManager::importNMQueue(const string& aFile) throw(FileException) {
	File f(aFile, File::READ, File::OPEN);
	
	u_int32_t size = (u_int32_t)f.getSize();
	
	string tmp;
	if(size > 16) {
		u_int8_t* buf = new u_int8_t[size];
		f.read(buf, size);
		CryptoManager::getInstance()->decodeHuffman(buf, tmp);
		delete[] buf;
	} else {
		tmp = Util::emptyString;
	}
	
	StringTokenizer line(tmp);
	StringList& tokens = line.getTokens();
	
	for(StringIter i = tokens.begin(); i != tokens.end(); i++) {
		string& tok = *i;
		string::size_type k = tok.find('|');

		if( (k == string::npos) || ((k+1) >= tok.size()) )
			continue;

		if(tok.substr(k + 1).compare("Active") == 0 || tok.substr(k + 1).compare("Paused") == 0) {
			continue; // ignore first line
		}
		
		StringTokenizer t(*i, '\t');
		StringList& records = t.getTokens();
		
		if(records.size() < 5)
			continue;

		for(StringIter j = records.begin(); j != records.end(); ++j) {
			++j; // filename

			const string& size   = *(++j);
			const string& target = *(++j);
			const string& file   = *(++j);
			const string& nick   = *(++j);

			try {
				add(file, size, ClientManager::getInstance()->getUser(nick), target);
			} catch(Exception e) {
				// ...
			}
			break;			
		}
	}
		
	return;
}

/**
 * @file QueueManager.cpp
 * $Id: QueueManager.cpp,v 1.25 2002/05/12 21:54:08 arnetheduck Exp $
 */


