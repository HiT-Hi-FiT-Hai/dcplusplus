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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "QueueManager.h"
#include "ConnectionManager.h"
#include "SearchManager.h"
#include "ClientManager.h"
#include "DownloadManager.h"
#include "UserConnection.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "DirectoryListing.h"
#include "CryptoManager.h"
#include "ShareManager.h"
#include "DirectoryListing.h"

QueueManager* Singleton<QueueManager>::instance = NULL;

const string QueueManager::USER_LIST_NAME = "MyList.DcLst";

void QueueManager::UserQueue::add(QueueItem* qi) {
	for(QueueItem::Source::Iter i = qi->getSources().begin(); i != qi->getSources().end(); ++i) {
		add(qi, (*i)->getUser());
	}
}

void QueueManager::UserQueue::add(QueueItem* qi, const User::Ptr& aUser) {
	dcassert(qi->getStatus() == QueueItem::STATUS_WAITING);
	dcassert(qi->isSource(aUser));
	dcassert(qi->getCurrent() == NULL);

	QueueItem::List& l = userQueue[qi->getPriority()][aUser];
	if(qi->isSet(QueueItem::FLAG_EXISTS)) {
		l.insert(l.begin(), qi);
	} else {
		l.push_back(qi);
	}
}

QueueItem* QueueManager::UserQueue::getNext(const User::Ptr& aUser, QueueItem::Priority minPrio) {
	int p = QueueItem::LAST - 1;

	do {
		QueueItem::UserListIter i = userQueue[p].find(aUser);
		if(i != userQueue[p].end()) {
			dcassert(!i->second.empty());
			return i->second.front();
		}
		p--;
	} while(p >= minPrio);

	return NULL;
}

void QueueManager::UserQueue::setRunning(QueueItem* qi, const User::Ptr& aUser) {
	dcassert(qi->getCurrent() == NULL);
	dcassert(qi->getStatus() == QueueItem::STATUS_WAITING);

	// Remove the download from the userQueue...
	remove(qi);
	
	// Set the flag to running...
	qi->setStatus(QueueItem::STATUS_RUNNING);
	qi->setCurrent(aUser);

	// Move the download to the running list...
	dcassert(running.find(aUser) == running.end());
	running[aUser] = qi;

}

void QueueManager::UserQueue::setWaiting(QueueItem* qi) {
	dcassert(qi->getCurrent() != NULL);
	dcassert(qi->getStatus() == QueueItem::STATUS_RUNNING);

	dcassert(running.find(qi->getCurrent()->getUser()) != running.end());
	// Remove the download from running
	running.erase(qi->getCurrent()->getUser());

	// Set flag to waiting
	qi->setCurrent(NULL);
	qi->setStatus(QueueItem::STATUS_WAITING);

	// Add to the userQueue
	add(qi);
}

QueueItem* QueueManager::UserQueue::getRunning(const User::Ptr& aUser) {
	QueueItem::UserIter i = running.find(aUser);
	return (i == running.end()) ? NULL : i->second;
}

void QueueManager::UserQueue::remove(QueueItem* qi) {
	if(qi->getStatus() == QueueItem::STATUS_RUNNING) {
		dcassert(qi->getCurrent() != NULL);
		remove(qi, qi->getCurrent()->getUser());
	} else {
		for(QueueItem::Source::Iter i = qi->getSources().begin(); i != qi->getSources().end(); ++i) {
			remove(qi, (*i)->getUser());
		}
	}
}

void QueueManager::UserQueue::remove(QueueItem* qi, const User::Ptr& aUser) {
	if(qi->getStatus() == QueueItem::STATUS_RUNNING) {
		// Remove from running...
		dcassert(qi->getCurrent() != NULL);
		dcassert(running.find(aUser) != running.end());
		running.erase(aUser);
	} else {
		dcassert(qi->isSource(aUser));
		dcassert(qi->getCurrent() == NULL);
		QueueItem::UserListMap& ulm = userQueue[qi->getPriority()];
		QueueItem::UserListIter j = ulm.find(aUser);
		dcassert(j != ulm.end());
		QueueItem::List& l = j->second;
		dcassert(find(l.begin(), l.end(), qi) != l.end());
		l.erase(find(l.begin(), l.end(), qi));
		
		if(l.empty()) {
			ulm.erase(j);
		}
	}
}

