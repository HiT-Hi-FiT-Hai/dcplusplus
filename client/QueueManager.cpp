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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "QueueManager.h"
#include "ConnectionManager.h"
#include "ClientManager.h"
#include "DownloadManager.h"
#include "UserConnection.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"

QueueManager* QueueManager::instance = NULL;

const string QueueManager::USER_LIST_NAME = "MyList.DcLst";

void QueueManager::onTimerMinute(DWORD /*aTick*/) {

	{
		Lock l(cs);

		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			QueueItem* q = *i;
			
			if(q->getStatus() == QueueItem::RUNNING) {
				continue;
			}
			
			if(BOOLSETTING(AUTO_SEARCH) && q->getPriority() != QueueItem::PAUSED) {
				bool online = false;
				for(QueueItem::Source::Iter j = q->getSources().begin(); j != q->getSources().end(); ++j) {
					if(((*j)->getUser()) && (*j)->getUser()->isOnline()) {
						online = true;
						break;
					}
				}

				if(!online) {
					if(search.find(q->getTarget()) == search.end()) {
						search[q->getTarget()] = TimerManager::getTick();
					}
				}
			}
		}
		for(map<string, DWORD>::iterator k = search.begin(); k != search.end(); ++k) {
			if( (k->second + 3*60*1000) < TimerManager::getTick() ) {
				QueueItem* q = findByTarget(k->first);
				if(q != NULL) {
					bool online = false;
					for(QueueItem::Source::Iter j = q->getSources().begin(); j != q->getSources().end(); ++j) {
						if( ((*j)->getUser()) && (*j)->getUser()->isOnline()) {
							online = true;
							break;
						}
					}
					if(!online) {
						dcdebug("QueueManager::onTimerMinute Doing autosearch for %s\n", SearchManager::clean(q->getTargetFileName()).c_str());
						SearchManager::getInstance()->search(SearchManager::clean(q->getTargetFileName()), q->getSize() - 1, 0, SearchManager::SIZE_ATLEAST);
						search.erase(k);
						break;
					}
				}
			}
		}
	}

	if(dirty) {
		SettingsManager::getInstance()->save();		
	}
	
}

void QueueManager::add(const string& aFile, LONGLONG aSize, const User::Ptr& aUser, const string& aTarget, 
					   bool aResume) throw(QueueException, FileException) {

	// Alright, first we get a queue item, new or old...
	bool newItem = false;
	QueueItem* q = getQueueItem(aFile, aTarget, aSize,aResume, newItem);
	QueueItem::Source* s = NULL;

	{
		Lock l(cs);
		s = q->addSource(aUser, aFile);
		if(newItem) {
			queue.push_back(q);
		}
	}

	// Good, now notify the listeners of the changes
	if(newItem)
		fire(QueueManagerListener::ADDED, q);

	fire(QueueManagerListener::SOURCES_UPDATED, q);
	dirty = true;
	// And make sure we're trying to connect to this fellow...
	ConnectionManager::getInstance()->getDownloadConnection(aUser);
}

QueueItem* QueueManager::getQueueItem(const string& aFile, const string& aTarget, LONGLONG aSize, bool aResume, bool& newItem) {
	QueueItem* q = findByTarget(aTarget);
	
	if(q == NULL) {
		newItem = true;
		if(aSize <= 16*1024)
			q = new QueueItem(aTarget, aSize, QueueItem::HIGH, aResume);
		else
			q = new QueueItem(aTarget, aSize, QueueItem::NORMAL, aResume);
		if(aFile == USER_LIST_NAME)
			q->setFlag(QueueItem::USER_LIST);

	} else {
		newItem = false;
		if(q->getSize() != aSize) {
			throw QueueException("A file with a different size already exists in the queue");
		}

		if( (q->getSize() != -1) && (q->getSize() <= File::getSize(aTarget)) )  {
			throw FileException("A file of equal or larger size already exists at the target location");
		}

		if(aFile == USER_LIST_NAME) {
			throw QueueException("Already getting that list");
		}
	}

	return q;
}

Download* QueueManager::getDownload(UserConnection* aUserConnection) {

	QueueItem* q = NULL;

	{
		Lock l(cs);

		// Find a suitable download for this user
		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			if( ((*i)->isSource(aUserConnection->getUser())) && 
				((*i)->getStatus() == QueueItem::WAITING) && 
				((*i)->getPriority() != QueueItem::PAUSED) ) {

				if(q == NULL) {
					q = *i;
					dcdebug("Found download for %s: %s\n", aUserConnection->getUser()->getNick().c_str(), q->getTarget().c_str());
					continue;
				} else if(q->getPriority() < (*i)->getPriority()) {
					q = *i;
					dcdebug("Found better download for %s: %s\n", aUserConnection->getUser()->getNick().c_str(), q->getTarget().c_str());
				}
			}
		}
		if(q == NULL)
			return NULL;

		// Set the flag to running, so that we don't get the same download twice...
		q->setStatus(QueueItem::RUNNING);
		q->setCurrent(aUserConnection->getUser());
	}
	fire(QueueManagerListener::STATUS_UPDATED, q);
	return new Download(q, ConnectionManager::getInstance()->getQueueItem(aUserConnection));
}

