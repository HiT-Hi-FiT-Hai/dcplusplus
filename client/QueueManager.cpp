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

		// We keep max 30 recent searches...
		while(recent.size() > 30) {
			recent.erase(recent.begin());
		}

		{
			Lock l(cs);
			if(queue.size() > 0) {
				bool searched = false;
				// We pick a start position at random, hoping that we will find something to search for...
				QueueItem::StringMap::size_type start = (QueueItem::StringMap::size_type)Util::rand(queue.size());

				QueueItem::StringIter i = queue.begin();
				advance(i, start);
				QueueItem::StringIter j = i;
				for(; j != queue.end(); ++j) {
					QueueItem* q = j->second;
					
					if( (q->isSet(QueueItem::USER_LIST)) || 
						(q->getStatus() == QueueItem::RUNNING) ||
						(q->getPriority() == QueueItem::PAUSED) ||
						(q->hasOnlineUsers()) ||
						(find(recent.begin(), recent.end(), q->getTarget()) != recent.end())
					  ) {
						continue;
					}
					searched = true;
					SearchManager::getInstance()->search(SearchManager::clean(q->getTargetFileName()), q->getSize() - 1, SearchManager::TYPE_ANY, SearchManager::SIZE_ATLEAST);
					recent.push_back(q->getTarget());
					break;
				}
				if(!searched) {
					for(j = queue.begin(); j != i; ++j) {
						QueueItem* q = j->second;

						if( (q->isSet(QueueItem::USER_LIST)) || 
							(q->getStatus() == QueueItem::RUNNING) ||
							(q->getPriority() == QueueItem::PAUSED) ||
							(q->hasOnlineUsers()) ||
							(find(recent.begin(), recent.end(), q->getTarget()) != recent.end())
							) {
								continue;
						}

						SearchManager::getInstance()->search(SearchManager::clean(q->getTargetFileName()), q->getSize() - 1, SearchManager::TYPE_ANY, SearchManager::SIZE_ATLEAST);
						recent.push_back(q->getTarget());
						break;
					}
				}
			}
		}
	}
}

enum { TEMP_LENGTH = 8 };
string QueueManager::getTempName(const string& aFileName) {
	string tmp(TEMP_LENGTH, ' ');
	for(int i = 0; i < TEMP_LENGTH; i++) {
		tmp[i] = 'a' + (char)( ((double)('z'-'a')) * (((double)rand()) / ((double)RAND_MAX) ) );
	}
	string::size_type j = aFileName.rfind('.');
	if(j == string::npos) {
		return aFileName + tmp;
	} else {
		return aFileName.substr(0, j) + tmp + aFileName.substr(j);
	}
}

void QueueManager::add(const string& aFile, int64_t aSize, User::Ptr aUser, const string& aTarget, 
					   bool aResume /* = true */, QueueItem::Priority p /* = QueueItem::DEFAULT */,
					   const string& aTempTarget /* = Util::emptyString */, bool addBad /* = true */) throw(QueueException, FileException) {
	// Check that we're not downloading from ourselves...
	if(aUser->getClientNick() == aUser->getNick()) {
		throw QueueException(STRING(NO_DOWNLOADS_FROM_SELF));
	}

	if(aUser->isSet(User::PASSIVE) && (SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_PASSIVE) ) {
		throw QueueException(STRING(CANT_CONNECT_IN_PASSIVE_MODE));
	}

	// Check that the file doesn't already exist...
	if( (aSize != -1) && (aSize <= File::getSize(aTarget)) )  {
		throw FileException(STRING(LARGER_TARGET_FILE_EXISTS));
	}

	{
		Lock l(cs);
		// Alright, get a queue item, new or old...
		bool newItem = false;
		QueueItem* q = getQueueItem(aFile, aTarget, aSize, aResume, newItem);
		QueueItem::Source* s = NULL;

		{
			for(QueueItem::Source::Iter i = q->getSources().begin(); i != q->getSources().end(); ++i) {
				if( ((*i)->getUser() == aUser) || ((*i)->getUser()->getNick() == aUser->getNick() && (*i)->getPath() == aFile) ) {
					return;
				}
			}
		}
		if(!addBad) {
			for(QueueItem::Source::Iter i = q->getBadSources().begin(); i != q->getBadSources().end(); ++i) {
				if( ((*i)->getUser() == aUser) || ((*i)->getUser()->getNick() == aUser->getNick() && (*i)->getPath() == aFile) ) {
					return;
				}
			}
		}

		s = q->addSource(aUser, aFile);
		if(newItem && (p != QueueItem::DEFAULT)) {
			q->setPriority(p);
		}

		if(q->getStatus() != QueueItem::RUNNING) {
			userQueue[q->getPriority()][aUser].push_back(q);
		}

		if(newItem) {
			queue.insert(make_pair(q->getTarget(), q));
			fire(QueueManagerListener::ADDED, q);

			if(!q->isSet(QueueItem::USER_LIST)) {
				if(aTempTarget.empty()) {
					if(!SETTING(TEMP_DOWNLOAD_DIRECTORY).empty() && (File::getSize(q->getTarget()) == -1)) {
						q->setTempTarget(SETTING(TEMP_DOWNLOAD_DIRECTORY) + getTempName(q->getTargetFileName()));
					}
				} else {
					q->setTempTarget(aTempTarget);
				}
			}
		}

		fire(QueueManagerListener::SOURCES_UPDATED, q);
		dirty = true;
	}

	if(aUser->isOnline())
		ConnectionManager::getInstance()->getDownloadConnection(aUser);
}

