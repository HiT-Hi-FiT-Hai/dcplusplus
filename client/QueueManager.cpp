/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "CryptoManager.h"
#include "ShareManager.h"
#include "LogManager.h"

#include "UserConnection.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "DirectoryListing.h"

#include <limits>

#ifdef _WIN32
#define FILELISTS_DIR "FileLists\\"
#else
#define FILELISTS_DIR "filelists/"
#endif

#ifdef ff
#undef ff
#endif

#ifndef _WIN32
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#endif

const string QueueManager::USER_LIST_NAME = "MyList.DcLst";

namespace {
	const char* badChars = "$|.[]()-_+";
	const string TEMP_EXTENSION = ".dctmp";

	string getTempName(const string& aFileName, const TTHValue* aRoot) {
		string tmp(aFileName);
		if(aRoot != NULL) {
			TTHValue tmpRoot(*aRoot);
			tmp += "." + tmpRoot.toBase32();
		}
		tmp += TEMP_EXTENSION;
		return tmp;
	}
}
string QueueItem::getSearchString() const {
	return SearchManager::clean(getTargetFileName());
}

namespace {
}

const string& QueueItem::getTempTarget() {
	if(!isSet(QueueItem::FLAG_USER_LIST) && tempTarget.empty()) {
		if(!SETTING(TEMP_DOWNLOAD_DIRECTORY).empty() && (File::getSize(getTarget()) == -1)) {
			setTempTarget(SETTING(TEMP_DOWNLOAD_DIRECTORY) + getTempName(getTargetFileName(), getTTH()));
		}
	}
	return tempTarget;
}

QueueItem* QueueManager::FileQueue::add(const string& aTarget, int64_t aSize, 
						  int aFlags, QueueItem::Priority p, const string& aTempTarget,
						  int64_t aDownloadedBytes, u_int32_t aAdded, const TTHValue* root) throw(QueueException, FileException) 
{
	if(p == QueueItem::DEFAULT)
		p = (aSize <= 64*1024) ? QueueItem::HIGHEST : QueueItem::NORMAL;

	QueueItem* qi = new QueueItem(aTarget, aSize, p, aFlags, aDownloadedBytes, aAdded, root);

	if(!qi->isSet(QueueItem::FLAG_USER_LIST)) {
		if(!aTempTarget.empty()) {
			qi->setTempTarget(aTempTarget);
		}
	}

	if((qi->getDownloadedBytes() > 0))
		qi->setFlag(QueueItem::FLAG_EXISTS);

	dcassert(find(aTarget) == NULL);
	add(qi);
	return qi;
}

void QueueManager::FileQueue::add(QueueItem* qi) {
	if(lastInsert == queue.end())
		lastInsert = queue.insert(make_pair(const_cast<string*>(&qi->getTarget()), qi)).first;
	else
		lastInsert = queue.insert(lastInsert, make_pair(const_cast<string*>(&qi->getTarget()), qi));
}

QueueItem* QueueManager::FileQueue::find(const string& target) {
	QueueItem::StringIter i = queue.find(const_cast<string*>(&target));
	return (i == queue.end()) ? NULL : i->second;
}

void QueueManager::FileQueue::find(QueueItem::List& sl, int64_t aSize, const string& suffix) {
	for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
		if(i->second->getSize() == aSize) {
			const string& t = i->second->getTarget();
			if(suffix.empty() || (suffix.length() < t.length() &&
				Util::stricmp(suffix.c_str(), t.c_str() + (t.length() - suffix.length())) == 0) )
				sl.push_back(i->second);
		}
	}
}

void QueueManager::FileQueue::find(QueueItem::List& ql, const TTHValue* tth) {
	for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
		QueueItem* qi = i->second;
		if(qi->getTTH() != NULL && *qi->getTTH() == *tth) {
			ql.push_back(qi);
		}
	}
}

static QueueItem* findCandidate(QueueItem::StringIter start, QueueItem::StringIter end, StringList& recent) {
	QueueItem* cand = NULL;
	for(QueueItem::StringIter i = start; i != end; ++i) {
		QueueItem* q = i->second;

		// We prefer to search for things that are not running...
		if((cand != NULL) && (q->getStatus() == QueueItem::STATUS_RUNNING)) 
			continue;
		// No user lists
		if(q->isSet(QueueItem::FLAG_USER_LIST))
			continue;
		// No paused downloads
		if(q->getPriority() == QueueItem::PAUSED)
			continue;
		// No files that already have more than 5 online sources
		if(q->countOnlineUsers() >= 5)
			continue;
		// Did we search for it recently?
        if(find(recent.begin(), recent.end(), q->getTarget()) != recent.end())
			continue;

		cand = q;

		if(cand->getStatus() != QueueItem::STATUS_RUNNING)
			break;
	}
	return cand;
}