QueueManager::QueueManager() : dirty(false), searched(false), lastSave(0), queueFile(Util::getAppPath() + "Queue.xml") { 
	TimerManager::getInstance()->addListener(this); 
	SearchManager::getInstance()->addListener(this);
	ClientManager::getInstance()->addListener(this);
	Util::ensureDirectory(Util::getAppPath() + "FileLists\\");
	lastInsert = queue.end();
};

QueueManager::~QueueManager() { 
	SearchManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this); 
	ClientManager::getInstance()->removeListener(this);

	saveQueue();

#ifdef WIN32

	if(!BOOLSETTING(KEEP_LISTS)) {
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
	}
#endif
	for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
		delete i->second;
	}
};

void QueueManager::onTimerMinute(u_int32_t /*aTick*/) {
	if(BOOLSETTING(AUTO_SEARCH)) {
		if(searched) {
			searched = false;
			return;
		}

		// We keep max 30 recent searches...this way we get an hour between
		// each duplicate search.
		while(recent.size() > 30) {
			recent.erase(recent.begin());
		}

		string fn;
		int64_t sz = 0;

		{
			Lock l(cs);
			if(queue.size() > 0) {
				// We pick a start position at random, hoping that we will find something to search for...
				QueueItem::StringMap::size_type start = (QueueItem::StringMap::size_type)Util::rand(queue.size());

				QueueItem::StringIter i = queue.begin();
				advance(i, start);
				QueueItem::StringIter j = i;
				for(; j != queue.end(); ++j) {
					QueueItem* q = j->second;
					
					if( (q->isSet(QueueItem::FLAG_USER_LIST)) || 
						(q->getStatus() == QueueItem::STATUS_RUNNING) ||
						(q->getPriority() == QueueItem::PAUSED) ||
						(q->hasOnlineUsers()) ||
						(find(recent.begin(), recent.end(), q->getTargetFileName()) != recent.end())
					  ) {
						continue;
					}
					searched = true;
					fn = q->getTargetFileName();
					sz = q->getSize() - 1;
					recent.push_back(q->getTargetFileName());
					break;
				}

				if(!searched) {
					for(j = queue.begin(); j != i; ++j) {
						QueueItem* q = j->second;

						if( (q->isSet(QueueItem::FLAG_USER_LIST)) || 
							(q->getStatus() == QueueItem::STATUS_RUNNING) ||
							(q->getPriority() == QueueItem::PAUSED) ||
							(q->hasOnlineUsers()) ||
							(find(recent.begin(), recent.end(), q->getTargetFileName()) != recent.end())
							) {
								continue;
						}

						fn = q->getTargetFileName();
						sz = q->getSize() - 1;
						recent.push_back(q->getTargetFileName());
						searched = true;
						break;
					}
				}
			}
		}

		if(!fn.empty())
			SearchManager::getInstance()->search(SearchManager::clean(fn), sz, ShareManager::getInstance()->getType(fn), SearchManager::SIZE_ATLEAST);
	}
}

enum { TEMP_LENGTH = 8 };
string QueueManager::getTempName(const string& aFileName) {
	string tmp;
	tmp.reserve(aFileName.length() + TEMP_LENGTH);
	string::size_type j = aFileName.rfind('.');
	
	if(j == string::npos) {
		tmp.append(aFileName);
	} else {
		tmp.append(aFileName.data(), j);
	}
	for(int i = 0; i < TEMP_LENGTH; i++) {
		tmp.append(1, (char)Util::rand('a', 'z'));
	}
	if(j != string::npos) {
		tmp.append(aFileName.data() + j, aFileName.length() - j);
	}
	return tmp;
}

