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

	{
		Lock l(cs);
		// Alright, get a queue item, new or old...
		bool newItem = false;
		QueueItem* q = getQueueItem(aFile, aTarget, aSize, aResume, newItem);
		QueueItem::Source* s = NULL;

		for(QueueItem::Source::Iter i = q->getSources().begin(); i != q->getSources().end(); ++i) {
			if( ((*i)->getUser() == aUser) || ((*i)->getUser()->getNick() == aUser->getNick() && (*i)->getPath() == aFile) ) {
				return;
			}
		}

		s = q->addSource(aUser, aFile);
		if(newItem && (p != QueueItem::DEFAULT)) {
			q->setPriority(p);
		}

		if(q->getStatus() != QueueItem::RUNNING) {
			QueueItem::UserListIter j = userQueue.find(aUser);
			if(j == userQueue.end()) {
				userQueue[aUser].resize(QueueItem::LAST);
				j = userQueue.find(aUser);
			}
			j->second[q->getPriority()].push_back(q);

		}

		if(newItem) {
			queue.insert(make_pair(q->getTarget(), q));
			fire(QueueManagerListener::ADDED, q);
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

		QueueItem::UserListIter i = userQueue.find(aUser);
		if(i == userQueue.end()) {
			return NULL;
		}
		int p = QueueItem::LAST - 1;
		do {
			if(!i->second[p].empty()) {
				q = i->second[p].front();
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
		d->setSize(q->getSize());
		return d;
	}
}

void QueueManager::putDownload(Download* aDownload, bool finished /* = false */) {
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
					if(userQueue.find((*j)->getUser()) == userQueue.end()) {
						userQueue[(*j)->getUser()].resize(QueueItem::LAST);
					}
					userQueue[(*j)->getUser()][q->getPriority()].push_back(q);
					
					if( (q->getPriority() != QueueItem::PAUSED) && (*j)->getUser()->isOnline()) {
						ConnectionManager::getInstance()->getDownloadConnection((*j)->getUser());
					}
				}
				fire(QueueManagerListener::STATUS_UPDATED, q);
			}
		}
 		aDownload->setUserConnection(NULL);
		delete aDownload;
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

void QueueManager::removeSource(const string& aTarget, User::Ptr& aUser, bool removeConn /* = true */)  {
	Lock l(cs);
	QueueItem* q = findByTarget(aTarget);
	string x;
	if(q != NULL) {
		dcassert(q->isSource(aUser));
		if(q->isSet(QueueItem::USER_LIST)) {
			remove(q->getTarget());
			return;
		}
		q->removeSource(aUser);
		if((q->getStatus() == QueueItem::RUNNING) && q->getCurrent()->getUser() == aUser) {
			x = q->getTarget();
			dcassert(running.find(aUser) != running.end());
			running.erase(aUser);
		} else if(q->getStatus() != QueueItem::RUNNING) {
			QueueItem::UserListIter i = userQueue.find(aUser);
			dcassert(i != userQueue.end());
			dcassert(find(i->second[q->getPriority()].begin(), i->second[q->getPriority()].end(), q) != i->second[q->getPriority()].end());
			i->second[q->getPriority()].erase(find(i->second[q->getPriority()].begin(), i->second[q->getPriority()].end(), q));
			int j = 0;
			for(; j < QueueItem::LAST; ++i) {
				if(!i->second[q->getPriority()].empty())
					break;
			}
			if(j == QueueItem::LAST) {
				userQueue.erase(i);
			}
		}
		fire(QueueManagerListener::SOURCES_UPDATED, q);
		dirty = true;
	}
	if(removeConn && !x.empty()) {
		DownloadManager::getInstance()->abortDownload(x);
	}
}

void QueueManager::removeAll(QueueItem* q) {
	for(QueueItem::Source::Iter i = q->getSources().begin(); i != q->getSources().end(); ++i) {
		QueueItem::UserListIter j = userQueue.find((*i)->getUser());
		dcassert(j != userQueue.end());
		dcassert(find(j->second[q->getPriority()].begin(), j->second[q->getPriority()].end(), q) != j->second[q->getPriority()].end());
		j->second[q->getPriority()].erase(find(j->second[q->getPriority()].begin(), j->second[q->getPriority()].end(), q));
		int i = 0;
		for(; i < QueueItem::LAST; i++) {
			if(!j->second[i].empty())
				break;
		}

		if(i == QueueItem::LAST) {
			userQueue.erase(j);
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
					QueueItem::UserListIter j = userQueue.find((*i)->getUser());
					dcassert(j != userQueue.end());
					dcassert(find(j->second[q->getPriority()].begin(), j->second[q->getPriority()].end(), q) != j->second[q->getPriority()].end());
					j->second[q->getPriority()].erase(find(j->second[q->getPriority()].begin(), j->second[q->getPriority()].end(), q));
					j->second[p].push_back(q);
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
					add(sr->getFile(), sr->getSize(), sr->getUser(), *i);
				} catch(Exception e) {
					// ...
				}
			}
		}
	}
}

// SettingsManagerListener
void QueueManager::onAction(SettingsManagerListener::Types type, SimpleXML* xml) {
	switch(type) {
	case SettingsManagerListener::LOAD: load(xml); break;
	}
}

// ClientManagerListener
void QueueManager::onAction(ClientManagerListener::Types type, const User::Ptr& aUser) {
	bool hasDown = false;
	switch(type) {
	case ClientManagerListener::USER_UPDATED:
		{
			Lock l(cs);
			QueueItem::UserListIter j = userQueue.find(aUser);
			if(j != userQueue.end()) {
				for(int k = 0; k < QueueItem::LAST; ++k) {
					for(QueueItem::Iter i = j->second[k].begin(); i != j->second[k].end(); ++i)
						fire(QueueManagerListener::SOURCES_UPDATED, *i);
					if(k != QueueItem::PAUSED && !j->second[k].empty())
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
 * $Id: QueueManager.cpp,v 1.28 2002/05/30 19:09:33 arnetheduck Exp $
 */