QueueItem* QueueManager::getQueueItem(const string& aFile, const string& aTarget, int64_t aSize, bool aResume, bool& newItem) throw(QueueException, FileException) {
	QueueItem* q = findByTarget(aTarget);
	
	if(q == NULL) {
		newItem = true;

		q = new QueueItem(aTarget, aSize, (aSize <= 16*1024) ? QueueItem::HIGHEST : QueueItem::NORMAL, aResume);

		if(aFile == USER_LIST_NAME) {
			q->setFlag(QueueItem::USER_LIST);
			q->setPriority(QueueItem::HIGHEST);
		}
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

		QueueItem::UserListIter i;
		int p = QueueItem::LAST - 1;
		do {
			i = userQueue[p].find(aUser);
			if(i != userQueue[p].end()) {
				dcassert(!i->second.empty());
				q = i->second.front();
				break;
			}
		} while(--p > QueueItem::PAUSED);

		if(q == NULL)
			return NULL;

		// Remove the download from all active lists...
		removeAll(q);
		// Move the download to the running list...
		dcassert(running.find(aUser) == running.end());
		running[aUser] = q;

		// Set the flag to running, so that we don't get the same download twice...
		q->setStatus(QueueItem::RUNNING);
		q->setCurrent(aUser);
		
		fire(QueueManagerListener::STATUS_UPDATED, q);
		
		Download* d = new Download(q->isSet(QueueItem::RESUME), q->isSet(QueueItem::USER_LIST));
		d->setSource(q->getCurrent()->getPath());
		d->setTarget(q->getTarget());
		d->setTempTarget(q->getTempTarget());
		d->setSize(q->getSize());
		return d;
	}
}

void QueueManager::putDownload(Download* aDownload, bool finished /* = false */) {
	User::List getConn;
	{
		Lock l(cs);
		QueueItem* q = findByTarget(aDownload->getTarget());

		if(q != NULL) {
			// Remove from the running list...
			QueueItem::UserIter i = running.find(q->getCurrent()->getUser());
			if(i != running.end()) {
				dcassert(i->second == q);
				running.erase(i);
			}

			if(finished) {
				queue.erase(q->getTarget());
				fire(QueueManagerListener::REMOVED, q);
				delete q;
				dirty = true;
			} else {
				q->setStatus(QueueItem::WAITING);
				q->setCurrent(NULL);
				for(QueueItem::Source::Iter j = q->getSources().begin(); j != q->getSources().end(); ++j) {
					userQueue[q->getPriority()][(*j)->getUser()].push_back(q);

					if( (q->getPriority() != QueueItem::PAUSED) && (*j)->getUser()->isOnline()) {
						getConn.push_back((*j)->getUser());
					}
				}
				fire(QueueManagerListener::STATUS_UPDATED, q);
			}
		}
 		aDownload->setUserConnection(NULL);
		delete aDownload;
	}

	for(User::Iter i = getConn.begin(); i != getConn.end(); ++i) {
		ConnectionManager::getInstance()->getDownloadConnection(*i);
	}
}