void QueueManager::add(const string& aFile, int64_t aSize, User::Ptr aUser, const string& aTarget, 
					   int aFlags /* = QueueItem::FLAG_RESUME */, QueueItem::Priority p /* = QueueItem::DEFAULT */,
					   const string& aTempTarget /* = Util::emptyString */, bool addBad /* = true */) throw(QueueException, FileException) {
	
	bool wantConnection = true;
	
	// Check that we're not downloading from ourselves...
	if(aUser->getClientNick() == aUser->getNick()) {
		throw QueueException(STRING(NO_DOWNLOADS_FROM_SELF));
	}

#ifdef WIN32
	if(aTarget.length() > MAX_PATH) {
		throw QueueException(STRING(TARGET_FILENAME_TOO_LONG));
	}
#endif

	// Check that target contains at least one directory...we don't want headless files...
	if(aTarget.find('\\') == string::npos) {
		throw QueueException(STRING(INVALID_TARGET_FILE));
	}

	string target = Util::filterFileName(aTarget);
	// Check that the file doesn't already exist...
	if( (aSize != -1) && (aSize <= File::getSize(target)) )  {
		throw FileException(STRING(LARGER_TARGET_FILE_EXISTS));
	}

	// Check if it's a zero-byte file, if so, create and return...
	if(aSize == 0) {
		if(!BOOLSETTING(SKIP_ZERO_BYTE)) {
			File f(aTarget, File::WRITE, File::CREATE);
		}
		return;
	}

	{
		Lock l(cs);
		// Alright, get a queue item, new or old...
		bool newItem = false;
		QueueItem* q = getQueueItem(target, aSize, aFlags, newItem);
		QueueItem::Source* s = NULL;
		
		dcassert((aFile == USER_LIST_NAME && q->isSet(QueueItem::FLAG_USER_LIST)) || (aFile != USER_LIST_NAME));

		if(!newItem && q->isSet(QueueItem::FLAG_USER_LIST)) {
			// We've done what we could...
			return;
		}
		
		if(q->isSource(aUser)) {
			throw QueueException(STRING(DUPLICATE_SOURCE));
		}

		{
			for(QueueItem::Source::Iter i = q->getSources().begin(); i != q->getSources().end(); ++i) {
				if(((*i)->getUser()->getNick() == aUser->getNick()) && ((*i)->getPath() == aFile) ) {
					throw QueueException(STRING(DUPLICATE_SOURCE));
				}
			}
		}

		if(!addBad) {
			if(q->isBadSource(aUser)) {
				throw QueueException(STRING(DUPLICATE_SOURCE));
			}
			for(QueueItem::Source::Iter i = q->getBadSources().begin(); i != q->getBadSources().end(); ++i) {
				if( ((*i)->getUser()->getNick() == aUser->getNick() && (*i)->getPath() == aFile) ) {
					throw QueueException(STRING(DUPLICATE_SOURCE));
				}
			}
		}

		s = q->addSource(aUser, aFile);
		
		if(newItem) {
			if(p != QueueItem::DEFAULT) {
				q->setPriority(p);
			}

			if(!q->isSet(QueueItem::FLAG_USER_LIST)) {
				if(aTempTarget.empty()) {
					if(!SETTING(TEMP_DOWNLOAD_DIRECTORY).empty() && (File::getSize(q->getTarget()) == -1)) {
						q->setTempTarget(SETTING(TEMP_DOWNLOAD_DIRECTORY) + getTempName(q->getTargetFileName()));
					}
				} else {
					q->setTempTarget(aTempTarget);
				}
			}

			if(lastInsert == queue.end()) {
				lastInsert = queue.insert(make_pair(q->getTarget(), q)).first;
			} else {
				lastInsert = queue.insert(lastInsert, make_pair(q->getTarget(), q));
			}
			fire(QueueManagerListener::ADDED, q);
		}

		if(q->getPriority() == QueueItem::PAUSED) {
			wantConnection = false;
		}

		const string& tgt = q->getTempTarget().empty() ? q->getTarget() : q->getTempTarget();
	
		if(File::getSize(tgt) > 0)
			q->setFlag(QueueItem::FLAG_EXISTS);

		if(aUser->isSet(User::PASSIVE) && (SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE) ) {
			q->removeSource(aUser, QueueItem::Source::FLAG_PASSIVE);
			wantConnection = false;
		} else if(q->getStatus() != QueueItem::STATUS_RUNNING) {
			userQueue.add(q, aUser);
		} else {
			wantConnection = false;
		}

		fire(QueueManagerListener::SOURCES_UPDATED, q);
		setDirty();
	}

	if(wantConnection && aUser->isOnline())
		ConnectionManager::getInstance()->getDownloadConnection(aUser);
}

QueueItem* QueueManager::getQueueItem(const string& aTarget, int64_t aSize, int aFlags, bool& newItem) throw(QueueException, FileException) {
	QueueItem* q;
	if((lastInsert != queue.end()) && (lastInsert->first == aTarget)) {
		q = lastInsert->second;
	} else {
		q = findByTarget(aTarget);
	}
		
	if(q == NULL) {
		newItem = true;

		q = new QueueItem(aTarget, aSize, (aSize <= 16*1024) ? QueueItem::HIGHEST : QueueItem::NORMAL, aFlags);

	} else {
		newItem = false;
		if(q->getSize() != aSize) {
			throw QueueException(STRING(FILE_WITH_DIFFERENT_SIZE));
		}
		q->setFlag(aFlags);
	}

	return q;
}