QueueItem* QueueManager::FileQueue::findAutoSearch(StringList& recent) {
	// We pick a start position at random, hoping that we will find something to search for...
	QueueItem::StringMap::size_type start = (QueueItem::StringMap::size_type)Util::rand((u_int32_t)queue.size());

	QueueItem::StringIter i = queue.begin();
	advance(i, start);

	QueueItem* cand = findCandidate(i, queue.end(), recent);
	if(cand == NULL) {
		cand = findCandidate(queue.begin(), i, recent);
	} else if(cand->getStatus() == QueueItem::STATUS_RUNNING) {
		QueueItem* cand2 = findCandidate(queue.begin(), i, recent);
		if(cand2 != NULL && (cand2->getStatus() != QueueItem::STATUS_RUNNING)) {
			cand = cand2;
		}
	}
	return cand;
}

void QueueManager::FileQueue::move(QueueItem* qi, const string& aTarget) {
	if(lastInsert != queue.end() && Util::stricmp(*lastInsert->first, qi->getTarget()) == 0)
		lastInsert = queue.end();
	queue.erase(const_cast<string*>(&qi->getTarget()));
	qi->setTarget(aTarget);
	add(qi);
}

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

QueueManager::QueueManager() : lastSave(0), queueFile(Util::getAppPath() + "Queue.xml"), dirty(true), nextSearch(0) { 
	TimerManager::getInstance()->addListener(this); 
	SearchManager::getInstance()->addListener(this);
	ClientManager::getInstance()->addListener(this);

	File::ensureDirectory(Util::getAppPath() + FILELISTS_DIR);
};

QueueManager::~QueueManager() throw() { 
	SearchManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this); 
	ClientManager::getInstance()->removeListener(this);

	saveQueue();

	if(!BOOLSETTING(KEEP_LISTS)) {
		string path = Util::getAppPath() + FILELISTS_DIR;

#ifdef _WIN32
		WIN32_FIND_DATA data;
		HANDLE hFind;
	
		hFind = FindFirstFile(Text::toT(path + "\\*.xml.bz2").c_str(), &data);
		if(hFind != INVALID_HANDLE_VALUE) {
			do {
				File::deleteFile(path + Text::fromT(data.cFileName));			
			} while(FindNextFile(hFind, &data));
			
			FindClose(hFind);
		}
		
		hFind = FindFirstFile(Text::toT(path + "\\*.DcLst").c_str(), &data);
		if(hFind != INVALID_HANDLE_VALUE) {
			do {
				File::deleteFile(path + Text::fromT(data.cFileName));			
			} while(FindNextFile(hFind, &data));
			
			FindClose(hFind);
		}

#else
		DIR* dir = opendir(path.c_str());
		if (dir) {
			while (struct dirent* ent = readdir(dir)) {
				if (fnmatch("*.xml.bz2", ent->d_name, 0) == 0 ||
					fnmatch("*.DcLst", ent->d_name, 0) == 0) {
					File::deleteFile(path + ent->d_name);	
				}
			}
			closedir(dir);
		}		
#endif
	}
};

void QueueManager::on(TimerManagerListener::Minute, u_int32_t aTick) throw() {
	string fn;
	string searchString;
	int64_t sz = 0;
	bool online = false;

	bool hasTTH = false;

	{
		Lock l(cs);
		QueueItem::UserMap& um = userQueue.getRunning();

		for(QueueItem::UserIter j = um.begin(); j != um.end(); ++j) {
			QueueItem* q = j->second;
			dcassert(q->getCurrentDownload() != NULL);
			q->setDownloadedBytes(q->getCurrentDownload()->getPos());
		}
		if(!um.empty())
			setDirty();

		if(BOOLSETTING(AUTO_SEARCH) && (aTick >= nextSearch) && (fileQueue.getSize() > 0)) {
			// We keep 30 recent searches to avoid duplicate searches
			while((recent.size() > fileQueue.getSize()) || (recent.size() > 30)) {
				recent.erase(recent.begin());
			}

			QueueItem* qi = fileQueue.findAutoSearch(recent);
			if(qi != NULL) {
				if(qi->getTTH()) {
					hasTTH = true;
					searchString = qi->getTTH()->toBase32();
				} else {
					fn = qi->getTargetFileName();
					sz = qi->getSize() - 1;
					searchString = qi->getSearchString();
				}
				online = qi->hasOnlineUsers();
				recent.push_back(qi->getTarget());
				nextSearch = aTick + (online ? 120000 : 300000);
			}
		}
	}

	if(hasTTH) {
		SearchManager::getInstance()->search(searchString, 0, SearchManager::TYPE_TTH, SearchManager::SIZE_DONTCARE);
	} else if(!fn.empty()) {
		SearchManager::getInstance()->search(searchString, sz, ShareManager::getInstance()->getType(fn), SearchManager::SIZE_ATLEAST);
	}
}