void QueueManager::remove(const string& aTarget) throw(QueueException) {
	string x;
	{
		Lock l(cs);

		QueueItem* q = findByTarget(aTarget);
		if(q != NULL) {
			queue.erase(q->getTarget());
			if(q->getStatus() == QueueItem::RUNNING) {
				x = q->getTarget();
				dcassert(running.find(q->getCurrent()->getUser()) != running.end());
				running.erase(q->getCurrent()->getUser());
			} else {
				removeAll(q);
			}
			fire(QueueManagerListener::REMOVED, q);
			delete q;
			dirty = true;
		}
	}
	if(!x.empty()) {
		DownloadManager::getInstance()->abortDownload(x);
	}
}

void QueueManager::removeSource(const string& aTarget, User::Ptr& aUser, int reason, bool removeConn /* = true */)  {
	Lock l(cs);
	QueueItem* q = findByTarget(aTarget);
	string x;
	if(q != NULL) {
		dcassert(q->isSource(aUser));
		if(q->isSet(QueueItem::USER_LIST)) {
			remove(q->getTarget());
			return;
		}
		q->removeSource(aUser, reason);
		if((q->getStatus() == QueueItem::RUNNING) && q->getCurrent()->getUser() == aUser) {
			x = q->getTarget();
			dcassert(running.find(aUser) != running.end());
			running.erase(aUser);
		} else if(q->getStatus() != QueueItem::RUNNING) {
			QueueItem::UserListIter i = userQueue[q->getPriority()].find(aUser);
			dcassert(i != userQueue[q->getPriority()].end());
			dcassert(find(i->second.begin(), i->second.end(), q) != i->second.end());
			i->second.erase(find(i->second.begin(), i->second.end(), q));
			if(i->second.empty()) {
				userQueue[q->getPriority()].erase(i);
			}
		}
		fire(QueueManagerListener::SOURCES_UPDATED, q);
		dirty = true;
	}
	if(removeConn && !x.empty()) {
		DownloadManager::getInstance()->abortDownload(x);
	}
}

void QueueManager::removeSources(User::Ptr& aUser, int reason)  {
	string x;
	{
		Lock l(cs);
		{
			for(int i = 0; i < QueueItem::LAST; i++) {
				QueueItem::UserListIter k = userQueue[i].find(aUser);
				if(k != userQueue[i].end()) {
					for(QueueItem::Iter j = k->second.begin(); j != k->second.end(); ++j) {
						(*j)->removeSource(aUser, reason);
						fire(QueueManagerListener::SOURCES_UPDATED, *j);
					}
					userQueue[i].erase(aUser);
				}
			}
		}
		{
			QueueItem::UserIter j = running.find(aUser);
			if(j != running.end()) {
				j->second->removeSource(aUser, reason);
				fire(QueueManagerListener::SOURCES_UPDATED, j->second);
				x = j->second->getTarget();
			}
		}
	}
	if(!x.empty()) {
		DownloadManager::getInstance()->abortDownload(x);
	}
}

void QueueManager::removeAll(QueueItem* q) {
	for(QueueItem::Source::Iter i = q->getSources().begin(); i != q->getSources().end(); ++i) {
		QueueItem::UserListIter j = userQueue[q->getPriority()].find((*i)->getUser());
		dcassert(j != userQueue[q->getPriority()].end());
		dcassert(find(j->second.begin(), j->second.end(), q) != j->second.end());
		j->second.erase(find(j->second.begin(), j->second.end(), q));
		if(j->second.empty()) {
			userQueue[q->getPriority()].erase(j);
		}
	}
}