void QueueManager::addDirectory(const string& aDir, const User::Ptr& aUser, const string& aTarget, QueueItem::Priority p /* = QueueItem::DEFAULT */) throw() {
	bool needList;
	{
		Lock l(cs);
		
		DirectoryItem::DirectoryPair dp = directories.equal_range(aUser);
		
		for(DirectoryItem::DirectoryIter i = dp.first; i != dp.second; ++i) {
			if(Util::stricmp(aTarget.c_str(), i->second->getName().c_str()) == 0)
				return;
		}
		
		// Unique directory, fine...
		directories.insert(make_pair(aUser, new DirectoryItem(aUser, aDir, aTarget, p)));
		needList = (dp.first == dp.second);
		setDirty();
	}

	if(needList) {
		try {
			addList(aUser, QueueItem::FLAG_DIRECTORY_DOWNLOAD);
		} catch(const Exception&) {
			// Ignore, we don't really care...
		}
	}
}

typedef vector<pair<DirectoryListing::File*, string> > MatchList;
typedef MatchList::iterator MatchIter;
typedef HASH_MULTIMAP<u_int32_t, QueueItem*> SizeMap;
typedef SizeMap::iterator SizeIter;
typedef pair<SizeIter, SizeIter> SizePair;

#define isnum(c) (((c) >= '0') && ((c) <= '9'))

static inline int adjustSize(u_int32_t sz, const string& name) {
	if(sz == 15 * 1000 * 1000 || sz == 50 * 1000 * 1000) {
		// Common rar-set size...handle specially because it makes
		// the hash map ineffective
		dcassert(name.length() > 2);
		// filename.r32
		u_int8_t c1 = (u_int8_t)name[name.length()-2];
		u_int8_t c2 = (u_int8_t)name[name.length()-1];
		if(isnum(c1) && isnum(c2)) {
			return (c1-'0')*10 + (c2-'0');
		} else if(name.length() > 6) {
			// filename.part32.rar
			c1 = name[name.length() - 6];
			c2 = name[name.length() - 5];
			if(isnum(c1) && isnum(c2)) {
				return (c1-'0')*10 + (c2-'0');
			}
		}
	} 

	return 0;
}

static void matchFiles(SizeMap& sizeMap, MatchList& f, DirectoryListing::Directory* dir) throw() {
	for(DirectoryListing::Directory::Iter j = dir->directories.begin(); j != dir->directories.end(); ++j) {
		if(!(*j)->getAdls())
			matchFiles(sizeMap, f, *j);
	}

	for(DirectoryListing::File::Iter i = dir->files.begin(); i != dir->files.end(); ++i) {
		DirectoryListing::File* df = *i;

		SizePair files = sizeMap.equal_range(((u_int32_t)df->getSize()) + adjustSize((u_int32_t)df->getSize(), df->getName()));
		for(SizeIter j = files.first; j != files.second; ++j) {
			QueueItem* qi = j->second;
			if(Util::stricmp(df->getName(), qi->getTargetFileName()) == 0 && df->getSize() == qi->getSize())
				f.push_back(make_pair(df, qi->getTarget()));
		}
	}
}

int QueueManager::matchListing(DirectoryListing* dl) throw() {
	MatchList f;
	{
		Lock l(cs);
		SizeMap sizeMap;
		for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
			QueueItem* qi = i->second;
			if(qi->getSize() != -1) {
				sizeMap.insert(make_pair(((u_int32_t)qi->getSize()) + adjustSize((u_int32_t)qi->getSize(), qi->getTarget()), qi));
			}
		}

		matchFiles(sizeMap, f, dl->getRoot());

	}
	int totalItems = (int)f.size();
	for(MatchIter i = f.begin(); i != f.end(); ++i) {
		DirectoryListing::File* df = i->first;
		try {
			add(dl->getPath(df) + df->getName(), df->getSize(), dl->getUser(), i->second);
		} catch(const Exception&) {
			totalItems--;
		}
	}
	return totalItems;
}