void QueueManager::add(const string& aFile, int64_t aSize, User::Ptr aUser, const string& aTarget, 
					   const TTHValue* root, 
					   int aFlags /* = QueueItem::FLAG_RESUME */, QueueItem::Priority p /* = QueueItem::DEFAULT */,
					   bool addBad /* = true */) throw(QueueException, FileException) 
{
	bool wantConnection = true;
	dcassert((aFile != USER_LIST_NAME) || (aFlags &QueueItem::FLAG_USER_LIST));

	// Check that we're not downloading from ourselves...
	if(aUser->getClientNick() == aUser->getNick()) {
		throw QueueException(STRING(NO_DOWNLOADS_FROM_SELF));
	}

	// Check if we're not downloading something already in our share
	if (BOOLSETTING(DONT_DL_ALREADY_SHARED) && root != NULL){
		TTHValue* r = const_cast<TTHValue*>(root);
		if (ShareManager::getInstance()->isTTHShared(r)){
			throw QueueException(STRING(TTH_ALREADY_SHARED));
		}
	}
    
	bool utf8 = (aFlags & QueueItem::FLAG_SOURCE_UTF8) > 0;
	aFlags &= ~QueueItem::FLAG_SOURCE_UTF8;

	string target = checkTarget(aTarget, aSize, aFlags);

	// Check if it's a zero-byte file, if so, create and return...
	if(aSize == 0) {
		if(!BOOLSETTING(SKIP_ZERO_BYTE)) {
			File f(target, File::WRITE, File::CREATE);
		}
		return;
	}

	{
		Lock l(cs);

		QueueItem* q = fileQueue.find(target);
		if(q == NULL) {
			q = fileQueue.add(target, aSize, aFlags, p, Util::emptyString, 0, GET_TIME(), root);
			fire(QueueManagerListener::Added(), q);
		} else {
			if(q->getSize() != aSize) {
				throw QueueException(STRING(FILE_WITH_DIFFERENT_SIZE));
			}
			if(root != NULL) {
				if(q->getTTH() == NULL) {
					q->setTTH(new TTHValue(*root));
				} else if(!(*root == *q->getTTH())) {
					throw QueueException(STRING(FILE_WITH_DIFFERENT_TTH));
				}
			}
			q->setFlag(aFlags);
			
			// We don't add any more sources to user list downloads...
			if(q->isSet(QueueItem::FLAG_USER_LIST))
				return;
		}

		wantConnection = addSource(q, aFile, aUser, addBad ? QueueItem::Source::FLAG_MASK : 0, utf8);
	}

	if(wantConnection && aUser->isOnline())
		ConnectionManager::getInstance()->getDownloadConnection(aUser);
}

void QueueManager::readd(const string& target, User::Ptr& aUser) throw(QueueException) {
	bool wantConnection = false;
	{
		Lock l(cs);
		QueueItem* q = fileQueue.find(target);
		if(q != NULL && q->isBadSource(aUser)) {
			QueueItem::Source* s = *q->getBadSource(aUser);
			wantConnection = addSource(q, s->getPath(), aUser, QueueItem::Source::FLAG_MASK, s->isSet(QueueItem::Source::FLAG_UTF8));
		}
	}
	if(wantConnection && aUser->isOnline())
		ConnectionManager::getInstance()->getDownloadConnection(aUser);
}

string QueueManager::checkTarget(const string& aTarget, int64_t aSize, int& flags) throw(QueueException, FileException) {
#ifdef _WIN32
	if(aTarget.length() > MAX_PATH) {
		throw QueueException(STRING(TARGET_FILENAME_TOO_LONG));
	}
	// Check that target starts with a drive or is an UNC path
	if( (aTarget[1] != ':' || aTarget[2] != '\\') &&
		(aTarget[0] != '\\' && aTarget[1] != '\\') ) {
		throw QueueException(STRING(INVALID_TARGET_FILE));
	}
#else
	if(aTarget.length() > PATH_MAX) {
		throw QueueException(STRING(TARGET_FILENAME_TOO_LONG));
	}
	// Check that target contains at least one directory...we don't want headless files...
	if(aTarget[0] != '/') {
		throw QueueException(STRING(INVALID_TARGET_FILE));
	}
#endif

	string target = Util::validateFileName(aTarget);

	// Check that the file doesn't already exist...
	int64_t sz = File::getSize(target);
	if( (aSize != -1) && (aSize <= sz) )  {
		throw FileException(STRING(LARGER_TARGET_FILE_EXISTS));
	}
	if(sz > 0)
		flags |= QueueItem::FLAG_EXISTS;

	return target;
}