bool QueueManager::hasDownload(const User::Ptr& aUser) {
	
	{
		Lock l(cs);
		
		// Find a suitable download for this user
		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			if( ((*i)->isSource(aUser)) && 
				((*i)->getStatus() == QueueItem::WAITING) && 
				((*i)->getPriority() != QueueItem::PAUSED) ) {
				
				return true;
			}
		}
	}

	return false;
}

void QueueManager::putDownload(Download* aDownload, bool finished /* = false */) {

	if(finished) {
		aDownload->getQueueItem()->setStatus(QueueItem::FINISHED);
		fire(QueueManagerListener::STATUS_UPDATED, aDownload->getQueueItem());

		if(aDownload->isSet(Download::USER_LIST) || BOOLSETTING(REMOVE_FINISHED)) {
			{
				Lock l(cs);
				dcassert(find(queue.begin(), queue.end(), aDownload->getQueueItem()) != queue.end());

				queue.erase(find(queue.begin(), queue.end(), aDownload->getQueueItem()));
			}

			fire(QueueManagerListener::REMOVED, aDownload->getQueueItem());
			
			delete aDownload->getQueueItem();
			delete aDownload;
		}
		dirty = true;
		
	} else {
		aDownload->getQueueItem()->setStatus(QueueItem::WAITING);
		aDownload->getQueueItem()->setCurrent(NULL);
		fire(QueueManagerListener::SOURCES_UPDATED, aDownload->getQueueItem());
		delete aDownload;
	}
}

void QueueManager::remove(const string& aTarget) throw(QueueException) {
	
	Lock l(cs);

	QueueItem* q = findByTarget(aTarget);
	if(q != NULL) {
		if(q->getStatus() == QueueItem::RUNNING) {
			DownloadManager::getInstance()->removeDownload(q);
		}
		
		QueueItem::Iter i = find(queue.begin(), queue.end(), q);
		if(i != queue.end()) {
			queue.erase(i);
			fire(QueueManagerListener::REMOVED, q);
			dirty = true;
			delete q;
		}
	}
}

void QueueManager::remove(QueueItem* aQI) throw(QueueException) {
	dcassert(aQI != NULL);
	
	QueueItem::Iter i = find(queue.begin(), queue.end(), aQI);
	if(i != queue.end()) {
		queue.erase(i);
		fire(QueueManagerListener::REMOVED, aQI);
		dirty = true;
		delete aQI;
	}
}

void QueueManager::removeSource(const string& aTarget, User::Ptr& aUser, bool removeConn /* = true */)  {
	Lock l(cs);
	QueueItem* q = findByTarget(aTarget);
	if(q != NULL) {

		if(removeConn && q->getStatus() == QueueItem::RUNNING) {
			dcassert(q->getCurrent());
			if(q->getCurrent()->getUser() == aUser) {
				// Oops...
				DownloadManager::getInstance()->removeDownload(q);
			}
		}
		if(q->isSet(QueueItem::USER_LIST)) {
			remove(q);
		} else {
			q->removeSource(aUser);
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

void QueueManager::save(SimpleXML* aXml) {
	Lock l(cs);
	
	aXml->addTag("Downloads");
	aXml->stepIn();
	for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
		QueueItem::Ptr d = *i;
		if(!d->isSet(QueueItem::USER_LIST)) {
			aXml->addTag("Download");
			aXml->addChildAttrib("Target", d->getTarget());
			aXml->addChildAttrib("Resume", d->isSet(QueueItem::RESUME));
			aXml->addChildAttrib("Size", d->getSize());
			aXml->addChildAttrib("Priority", d->getPriority());

			aXml->stepIn();
			for(QueueItem::Source::List::const_iterator j = d->sources.begin(); j != d->sources.end(); ++j) {
				QueueItem::Source* s = *j;
				aXml->addTag("Source");
				aXml->addChildAttrib("Nick", s->getUser()->getNick());
				aXml->addChildAttrib("Path", s->getPath());
			}
			aXml->stepOut();
			
		}
	}
	aXml->stepOut();
	dirty = false;
	
}

void QueueManager::load(SimpleXML* aXml) {
	Lock l(cs);
	aXml->resetCurrentChild();
	if(aXml->findChild("Downloads")) {
		aXml->stepIn();
		
		while(aXml->findChild("Download")) {
			const string& target = aXml->getChildAttrib("Target");
			bool resume = aXml->getBoolChildAttrib("Resume");
			LONGLONG size = aXml->getLongLongChildAttrib("Size");
			aXml->stepIn();
			while(aXml->findChild("Source")) {
				const string& nick = aXml->getChildAttrib("Nick");
				const string& path = aXml->getChildAttrib("Path");
				
				/** @todo Remove this when a few versions have passed (from 0.14...) */
				const string& file = aXml->getChildAttrib("FileName");
				
				try {
					add(path + file, size, ClientManager::getInstance()->getUser(nick), target, resume);
				} catch(Exception e) {
					// ...
				}
			}
			aXml->stepOut();
		}
		aXml->stepOut();
	}
}

void QueueManager::onAction(SearchManagerListener::Types, SearchResult* sr) {
	
	if(BOOLSETTING(AUTO_SEARCH) && sr->getUser()) {
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

/**
 * @file QueueManager.cpp
 * $Id: QueueManager.cpp,v 1.9 2002/02/27 12:02:09 arnetheduck Exp $
 * @if LOG
 * $Log: QueueManager.cpp,v $
 * Revision 1.9  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.8  2002/02/25 15:39:29  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.7  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * @endif
 */