void QueueManager::move(const string& aSource, const string& aTarget) throw() {
	if(Util::stricmp(aSource.c_str(), aTarget.c_str()) == 0)
		return;

	QueueItem* copy = NULL;

	Lock l(cs);
	QueueItem* qs = findByTarget(aSource);
	if(qs != NULL) {
		// Don't move running downloads
		if(qs->getStatus() == QueueItem::STATUS_RUNNING) {
			return;
		}
		// Don't move file lists
		if(qs->isSet(QueueItem::FLAG_USER_LIST))
			return;

		// Let's see if the target exists...then things get complicated...
		QueueItem* qt = findByTarget(aTarget);
		if(qt == NULL) {
			// Good, update the target and move in the queue...
			queue.erase(aSource);
			qs->setTarget(aTarget);
			lastInsert = queue.insert(make_pair(qs->getTarget(), qs)).first;
			fire(QueueManagerListener::MOVED, qs);
			setDirty();
		} else {
			// Don't move to target of different size
			if(qs->getSize() != qt->getSize())
				return;

			copy = new QueueItem(*qs);
		}
	}

	if(copy != NULL) {
		remove(aSource);
		// Damn...difficult...we add all sources then remove the old download...
		for(QueueItem::Source::Iter i = copy->getSources().begin(); i != copy->getSources().end(); ++i) {
			try {
				QueueItem::Source* s = *i;
				add(s->getPath(), copy->getSize(), s->getUser(), aTarget);
			} catch(const Exception&) {
				// Blah, ignore...
			}
		}
		delete copy;
	}
}


Download* QueueManager::getDownload(User::Ptr& aUser) throw() {
	Lock l(cs);

	QueueItem* q = userQueue.getNext(aUser);

	if(q == NULL)
		return NULL;

	userQueue.setRunning(q, aUser);

	fire(QueueManagerListener::STATUS_UPDATED, q);
	
	return new Download(q);
}


void QueueManager::putDownload(Download* aDownload, bool finished /* = false */) throw() {
	User::List getConn;
	string file;
	User::Ptr up;
	bool isBZ = false;
	int flag = 0;

	{
		Lock l(cs);
		QueueItem* q = findByTarget(aDownload->getTarget());

		if(q != NULL) {
			if(finished) {
				dcassert(q->getStatus() == QueueItem::STATUS_RUNNING);
				userQueue.remove(q);
				queue.erase(q->getTarget());
				lastInsert = queue.end();
				if(aDownload->isSet(Download::FLAG_USER_LIST) && aDownload->getSource() == "MyList.bz2") {
					q->setFlag(QueueItem::FLAG_BZLIST);
					isBZ = true;
				}
				fire(QueueManagerListener::FINISHED, q);
				fire(QueueManagerListener::REMOVED, q);
				// Now, let's see if this was a directory download filelist...
				if( (q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD) && directories.find(q->getCurrent()->getUser()) != directories.end()) ||
					(q->isSet(QueueItem::FLAG_MATCH_QUEUE)) ) {
					try {
						string fname = q->isSet(QueueItem::FLAG_BZLIST) ? aDownload->getTarget().substr(0, aDownload->getTarget().length() - 5) + "bz2" : aDownload->getTarget();
						file = File(fname, File::READ, File::OPEN).read();
						up = q->getCurrent()->getUser();
						flag = (q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD) ? QueueItem::FLAG_DIRECTORY_DOWNLOAD : 0)
							| (q->isSet(QueueItem::FLAG_MATCH_QUEUE) ? QueueItem::FLAG_MATCH_QUEUE : 0);
					} catch(const FileException&) {
						// ...
					}

				} 
				delete q;
				setDirty();
			} else {
				if(q->getPriority() != QueueItem::PAUSED) {
					for(QueueItem::Source::Iter j = q->getSources().begin(); j != q->getSources().end(); ++j) {
						if((*j)->getUser()->isOnline()) {
							getConn.push_back((*j)->getUser());
						}
					}
				}

				// This might have been set to wait by removesource already...
				if(q->getStatus() == QueueItem::STATUS_RUNNING) {
					userQueue.setWaiting(q);
					fire(QueueManagerListener::STATUS_UPDATED, q);
				}
				if(q->isSet(QueueItem::FLAG_USER_LIST)) {
					// Blah...no use keeping an unfinished file list...
					string fname = q->isSet(QueueItem::FLAG_BZLIST) ? aDownload->getTarget().substr(0, aDownload->getTarget().length() - 5) + "bz2" : aDownload->getTarget();
					File::deleteFile(fname);
				}
			}
		} else if(!aDownload->getTempTarget().empty() && aDownload->getTempTarget() != aDownload->getTarget()) {
			File::deleteFile(aDownload->getTempTarget());
		}
		aDownload->setUserConnection(NULL);
		delete aDownload;
	}

	for(User::Iter i = getConn.begin(); i != getConn.end(); ++i) {
		ConnectionManager::getInstance()->getDownloadConnection(*i);
	}

	if(!file.empty()) {
		string userList;
		try {
			if(isBZ) {
				CryptoManager::getInstance()->decodeBZ2((u_int8_t*)file.c_str(), file.size(), userList);
			} else {
				CryptoManager::getInstance()->decodeHuffman((u_int8_t*)file.c_str(), userList);
			}
		} catch(const CryptoException&) {
			addList(up, flag);
			return;
		}

		DirectoryListing dirList(up);
		dirList.load(userList);

		if(flag & QueueItem::FLAG_DIRECTORY_DOWNLOAD) {
			DirectoryItem::List dl;
			{
				Lock l(cs);
				DirectoryItem::DirectoryPair dp = directories.equal_range(up);
				for(DirectoryItem::DirectoryIter i = dp.first; i != dp.second; ++i) {
					dl.push_back(i->second);
				}
				directories.erase(up);
			}

			for(DirectoryItem::Iter i = dl.begin(); i != dl.end(); ++i) {
				DirectoryItem* di = *i;
				dirList.download(di->getName(), di->getTarget());
				delete di;
			}
		}
		if(flag & QueueItem::FLAG_MATCH_QUEUE) {
			matchListing(&dirList);
		}
	}
}