/** Add a source to an existing queue item */
bool QueueManager::addSource(QueueItem* qi, const string& aFile, User::Ptr aUser, Flags::MaskType addBad, bool utf8) throw(QueueException, FileException) {
	QueueItem::Source* s = NULL;
	bool wantConnection = (qi->getPriority() != QueueItem::PAUSED);

	if(qi->isSource(aUser, aFile)) {
		throw QueueException(STRING(DUPLICATE_SOURCE));
	}

	if(qi->isBadSourceExcept(aUser, aFile, addBad)) {
		throw QueueException(STRING(DUPLICATE_SOURCE));
	}

	s = qi->addSource(aUser, aFile);
	if(utf8)
		s->setFlag(QueueItem::Source::FLAG_UTF8);

	if(aUser->isSet(User::PASSIVE) && (SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE) ) {
		qi->removeSource(aUser, QueueItem::Source::FLAG_PASSIVE);
		wantConnection = false;
	} else if(qi->getStatus() != QueueItem::STATUS_RUNNING) {
		userQueue.add(qi, aUser);
	} else {
		wantConnection = false;
	}

	fire(QueueManagerListener::SourcesUpdated(), qi);
	setDirty();

	return wantConnection;
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

#define isnum(c) (((c) >= '0') && ((c) <= '9'))

static inline u_int32_t adjustSize(u_int32_t sz, const string& name) {
	if(name.length() > 2) {
		// filename.r32
		u_int8_t c1 = (u_int8_t)name[name.length()-2];
		u_int8_t c2 = (u_int8_t)name[name.length()-1];
		if(isnum(c1) && isnum(c2)) {
			return sz + (c1-'0')*10 + (c2-'0');
		} else if(name.length() > 6) {
			// filename.part32.rar
			c1 = name[name.length() - 6];
			c2 = name[name.length() - 5];
			if(isnum(c1) && isnum(c2)) {
				return sz + (c1-'0')*10 + (c2-'0');
			}
		}
	} 

	return sz;
}

typedef HASH_MULTIMAP<u_int32_t, QueueItem*> SizeMap;
typedef SizeMap::iterator SizeIter;
typedef pair<SizeIter, SizeIter> SizePair;

// *** WARNING *** 
// Lock(cs) makes sure that there's only one thread accessing these,
// I put them here to avoid growing a huge stack...

static DirectoryListing* curDl = NULL;
static SizeMap sizeMap;
static string utfTmp;

static const string& utfEscaper(const string& x) {
	return curDl->getUtf8() ? x : (utfTmp.clear(), Text::acpToUtf8(x, utfTmp));
}

int QueueManager::matchFiles(DirectoryListing::Directory* dir) throw() {
	int matches = 0;
	for(DirectoryListing::Directory::Iter j = dir->directories.begin(); j != dir->directories.end(); ++j) {
		if(!(*j)->getAdls())
			matches += matchFiles(*j);
	}

	for(DirectoryListing::File::Iter i = dir->files.begin(); i != dir->files.end(); ++i) {
		DirectoryListing::File* df = *i;

		SizePair files = sizeMap.equal_range(adjustSize((u_int32_t)df->getSize(), df->getName()));
		for(SizeIter j = files.first; j != files.second; ++j) {
			QueueItem* qi = j->second;
			bool equal = false;
			if(qi->getTTH() != NULL && df->getTTH() != NULL) {
				equal = (*qi->getTTH() == *df->getTTH());
			} else {
				if(Util::stricmp(utfEscaper(df->getName()), qi->getTargetFileName()) == 0) {
					dcassert(df->getSize() == qi->getSize());	
					equal = true;

					if(df->getTTH() != NULL) {
						dcassert(qi->getTTH() == NULL);
						qi->setTTH(new TTHValue(*df->getTTH()));
					}
				}
			}
			if(equal) {
				try {
					addSource(qi, curDl->getPath(df) + df->getName(), curDl->getUser(), QueueItem::Source::FLAG_FILE_NOT_AVAILABLE, curDl->getUtf8());
					matches++;
				} catch(const Exception&) {
				}
			}
		}
	}
	return matches;
}

int QueueManager::matchListing(DirectoryListing* dl) throw() {
	int matches = 0;
	{
		Lock l(cs);
		sizeMap.clear();
		matches = 0;
		curDl = dl;
		for(QueueItem::StringIter i = fileQueue.getQueue().begin(); i != fileQueue.getQueue().end(); ++i) {
			QueueItem* qi = i->second;
			if(qi->getSize() != -1) {
				sizeMap.insert(make_pair(adjustSize((u_int32_t)qi->getSize(), qi->getTarget()), qi));
			}
		}

		matches = matchFiles(dl->getRoot());
	}
	if(matches > 0)
		ConnectionManager::getInstance()->getDownloadConnection(dl->getUser());
	return matches;
}

void QueueManager::move(const string& aSource, const string& aTarget) throw() {
	string target = Util::validateFileName(aTarget);
	if(Util::stricmp(aSource, target) == 0)
		return;

	bool delSource = false;

	Lock l(cs);
	QueueItem* qs = fileQueue.find(aSource);
	if(qs != NULL) {
		// Don't move running downloads
		if(qs->getStatus() == QueueItem::STATUS_RUNNING) {
			return;
		}
		// Don't move file lists
		if(qs->isSet(QueueItem::FLAG_USER_LIST))
			return;

		// Let's see if the target exists...then things get complicated...
		QueueItem* qt = fileQueue.find(target);
		if(qt == NULL) {
			// Good, update the target and move in the queue...
			fileQueue.move(qs, target);
			fire(QueueManagerListener::Moved(), qs);
			setDirty();
		} else {
			// Don't move to target of different size
			if(qs->getSize() != qt->getSize())
				return;

			try {
				for(QueueItem::Source::Iter i = qs->getSources().begin(); i != qs->getSources().end(); ++i) {
					QueueItem::Source* s = *i;
					addSource(qt, s->getPath(), s->getUser(), QueueItem::Source::FLAG_MASK, s->isSet(QueueItem::Source::FLAG_UTF8));
				}
			} catch(const Exception&) {
			}
			delSource = true;
		}
	}

	if(delSource) {
		remove(aSource);
	}
}

void QueueManager::getTargetsBySize(StringList& sl, int64_t aSize, const string& suffix) throw() {
	Lock l(cs);
	QueueItem::List ql;
	fileQueue.find(ql, aSize, suffix);
	for(QueueItem::Iter i = ql.begin(); i != ql.end(); ++i) {
		sl.push_back((*i)->getTarget());
	}
}

void QueueManager::getTargetsByRoot(StringList& sl, const TTHValue& tth) {
	Lock l(cs);
	QueueItem::List ql;
	fileQueue.find(ql, &tth);
	for(QueueItem::Iter i = ql.begin(); i != ql.end(); ++i) {
		sl.push_back((*i)->getTarget());
	}
}


Download* QueueManager::getDownload(User::Ptr& aUser, bool supportsTrees) throw() {
	Lock l(cs);

	QueueItem* q = userQueue.getNext(aUser);

	if(q == NULL)
		return NULL;

	userQueue.setRunning(q, aUser);

	fire(QueueManagerListener::StatusUpdated(), q);
	
	Download* d = new Download(q);

	if(d->getSize() != -1 && d->getTTH()) {
		if(HashManager::getInstance()->getTree(*d->getTTH(), d->getTigerTree())) {
			d->setTreeValid(true);
		} else if(supportsTrees && !q->getCurrent()->isSet(QueueItem::Source::FLAG_NO_TREE) && d->getSize() > HashManager::MIN_BLOCK_SIZE) {
			// Get the tree unless the file is small (for small files, we'd probably only get the root anyway)
			d->setFlag(Download::FLAG_TREE_DOWNLOAD);
			d->getTigerTree().setFileSize(d->getSize());
			d->setPos(0);
			d->unsetFlag(Download::FLAG_RESUME);
		} else {
			// Use the root as tree to get some sort of validation at least...
			d->getTigerTree() = TigerTree(d->getSize(), d->getSize(), *d->getTTH());
			d->setTreeValid(true);
		}
	}

	if(!d->isSet(Download::FLAG_TREE_DOWNLOAD) && BOOLSETTING(ANTI_FRAG) ) {
		d->setStartPos(q->getDownloadedBytes());
	}

	q->setCurrentDownload(d);

	return d;
}


void QueueManager::putDownload(Download* aDownload, bool finished) throw() {
	User::List getConn;
 	string fname;
	User::Ptr up;
	int flag = 0;

	{
		Lock l(cs);
		QueueItem* q = fileQueue.find(aDownload->getTarget());

		if(q != NULL) {
			if(aDownload->isSet(Download::FLAG_USER_LIST)) {
				if(aDownload->getSource() == "files.xml.bz2") {
					q->setFlag(QueueItem::FLAG_XML_BZLIST);
				} else {
					q->unsetFlag(QueueItem::FLAG_XML_BZLIST);
				}
			}

			dcassert(q->getStatus() == QueueItem::STATUS_RUNNING);

			if(finished) {
				if(aDownload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
					// Got a full tree, now add it to the HashManager
					dcassert(aDownload->getTreeValid());
					HashManager::getInstance()->addTree(aDownload->getTigerTree());

					q->setCurrentDownload(NULL);
					userQueue.setWaiting(q);

					fire(QueueManagerListener::StatusUpdated(), q);
				} else {
					userQueue.remove(q);
					fire(QueueManagerListener::Finished(), q);
					fire(QueueManagerListener::Removed(), q);
					// Now, let's see if this was a directory download filelist...
					if( (q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD) && directories.find(q->getCurrent()->getUser()) != directories.end()) ||
						(q->isSet(QueueItem::FLAG_MATCH_QUEUE)) ) 
					{
						fname = q->getListName();
						up = q->getCurrent()->getUser();
						flag = (q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD) ? QueueItem::FLAG_DIRECTORY_DOWNLOAD : 0)
							| (q->isSet(QueueItem::FLAG_MATCH_QUEUE) ? QueueItem::FLAG_MATCH_QUEUE : 0);
					} 
					fileQueue.remove(q);
					setDirty();
				}
			} else {
				if(!aDownload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
					q->setDownloadedBytes(aDownload->getPos());

					if(q->getDownloadedBytes() > 0) {
						q->setFlag(QueueItem::FLAG_EXISTS);
					} else {
						q->setTempTarget(Util::emptyString);
					}
					if(q->isSet(QueueItem::FLAG_USER_LIST)) {
						// Blah...no use keeping an unfinished file list...
						File::deleteFile(q->getListName());
					}
				}

				if(q->getPriority() != QueueItem::PAUSED) {
					for(QueueItem::Source::Iter j = q->getSources().begin(); j != q->getSources().end(); ++j) {
						if((*j)->getUser()->isOnline()) {
							getConn.push_back((*j)->getUser());
						}
					}
				}

				q->setCurrentDownload(NULL);

				// This might have been set to wait by removesource already...
				if(q->getStatus() == QueueItem::STATUS_RUNNING) {
					userQueue.setWaiting(q);
					fire(QueueManagerListener::StatusUpdated(), q);
				}
			}
		} else if(!aDownload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
			if(!aDownload->getTempTarget().empty() && aDownload->getTempTarget() != aDownload->getTarget()) {
				File::deleteFile(aDownload->getTempTarget() + Download::ANTI_FRAG_EXT);
				File::deleteFile(aDownload->getTempTarget());
			}
		}
		aDownload->setUserConnection(NULL);
		delete aDownload;
	}

	for(User::Iter i = getConn.begin(); i != getConn.end(); ++i) {
		ConnectionManager::getInstance()->getDownloadConnection(*i);
	}

	if(!fname.empty()) {
		processList(fname, up, flag);
	}
}

