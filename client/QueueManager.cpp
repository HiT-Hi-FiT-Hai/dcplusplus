// QueueManager.cpp: implementation of the QueueManager class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DCPlusPlus.h"

#include "QueueManager.h"
#include "ConnectionManager.h"
#include "ClientManager.h"
#include "DownloadManager.h"
#include "UserConnection.h"
#include "SimpleXML.h"

QueueManager* QueueManager::instance = NULL;

const string QueueManager::USER_LIST_NAME = "MyList.DcLst";

void QueueManager::onTimerMinute(DWORD aTick) {

	// Avoid firing while holding cs:s...
	QueueItem::List updated;

	{
		Lock l(cs);

		for(QueueItem::Iter i = queue.begin(); i != queue.end(); ++i) {
			QueueItem* q = *i;
			
			if(q->getStatus() == QueueItem::RUNNING) {
				continue;
			}
			
			if(q->updateUsers())
				updated.push_back(q);
			
		}
	}

	for(QueueItem::Iter i = updated.begin(); i != updated.end(); ++i) {
		fire(QueueManagerListener::SOURCES_UPDATED, *i);
	}

	if(dirty) {
		SettingsManager::getInstance()->save();		
	}
	
}

bool QueueItem::updateUsers() {
	bool updated = false;
	for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
		Source* s = *i;
		if( !s->getUser()) {
			s->setUser(ClientManager::getInstance()->findUser(s->getNick()));
			if(!s->getUser()) {
				continue;
			}
			
			// Try to connect...
			ConnectionManager::getInstance()->getDownloadConnection(s->getUser());
			updated = true;
		}

		if( !s->getUser()->isOnline() ) {

			if(DownloadManager::getInstance()->isConnected(s->getUser())) {
				continue;
			}

			updated = true;
			s->setUser(ClientManager::getInstance()->findUser(s->getNick()));
			if(!s->getUser()) {
				continue;
			}
		}
	}

	return updated;
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

	// And make sure we're trying to connect to this fellow...
	ConnectionManager::getInstance()->getDownloadConnection(aUser);
}

void QueueManager::add(const string& aFile, LONGLONG aSize, const string& aNick, const string& aTarget, 
					   bool aResume) throw(QueueException, FileException) {
	User::Ptr& u = ClientManager::getInstance()->findUser(aNick);
	if(u) {
		add(aFile, aSize, u, aTarget, aResume);
	}

	// Alright, first we get a queue item, new or old...
	bool newItem = false;
	QueueItem* q = getQueueItem(aFile, aTarget, aSize, aResume, newItem);

	QueueItem::Source* s = NULL;
	{
		Lock l(cs);
		s = q->addSource(aNick, aFile);
		if(newItem) {
			queue.push_back(q);
		}
	}
	
	// Good, now notify the listeners of the changes
	if(newItem)
		fire(QueueManagerListener::ADDED, q);
	
	fire(QueueManagerListener::SOURCES_UPDATED, q);
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

		if( (q->getSize() != -1) && (q->getSize() >= File::getSize(aTarget)) )  {
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
	}
	fire(QueueManagerListener::STATUS_UPDATED, q);
	return new Download(q, ConnectionManager::getInstance()->getQueueItem(aUserConnection));
}

void QueueManager::putDownload(Download* aDownload, bool finished /* = false */) {

	if(finished) {
		aDownload->getQueueItem()->setStatus(QueueItem::FINISHED);
		fire(QueueManagerListener::STATUS_UPDATED, aDownload->getQueueItem());

		if(BOOLSETTING(REMOVE_FINISHED)) {
			{
				Lock l(cs);
				dcassert(find(queue.begin(), queue.end(), aDownload->getQueueItem()) != queue.end());

				queue.erase(find(queue.begin(), queue.end(), aDownload->getQueueItem()));
			}

			fire(QueueManagerListener::REMOVED, aDownload->getQueueItem());

			delete aDownload->getQueueItem();
			delete aDownload;
		}
	} else {
		aDownload->getQueueItem()->setStatus(QueueItem::WAITING);
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

			delete q;
		}
	}
}

void QueueManager::remove(QueueItem* qi) throw(QueueException) {
	
	Lock l(cs);
	QueueItem::Iter i = find(queue.begin(), queue.end(), qi);

	if(i != queue.end()) {
		if(qi->getStatus() == QueueItem::RUNNING) {
			DownloadManager::getInstance()->removeDownload(qi);
		}
		
		queue.erase(i);
		fire(QueueManagerListener::REMOVED, qi);
		
		delete qi;
	}
}

void QueueManager::removeSource(const string& aTarget, const string& aUser)  {
	Lock l(cs);
	QueueItem* q = findByTarget(aTarget);
	if(q != NULL) {

		if(q->getStatus() == QueueItem::RUNNING) {
			dcassert(q->getCurrent());
			if(q->getCurrent()->getUser()->getNick() == aUser) {
				// Oops...
				DownloadManager::getInstance()->removeDownload(q);
			}
		}
		q->removeSource(aUser);
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
			aXml->addChildAttrib("Resume", d->isSet(Download::RESUME));
			aXml->addChildAttrib("Size", d->getSize());
			aXml->addChildAttrib("Priority", d->getPriority());

			aXml->stepIn();
			for(QueueItem::Source::List::const_iterator j = d->sources.begin(); j != d->sources.end(); ++j) {
				QueueItem::Source* s = *j;
				aXml->addTag("Source");
				aXml->addChildAttrib("Nick", s->getNick());
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
					add(path + file, size, nick, target, resume);
				} catch(...) {
					// ...
				}
			}
			aXml->stepOut();
		}
		aXml->stepOut();
	}
}