void QueueManager::remove(const string& aTarget) throw() {
	string x;
	{
		Lock l(cs);

		QueueItem* q = findByTarget(aTarget);
		if(q != NULL) {
			if(q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD)) {
				dcassert(q->getSources().size() == 1);
				DirectoryItem::DirectoryPair dp = directories.equal_range(q->getSources()[0]->getUser());
				for(DirectoryItem::DirectoryIter i = dp.first; i != dp.second; ++i) {
					delete i->second;
				}
				directories.erase(q->getSources()[0]->getUser());
			}

			queue.erase(q->getTarget());
			lastInsert = queue.end();
			if(q->getStatus() == QueueItem::STATUS_RUNNING) {
				x = q->getTarget();
			} else if(!q->getTempTarget().empty() && q->getTempTarget() != q->getTarget()) {
				File::deleteFile(q->getTempTarget());
			}

			userQueue.remove(q);

			fire(QueueManagerListener::REMOVED, q);
			delete q;
			setDirty();
		}
	}
	if(!x.empty()) {
		DownloadManager::getInstance()->abortDownload(x);
	}
}

void QueueManager::removeSource(const string& aTarget, User::Ptr& aUser, int reason, bool removeConn /* = true */) throw() {
	Lock l(cs);
	QueueItem* q = findByTarget(aTarget);
	string x;
	if(q != NULL) {
		dcassert(q->isSource(aUser));
		if(q->isSet(QueueItem::FLAG_USER_LIST)) {
			remove(q->getTarget());
			return;
		}
		if(reason == QueueItem::Source::FLAG_CRC_WARN) {
			// Already flagged?
			QueueItem::Source* s = *q->getSource(aUser);
			if(s->isSet(QueueItem::Source::FLAG_CRC_WARN)) {
				reason = QueueItem::Source::FLAG_CRC_FAILED;
			} else {
				s->setFlag(reason);
				return;
			}
		}
		if((q->getStatus() == QueueItem::STATUS_RUNNING) && q->getCurrent()->getUser() == aUser) {
			if(removeConn)
				x = q->getTarget();
			userQueue.setWaiting(q);
			userQueue.remove(q, aUser);
		} else if(q->getStatus() == QueueItem::STATUS_WAITING) {
			userQueue.remove(q, aUser);
		}

		q->removeSource(aUser, reason);
		
		fire(QueueManagerListener::SOURCES_UPDATED, q);
		setDirty();
	}
	if(!x.empty()) {
		DownloadManager::getInstance()->abortDownload(x);
	}
}

void QueueManager::removeSources(User::Ptr& aUser, int reason) throw() {
	string x;
	{
		Lock l(cs);
		QueueItem* qi = NULL;
		while( (qi = userQueue.getNext(aUser, QueueItem::PAUSED)) != NULL) {
			if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
				remove(qi->getTarget());
			} else {
				userQueue.remove(qi, aUser);
				qi->removeSource(aUser, reason);
				fire(QueueManagerListener::SOURCES_UPDATED, qi);
				setDirty();
			}
		}
		
		qi = userQueue.getRunning(aUser);
		if(qi != NULL) {
			if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
				remove(qi->getTarget());
			} else {
				userQueue.setWaiting(qi);
				userQueue.remove(qi, aUser);
				x = qi->getTarget();
				qi->removeSource(aUser, reason);
				fire(QueueManagerListener::SOURCES_UPDATED, qi);
				setDirty();
			}
		}
	}
	if(!x.empty()) {
		DownloadManager::getInstance()->abortDownload(x);
	}
}