void QueueManager::processList(const string& name, User::Ptr& user, int flags) {
	DirectoryListing dirList(user);
	try {
		dirList.loadFile(name);
	} catch(const Exception&) {
		/** @todo Log this */
		return;
	}

	if(flags & QueueItem::FLAG_DIRECTORY_DOWNLOAD) {
		DirectoryItem::List dl;
		{
			Lock l(cs);
			DirectoryItem::DirectoryPair dp = directories.equal_range(user);
			for(DirectoryItem::DirectoryIter i = dp.first; i != dp.second; ++i) {
				dl.push_back(i->second);
			}
			directories.erase(user);
		}

		for(DirectoryItem::Iter i = dl.begin(); i != dl.end(); ++i) {
			DirectoryItem* di = *i;
			dirList.download(di->getName(), di->getTarget(), false);
			delete di;
		}
	}
	if(flags & QueueItem::FLAG_MATCH_QUEUE) {
		AutoArray<char> tmp(STRING(MATCHED_FILES).size() + 16);
		sprintf(tmp, CSTRING(MATCHED_FILES), matchListing(&dirList));
		LogManager::getInstance()->message(user->getNick() + ": " + string(tmp));			
	}
}

void QueueManager::remove(const string& aTarget) throw() {
	string x;
	{
		Lock l(cs);

		QueueItem* q = fileQueue.find(aTarget);
		if(q != NULL) {
			if(q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD)) {
				dcassert(q->getSources().size() == 1);
				DirectoryItem::DirectoryPair dp = directories.equal_range(q->getSources()[0]->getUser());
				for(DirectoryItem::DirectoryIter i = dp.first; i != dp.second; ++i) {
					delete i->second;
				}
				directories.erase(q->getSources()[0]->getUser());
			}

			if(q->getStatus() == QueueItem::STATUS_RUNNING) {
				x = q->getTarget();
			} else if(!q->getTempTarget().empty() && q->getTempTarget() != q->getTarget()) {
				File::deleteFile(q->getTempTarget() + Download::ANTI_FRAG_EXT);
				File::deleteFile(q->getTempTarget());
			}

			userQueue.remove(q);

			fire(QueueManagerListener::Removed(), q);
			fileQueue.remove(q);

			setDirty();
		}
	}
	if(!x.empty()) {
		DownloadManager::getInstance()->abortDownload(x);
	}
}