void QueueManager::setPriority(const string& aTarget, QueueItem::Priority p) throw() {
	{
		Lock l(cs);
	
		QueueItem* q = findByTarget(aTarget);
		if( (q != NULL) && (q->getPriority() != p) ) {
			if( q->getStatus() != QueueItem::RUNNING ) {
				for(QueueItem::Source::Iter i = q->getSources().begin(); i != q->getSources().end(); ++i) {
					QueueItem::UserListIter j = userQueue[q->getPriority()].find((*i)->getUser());
					dcassert(j != userQueue[q->getPriority()].end());
					dcassert(find(j->second.begin(), j->second.end(), q) != j->second.end());
					j->second.erase(find(j->second.begin(), j->second.end(), q));
					if(j->second.empty()) {
						userQueue[q->getPriority()].erase(j);
					}
					userQueue[p][(*i)->getUser()].push_back(q);
					if(q->getPriority() == QueueItem::PAUSED && (*i)->getUser()->isOnline()) {
						ConnectionManager::getInstance()->getDownloadConnection((*i)->getUser());
					}
				}
			}
			q->setPriority(p);
			fire(QueueManagerListener::STATUS_UPDATED, q);
		}
	}
}

QueueItem* QueueManager::findByTarget(const string& aTarget) {
	// The hash is (hopefully...) case-insensitive...
	QueueItem::StringIter i = queue.find(aTarget);
	if(i != queue.end())
		return i->second;
	return NULL;
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
				xml.addChildAttrib("TempTarget", d->getTempTarget());
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
			const string& tempTarget = aXml->getChildAttrib("TempTarget");
			const string& target = aXml->getChildAttrib("Target");
			bool resume = aXml->getBoolChildAttrib("Resume");
			int64_t size = aXml->getLongLongChildAttrib("Size");
			QueueItem::Priority p = (QueueItem::Priority)aXml->getIntChildAttrib("Priority");
			aXml->stepIn();
			while(aXml->findChild("Source")) {
				const string& nick = aXml->getChildAttrib("Nick");
				const string& path = aXml->getChildAttrib("Path");
				
				try {
					add(path, size, ClientManager::getInstance()->getUser(nick), target, resume, p, tempTarget);
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
		const string& tok = *i;
		string::size_type k = tok.find('|');

		if( (k == string::npos) || ((k+1) >= tok.size()) )
			continue;

		string tmp = tok.substr(k+1);
		if( (tmp == "Active") || (tmp == "Paused") ) {
			continue; // ignore first line
		}
		
		StringTokenizer t(tok, '\t');
		StringList& records = t.getTokens();
		
		if(records.size() < 5)
			continue;

		StringIter j = records.begin();
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
	}
		
	return;
}

// SearchManagerListener
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
					add(sr->getFile(), sr->getSize(), sr->getUser(), *i, true, 
						QueueItem::DEFAULT, Util::emptyString, false);
				} catch(Exception e) {
					// ...
				}
			}
		}
	}
}

// ClientManagerListener
void QueueManager::onAction(ClientManagerListener::Types type, const User::Ptr& aUser) {
	bool hasDown = false;
	switch(type) {
	case ClientManagerListener::USER_UPDATED:
		{
			Lock l(cs);
			for(int i = 0; i < QueueItem::LAST; ++i) {
				QueueItem::UserListIter j = userQueue[i].find(aUser);
				if(j != userQueue[i].end()) {
					for(QueueItem::Iter m = j->second.begin(); m != j->second.end(); ++m)
						fire(QueueManagerListener::SOURCES_UPDATED, *m);
					if(i != QueueItem::PAUSED)
						hasDown = true;
				}
			}
		}
		break;
	}
	if(aUser->isOnline() && hasDown)	
		ConnectionManager::getInstance()->getDownloadConnection(aUser);

}

void QueueManager::onAction(TimerManagerListener::Types type, u_int32_t aTick) {
	switch(type) {
	case TimerManagerListener::MINUTE:
		onTimerMinute(aTick); break;
	case TimerManagerListener::SECOND:
		if(dirty && ((lastSave + 10000) < aTick)) {
			saveQueue();
		}
		break;
	}
}

/**
 * @file QueueManager.cpp
 * $Id: QueueManager.cpp,v 1.34 2002/06/29 18:58:49 arnetheduck Exp $
 */