void QueueManager::setPriority(const string& aTarget, QueueItem::Priority p) throw() {
	User::List ul;

	{
		Lock l(cs);
	
		QueueItem* q = findByTarget(aTarget);
		if( (q != NULL) && (q->getPriority() != p) ) {
			if( q->getStatus() != QueueItem::STATUS_RUNNING ) {
				if(q->getPriority() == QueueItem::PAUSED) {
					// Problem, we have to request connections to all these users...
					q->getUsers(ul);
				}

				userQueue.remove(q);
				q->setPriority(p);
				userQueue.add(q);
			} else {
				q->setPriority(p);
			}
			setDirty();
			fire(QueueManagerListener::STATUS_UPDATED, q);
		}
	}

	for(User::Iter i = ul.begin(); i != ul.end(); ++i) {
		ConnectionManager::getInstance()->getDownloadConnection(*i);
	}
}

QueueItem* QueueManager::findByTarget(const string& aTarget) {
	// The hash is (hopefully...) case-insensitive...
	QueueItem::StringIter i = queue.find(aTarget);
	return (i == queue.end()) ? NULL : i->second;
}

void QueueManager::saveQueue() throw() {
	if(!dirty)
		return;

	Lock l(cs);

	try {
		
#define STRINGLEN(n) n, sizeof(n)-1
#define CHECKESCAPE(n) SimpleXML::needsEscape(n, true) ? SimpleXML::escape(n, true) : n
		
		BufferedFile f(getQueueFile() + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(STRINGLEN("<?xml version=\"1.0\" encoding=\"windows-1252\"?>\r\n"));
		f.write(STRINGLEN("<Downloads>\r\n"));

		for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
			const QueueItem::Ptr& d = i->second;
			if(!d->isSet(QueueItem::FLAG_USER_LIST)) {
				f.write(STRINGLEN("\t<Download Target=\""));
				f.write(CHECKESCAPE(d->getTarget()));
				f.write(STRINGLEN("\" Resume=\""));
				f.write(Util::toString(d->isSet(QueueItem::FLAG_RESUME)));
				f.write(STRINGLEN("\" Size=\""));
				f.write(Util::toString(d->getSize()));
				f.write(STRINGLEN("\" Priority=\""));
				f.write(Util::toString((int)d->getPriority()));
				f.write(STRINGLEN("\" TempTarget=\""));
				f.write(CHECKESCAPE(d->getTempTarget()));
				f.write(STRINGLEN("\">\r\n"));

				for(QueueItem::Source::List::const_iterator j = d->sources.begin(); j != d->sources.end(); ++j) {
					QueueItem::Source* s = *j;
					f.write(STRINGLEN("\t\t<Source Nick=\""));
					f.write(CHECKESCAPE(s->getUser()->getNick()));
					f.write(STRINGLEN("\" Path=\""));
					f.write(CHECKESCAPE(s->getPath()));
					f.write(STRINGLEN("\"/>\r\n"));
				}

				f.write(STRINGLEN("\t</Download>\r\n"));
			}
		}

		for(DirectoryItem::DirectoryIter j = directories.begin(); j != directories.end(); ++j) {
			DirectoryItem::Ptr d = j->second;
			f.write(STRINGLEN("\t<Directory Target=\""));
			f.write(CHECKESCAPE(d->getTarget()));
			f.write(STRINGLEN("\" Nick=\""));
			f.write(CHECKESCAPE(d->getUser()->getNick()));
			f.write(STRINGLEN("\" Priority=\""));
			f.write(Util::toString((int)d->getPriority()));
			f.write(STRINGLEN("\" Source=\""));
			f.write(CHECKESCAPE(d->getName()));
			f.write(STRINGLEN("\"/>\r\n"));
		}
		
		f.write("</Downloads>\r\n");
		f.close();
		File::deleteFile(getQueueFile());
		File::renameFile(getQueueFile() + ".tmp", getQueueFile());

		dirty = false;
		lastSave = GET_TICK();
	} catch(const FileException&) {
		// ...
	}
}

void QueueManager::loadQueue() throw() {
	try {
		SimpleXML xml(5);
		xml.fromXML(File(getQueueFile(), File::READ, File::OPEN).read());

		load(&xml);
	} catch(const Exception&) {
		// ...
	}
}