void QueueManager::removeSource(const string& aTarget, User::Ptr& aUser, int reason, bool removeConn /* = true */, bool isTree /* = false */) throw() {
	Lock l(cs);
	QueueItem* q = fileQueue.find(aTarget);
	string x;
	if(q != NULL) {
		dcassert(q->isSource(aUser));
		if(q->isSet(QueueItem::FLAG_USER_LIST)) {
			remove(q->getTarget());
			return;
		}

		if(isTree) {
			QueueItem::Source* s = *q->getSource(aUser);
			s->setFlag(QueueItem::Source::FLAG_NO_TREE);
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
		
		fire(QueueManagerListener::SourcesUpdated(), q);
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
				fire(QueueManagerListener::SourcesUpdated(), qi);
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
				fire(QueueManagerListener::SourcesUpdated(), qi);
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
	
		QueueItem* q = fileQueue.find(aTarget);
		if( (q != NULL) && (q->getPriority() != p) ) {
			if( q->getStatus() != QueueItem::STATUS_RUNNING ) {
				if(q->getPriority() == QueueItem::PAUSED || p == QueueItem::HIGHEST) {
					// Problem, we have to request connections to all these users...
					q->getOnlineUsers(ul);
				}

				userQueue.remove(q);
				q->setPriority(p);
				userQueue.add(q);
			} else {
				q->setPriority(p);
			}
			setDirty();
			fire(QueueManagerListener::StatusUpdated(), q);
		}
	}

	for(User::Iter i = ul.begin(); i != ul.end(); ++i) {
		ConnectionManager::getInstance()->getDownloadConnection(*i);
	}
}

void QueueManager::saveQueue() throw() {
	if(!dirty)
		return;

	Lock l(cs);

	try {
		
#define STRINGLEN(n) n, sizeof(n)-1
#define CHECKESCAPE(n) SimpleXML::escape(n, tmp, true)

		File ff(getQueueFile() + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&ff);

		f.write(SimpleXML::utf8Header);
		f.write(STRINGLEN("<Downloads Version=\"" VERSIONSTRING "\">\r\n"));
		string tmp;
		string b32tmp;
		for(QueueItem::StringIter i = fileQueue.getQueue().begin(); i != fileQueue.getQueue().end(); ++i) {
			QueueItem* qi = i->second;
			if(!qi->isSet(QueueItem::FLAG_USER_LIST)) {
				f.write(STRINGLEN("\t<Download Target=\""));
				f.write(CHECKESCAPE(qi->getTarget()));
				f.write(STRINGLEN("\" Size=\""));
				f.write(Util::toString(qi->getSize()));
				f.write(STRINGLEN("\" Priority=\""));
				f.write(Util::toString((int)qi->getPriority()));
				f.write(STRINGLEN("\" Added=\""));
				f.write(Util::toString(qi->getAdded()));
				if(qi->getTTH() != NULL) {
					b32tmp.clear();
					f.write(STRINGLEN("\" TTH=\""));
					f.write(qi->getTTH()->toBase32(b32tmp));
				}
				if(qi->getDownloadedBytes() > 0) {
					f.write(STRINGLEN("\" TempTarget=\""));
					f.write(CHECKESCAPE(qi->getTempTarget()));
					f.write(STRINGLEN("\" Downloaded=\""));
					f.write(Util::toString(qi->getDownloadedBytes()));
				}
				f.write(STRINGLEN("\">\r\n"));

				for(QueueItem::Source::List::const_iterator j = qi->sources.begin(); j != qi->sources.end(); ++j) {
					QueueItem::Source* s = *j;
					f.write(STRINGLEN("\t\t<Source Nick=\""));
					f.write(CHECKESCAPE(s->getUser()->getNick()));
					f.write(STRINGLEN("\" Path=\""));
					f.write(CHECKESCAPE(s->getPath()));
					f.write(STRINGLEN("\" Utf8=\""));
					f.write(s->isSet(QueueItem::Source::FLAG_UTF8) ? "1" : "0", 1);
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
		f.flush();
		ff.close();
		File::deleteFile(getQueueFile());
		File::renameFile(getQueueFile() + ".tmp", getQueueFile());

		dirty = false;
	} catch(const FileException&) {
		// ...
	}
	// Put this here to avoid very many saves tries when disk is full...
	lastSave = GET_TICK();
}

class QueueLoader : public SimpleXMLReader::CallBack {
public:
	QueueLoader() : cur(NULL), inDownloads(false) { };
	virtual ~QueueLoader() { }
	virtual void startTag(const string& name, StringPairList& attribs, bool simple);
	virtual void endTag(const string& name, const string& data);
private:
	string target;

	QueueItem* cur;
	bool inDownloads;
};

void QueueManager::loadQueue() throw() {
	try {
		QueueLoader l;
		SimpleXMLReader(&l).fromXML(File(getQueueFile(), File::READ, File::OPEN).read());
		dirty = false;
	} catch(const Exception&) {
		// ...
	}
}

static const string sDownload = "Download";
static const string sTempTarget = "TempTarget";
static const string sTarget = "Target";
static const string sSize = "Size";
static const string sDownloaded = "Downloaded";
static const string sPriority = "Priority";
static const string sSource = "Source";
static const string sNick = "Nick";
static const string sPath = "Path";
static const string sDirectory = "Directory";
static const string sAdded = "Added";
static const string sUtf8 = "Utf8";
static const string sTTH = "TTH";

void QueueLoader::startTag(const string& name, StringPairList& attribs, bool simple) {
	QueueManager* qm = QueueManager::getInstance();
	if(!inDownloads && name == "Downloads") {
		inDownloads = true;
	} else if(inDownloads) {
		if(cur == NULL && name == sDownload) {
			int flags = QueueItem::FLAG_RESUME;
			int64_t size = Util::toInt64(getAttrib(attribs, sSize, 1));
			if(size == 0)
				return;
			try {
				const string& tgt = getAttrib(attribs, sTarget, 0);
				target = QueueManager::checkTarget(tgt, size, flags);
				if(target.empty())
					return;
			} catch(const Exception&) {
				return;
			}
			QueueItem::Priority p = (QueueItem::Priority)Util::toInt(getAttrib(attribs, sPriority, 3));
			u_int32_t added = (u_int32_t)Util::toInt(getAttrib(attribs, sAdded, 4));
			const string& tthRoot = getAttrib(attribs, sTTH, 5);
			string tempTarget = getAttrib(attribs, sTempTarget, 5);
			int64_t downloaded = Util::toInt64(getAttrib(attribs, sDownloaded, 5));
			if (downloaded > size || downloaded < 0)
				downloaded = 0;

			if(added == 0)
				added = GET_TIME();

			QueueItem* qi = qm->fileQueue.find(target);

			if(qi == NULL) {
				if(tthRoot.empty()) {
					qi = qm->fileQueue.add(target, size, flags, p, tempTarget, downloaded, added, NULL);
				} else {
					TTHValue root(tthRoot);
					qi = qm->fileQueue.add(target, size, flags, p, tempTarget, downloaded, added, &root);
				}
				qm->fire(QueueManagerListener::Added(), qi);
			}
			if(!simple)
				cur = qi;
		} else if(cur != NULL && name == sSource) {
			const string& nick = getAttrib(attribs, sNick, 0);
			if(nick.empty())
				return;
			const string& path = getAttrib(attribs, sPath, 1);
			if(path.empty())
				return;
			const string& utf8 = getAttrib(attribs, sUtf8, 2);
			bool isUtf8 = (utf8 == "1");
			User::Ptr user = ClientManager::getInstance()->getUser(nick);
			try {
				if(qm->addSource(cur, path, user, 0, isUtf8) && user->isOnline())
					ConnectionManager::getInstance()->getDownloadConnection(user);
			} catch(const Exception&) {
				return;
			}
		} else if(cur == NULL && name == sDirectory) {
			const string& targetd = getAttrib(attribs, sTarget, 0);
			if(targetd.empty())
				return;
			const string& nick = getAttrib(attribs, sNick, 1);
			if(nick.empty())
				return;
			QueueItem::Priority p = (QueueItem::Priority)Util::toInt(getAttrib(attribs, sPriority, 2));
			const string& source = getAttrib(attribs, sSource, 3);
			if(source.empty())
				return;

			qm->addDirectory(source, ClientManager::getInstance()->getUser(nick), targetd, p);
		}
	}
}

void QueueLoader::endTag(const string& name, const string&) {
	if(inDownloads) {
		if(name == sDownload)
			cur = NULL;
		else if(name == "Downloads")
			inDownloads = false;
	}
}

// SearchManagerListener
void QueueManager::on(SearchManagerListener::SR, SearchResult* sr) throw() {
	bool added = false;

	if(BOOLSETTING(AUTO_SEARCH)) {
		Lock l(cs);
		QueueItem::List matches;

		fileQueue.find(matches, sr->getSize(), Util::getFileExt(sr->getFile()));

		if(sr->getTTH() != NULL) {
			fileQueue.find(matches, sr->getTTH());
		}

		for(QueueItem::Iter i = matches.begin(); i != matches.end(); ++i) {
			QueueItem* qi = *i;
			bool found = false;
			if(qi->getTTH()) {
				found = sr->getTTH() && (*qi->getTTH() == *sr->getTTH()) && (qi->getSize() == sr->getSize());
			} else {
				found = (Util::stricmp(qi->getTargetFileName(), sr->getFileName()) == 0);
			}

			if(found) {
				try {
					addSource(qi, sr->getFile(), sr->getUser(), 0, false);
					added = true;
				} catch(const Exception&) {
					// ...
				}
				break;
			}
		}
	}

	if(added && BOOLSETTING(AUTO_SEARCH_AUTO_MATCH))
		try {
			addList(sr->getUser(), QueueItem::FLAG_MATCH_QUEUE);
		} catch(const Exception&) {
			// ...
		}
}

// ClientManagerListener
void QueueManager::on(ClientManagerListener::UserUpdated, const User::Ptr& aUser) throw() {
	bool hasDown = false;
	{
		Lock l(cs);
		for(int i = 0; i < QueueItem::LAST; ++i) {
			QueueItem::UserListIter j = userQueue.getList(i).find(aUser);
			if(j != userQueue.getList(i).end()) {
				for(QueueItem::Iter m = j->second.begin(); m != j->second.end(); ++m)
					fire(QueueManagerListener::StatusUpdated(), *m);
				if(i != QueueItem::PAUSED)
					hasDown = true;
			}
		}
	}

	if(aUser->isOnline() && hasDown)	
		ConnectionManager::getInstance()->getDownloadConnection(aUser);
}

void QueueManager::on(TimerManagerListener::Second, u_int32_t aTick) throw() {
	if(dirty && ((lastSave + 10000) < aTick)) {
		saveQueue();
	}
}

/**
 * @file
 * $Id: QueueManager.cpp,v 1.116 2005/01/13 15:07:57 arnetheduck Exp $
 */