void QueueManager::load(SimpleXML* aXml) {
	Lock l(cs);

	aXml->resetCurrentChild();
	if(aXml->findChild("Downloads")) {

		const string sDownload = "Download";
		const string sTempTarget = "TempTarget";
		const string sTarget = "Target";
		const string sResume = "Resume";
		const string sSize = "Size";
		const string sPriority = "Priority";
		const string sSource = "Source";
		const string sNick = "Nick";
		const string sPath = "Path";
		const string sDirectory = "Directory";
		
		aXml->stepIn();
		
		while(aXml->findChild(sDownload)) {
			const string& tempTarget = aXml->getChildAttrib(sTempTarget);
			const string& target = aXml->getChildAttrib(sTarget);
			int flags = aXml->getBoolChildAttrib(sResume) ? QueueItem::FLAG_RESUME : 0;
			int64_t size = aXml->getLongLongChildAttrib(sSize);
			QueueItem::Priority p = (QueueItem::Priority)aXml->getIntChildAttrib(sPriority);
			aXml->stepIn();
			if(aXml->findChild(sSource)) {
				do {
					const string& nick = aXml->getChildAttrib(sNick);
					const string& path = aXml->getChildAttrib(sPath);

					try {
						add(path, size, ClientManager::getInstance()->getUser(nick), target, flags, p, tempTarget);
					} catch(const Exception&) {
						// ...
					}
				} while(aXml->findChild(sSource));
			} else {
				// Sourceless file!
				Lock l(cs);
				bool newItem = false;
				if(target.find('\\') == string::npos)
					continue;

				QueueItem* qi = getQueueItem(target, size, flags, newItem);
				if(newItem) {
					qi->setPriority(p);
					qi->setTempTarget(tempTarget);
					
					if(lastInsert == queue.end()) {
						lastInsert = queue.insert(make_pair(qi->getTarget(), qi)).first;
					} else {
						lastInsert = queue.insert(lastInsert, make_pair(qi->getTarget(), qi));
					}
					fire(QueueManagerListener::ADDED, qi);
				}
			}
			aXml->stepOut();
		}
		aXml->resetCurrentChild();
		while(aXml->findChild(sDirectory)) {
			const string& name = aXml->getChildAttrib(sSource);
			const string& target = aXml->getChildAttrib(sTarget);
			const string& nick = aXml->getChildAttrib(sNick);
			QueueItem::Priority p = (QueueItem::Priority)aXml->getIntChildAttrib(sPriority);
			
			addDirectory(name, ClientManager::getInstance()->getUser(nick), target, p);
		}

		aXml->stepOut();
	}

	// We don't need to save the queue when we've just loaded it...
	dirty = false;
}

void QueueManager::importNMQueue(const string& aFile) throw(FileException) {
	File f(aFile, File::READ, File::OPEN);
	
	u_int32_t size = (u_int32_t)f.getSize();
	
	string tmp;
	if(size > 16) {
		AutoArray<u_int8_t> buf(size);
		f.read(buf, size);
		try {
			CryptoManager::getInstance()->decodeHuffman(buf, tmp);
		} catch(const CryptoException&) {
			return;
		}
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
		} catch(const Exception&) {
			// ...
		}
	}
}

// SearchManagerListener
void QueueManager::onAction(SearchManagerListener::Types type, SearchResult* sr) throw() {

	if(type == SearchManagerListener::SEARCH_RESULT && BOOLSETTING(AUTO_SEARCH)) {
		StringList l = getTargetsBySize(sr->getSize(), Util::getFileExt(sr->getFile()));
		
		StringTokenizer t(SearchManager::clean(sr->getFileName()), ' ');
		StringList& tok = t.getTokens();

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
					add(sr->getFile(), sr->getSize(), sr->getUser(), *i, QueueItem::FLAG_RESUME, 
						QueueItem::DEFAULT, Util::emptyString, false);
				} catch(const Exception&) {
					// ...
				}
			}
		}
	}
}

// ClientManagerListener
void QueueManager::onAction(ClientManagerListener::Types type, const User::Ptr& aUser) throw() {
	bool hasDown = false;
	switch(type) {
	case ClientManagerListener::USER_UPDATED:
		{
			Lock l(cs);
			for(int i = 0; i < QueueItem::LAST; ++i) {
				QueueItem::UserListIter j = userQueue.getList(i).find(aUser);
				if(j != userQueue.getList(i).end()) {
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

void QueueManager::onAction(TimerManagerListener::Types type, u_int32_t aTick) throw() {
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
 * @file
 * $Id: QueueManager.cpp,v 1.46 2003/10/20 21:04:55 arnetheduck Exp $
 */
