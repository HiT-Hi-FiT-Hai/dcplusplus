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

#include "DownloadManager.h"
#include "ConnectionManager.h"
#include "User.h"
#include "QueueManager.h"
#include "LogManager.h"

DownloadManager* Singleton<DownloadManager>::instance = NULL;

static string DOWNLOAD_AREA = "Downloads";

void DownloadManager::onTimerSecond(u_int32_t /*aTick*/) {
	Lock l(cs);

	Download::List tickList;
	// Tick each ongoing download
	for(Download::Iter i = downloads.begin(); i != downloads.end(); ++i) {
		if((*i)->getTotal() > 0) {
			tickList.push_back(*i);
		}
	}

	if(tickList.size() > 0)
		fire(DownloadManagerListener::TICK, tickList);
}

void DownloadManager::removeConnection(UserConnection::Ptr aConn, bool reuse /* = false */) {
	dcassert(aConn->getDownload() == NULL);
	aConn->removeListener(this);
	ConnectionManager::getInstance()->putDownloadConnection(aConn, reuse);
}

void DownloadManager::checkDownloads(UserConnection* aConn) {

	if( ((SETTING(DOWNLOAD_SLOTS) != 0) && getDownloads() >= SETTING(DOWNLOAD_SLOTS)) ||
		((SETTING(MAX_DOWNLOAD_SPEED) != 0 && getAverageSpeed() >= (SETTING(MAX_DOWNLOAD_SPEED)*1024)) ) ) {
		
		removeConnection(aConn);
		return;
	}
	
	Download* d = QueueManager::getInstance()->getDownload(aConn->getUser());
	
	if(d) {
		dcassert(aConn->getDownload() == NULL);
		d->setUserConnection(aConn);
		aConn->setDownload(d);
		aConn->setState(UserConnection::STATE_FILELENGTH);

		{
			Lock l(cs);
			downloads.push_back(d);
		}
		
		if(d->isSet(Download::RESUME)) {
			int64_t size = File::getSize(d->getTempTarget().empty() ? d->getTarget() : d->getTempTarget());
			int rollback = SETTING(ROLLBACK);
			int cutoff = max(SETTING(ROLLBACK), SETTING(BUFFER_SIZE)*1024);

			// dcassert(d->getSize() != -1);
			if( (rollback + cutoff) > min(size, d->getSize()) ) {
				d->setPos(0);
			} else {
				d->setPos(size - rollback - cutoff);
				d->setRollbackBuffer(rollback);
				d->setFlag(Download::ROLLBACK);
			}
		} else {
			d->setPos(0);
		}
		if(d->isSet(Download::USER_LIST) && aConn->isSet(UserConnection::FLAG_SUPPORTS_BZLIST)) {
			d->setSource("MyList.bz2");
		}
		aConn->get(d->getSource(), d->getPos());
		return;
	}

	// Connection not needed any more, return it to the ConnectionManager...
	removeConnection(aConn, true);
}

void DownloadManager::onFileLength(UserConnection* aSource, const string& aFileLength) {
	if(aSource->getState() != UserConnection::STATE_FILELENGTH) {
		dcdebug("DM::onFileLength Bad state, ignoring\n");
		return;
	}

	int64_t fileLength = Util::toInt64(aFileLength);
	
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	string target = d->getTempTarget().empty() ? d->getTarget() : d->getTempTarget();
	Util::ensureDirectory(target);
	if(d->isSet(Download::USER_LIST) && aSource->isSet(UserConnection::FLAG_SUPPORTS_BZLIST)) {
		target.replace(target.size() - 5, 5, "bz2");
	}
	File* file;
	try {
		file = new BufferedFile(target, File::WRITE | File::READ, File::OPEN | File::CREATE | (d->isSet(Download::RESUME) ? 0 : File::TRUNCATE));
	} catch(FileException e) {
		fire(DownloadManagerListener::FAILED, d, STRING(COULD_NOT_OPEN_TARGET_FILE) + e.getError());
		aSource->setDownload(NULL);
		removeDownload(d);
		removeConnection(aSource);
		return;
	}

	dcassert(d->getPos() != -1);
	file->setPos(d->getPos());
	d->setFile(file);
	d->setSize(fileLength);

	if(d->getSize() <= d->getPos()) {
		aSource->setDownload(NULL);
		removeDownload(d, true);
		removeConnection(aSource);
	} else {
		d->setStart(GET_TICK());
		aSource->setState(UserConnection::STATE_DONE);
		
		fire(DownloadManagerListener::STARTING, d);
		
		aSource->setDataMode(d->getSize() - d->getPos());
		aSource->startSend();
	}
}

bool DownloadManager::checkRollback(Download* d, const u_int8_t* aData, int aLen) throw(FileException) {
	dcassert(d->getRollbackBuffer());
	
	if(d->getTotal() + aLen >= d->getRollbackSize()) {
		u_int8_t* buf = new u_int8_t[d->getRollbackSize()];
		int len = d->getRollbackSize() - (int)d->getTotal();
		dcassert(len > 0);
		dcassert(len <= d->getRollbackSize());
		memcpy(d->getRollbackBuffer() + d->getTotal(), aData, len);
		
		try {
			d->getFile()->read(buf, d->getRollbackSize());
		} catch(...) {
			delete[] buf;
			throw;
		}
		
		int cmp = memcmp(d->getRollbackBuffer(), buf, d->getRollbackSize());
		
		delete[] buf;
		d->unsetFlag(Download::ROLLBACK);
		d->setRollbackBuffer(0);
		
		if(cmp != 0) {
			return false;
		}
		d->getFile()->setEOF();
		// Write the rest...the file pointer should have been moved to the correct position by now...
		d->getFile()->write(aData+len, aLen - len);
	} else {
		memcpy(d->getRollbackBuffer() + d->getTotal(), aData, aLen);
	}
	
	return true;
}

void DownloadManager::onData(UserConnection* aSource, const u_int8_t* aData, int aLen) {
	dcassert(aSource->getState() == UserConnection::STATE_DONE);

	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	try {
		if(d->isSet(Download::ROLLBACK)) {
			if(!checkRollback(d, aData, aLen)) {
				fire(DownloadManagerListener::FAILED, d, STRING(ROLLBACK_INCONSISTENCY));
				
				string target = d->getTarget();
				
				aSource->setDownload(NULL);
				removeDownload(d);				

				QueueManager::getInstance()->removeSource(target, aSource->getUser());
				removeConnection(aSource);
				return;
			} 
		} else {
			d->getFile()->write(aData, aLen);
		}
		d->addPos(aLen);
	} catch(FileException e) {
		fire(DownloadManagerListener::FAILED, d, e.getError());
		
		aSource->setDownload(NULL);
		removeDownload(d);
		removeConnection(aSource);
		return;
	}
}

/** Download finished! */
void DownloadManager::onModeChange(UserConnection* aSource, int /*aNewMode*/) {

	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	dcassert(d->getPos() == d->getSize());
	dcdebug("Download finished: %s, size %I64d\n", d->getTarget().c_str(), d->getSize());

	if(d->getFile()) {
		delete d->getFile();
		d->setFile(NULL);
	}
	
	if(BOOLSETTING(LOG_DOWNLOADS)) {
		StringMap params;
		params["target"] = d->getTarget();
		params["user"] = aSource->getUser()->getNick();
		params["size"] = Util::toString(d->getSize());
		params["sizeshort"] = Util::formatBytes(d->getSize());
		params["chunksize"] = Util::toString(d->getTotal());
		params["chunksizeshort"] = Util::formatBytes(d->getTotal());
		params["speed"] = Util::formatBytes(d->getAverageSpeed()) + "/s";
		params["time"] = Util::formatSeconds((GET_TICK() - d->getStart()) / 1000);
		LOG(DOWNLOAD_AREA, Util::formatParams(SETTING(LOG_FORMAT_POST_DOWNLOAD), params));
	}

	// Check if we need to move the file
	if( !d->getTempTarget().empty() && (Util::stricmp(d->getTarget().c_str(), d->getTempTarget().c_str()) != 0) ) {
		try {
			Util::ensureDirectory(d->getTarget());
			File::renameFile(d->getTempTarget(), d->getTarget());
			d->setTempTarget(Util::emptyString);
		} catch(FileException e) {
			// Huh??? Now what??? Oh well...let it be...
		}
	}
	fire(DownloadManagerListener::COMPLETE, d);
	
	aSource->setDownload(NULL);
	removeDownload(d, true);
	checkDownloads(aSource);
}

void DownloadManager::onMaxedOut(UserConnection* aSource) { 
	if(aSource->getState() != UserConnection::STATE_FILELENGTH) {
		dcdebug("DM::onMaxedOut Bad state, ignoring\n");
		return;
	}

	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	fire(DownloadManagerListener::FAILED, d, STRING(NO_SLOTS_AVAILABLE));

	aSource->setDownload(NULL);
	removeDownload(d);
	removeConnection(aSource);
}

void DownloadManager::onFailed(UserConnection* aSource, const string& aError) {
	Download* d = aSource->getDownload();

	if(d == NULL) {
		removeConnection(aSource);
		return;
	}
	
	fire(DownloadManagerListener::FAILED, d, aError);

	string target = d->getTarget();
	aSource->setDownload(NULL);
	removeDownload(d);
	
	if(aError == "File Not Available") {
		QueueManager::getInstance()->removeSource(target, aSource->getUser(), false);
	}

	removeConnection(aSource);
}

void DownloadManager::removeDownload(Download* d, bool finished /* = false */) {
	if(d->getFile()) {
		delete d->getFile();
		d->setFile(NULL);
	}

	{
		Lock l(cs);
		// Either I'm stupid or the msvc7 optimizer is doing something _very_ strange here...
		// STL-port -D_STL_DEBUG complains that .begin() and .end() don't have the same owner (!),
		// but only in release build

		dcassert(find(downloads.begin(), downloads.end(), d) != downloads.end());

		//		downloads.erase(find(downloads.begin(), downloads.end(), d));
		
		for(Download::Iter i = downloads.begin(); i != downloads.end(); ++i) {
			if(*i == d) {
				downloads.erase(i);
				break;
			}
		}
	}
	QueueManager::getInstance()->putDownload(d, finished);
	
}

void DownloadManager::abortDownload(const string& aTarget) {
	Lock l(cs);
	for(Download::Iter i = downloads.begin(); i != downloads.end(); ++i) {
		Download* d = *i;
		if(d->getTarget() == aTarget) {
			dcassert(d->getUserConnection() != NULL);
			d->getUserConnection()->disconnect();
			break;
		}
	}
}

void DownloadManager::onFileNotAvailabe(UserConnection* aSource) {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	dcdebug("File Not Available: %s\n", d->getTarget().c_str());

	if(d->getFile()) {
		delete d->getFile();
		d->setFile(NULL);
	}

	fire(DownloadManagerListener::FAILED, d, STRING(FILE_NOT_AVAILABLE));

	aSource->setDownload(NULL);

	QueueManager::getInstance()->removeSource(d->getTarget(), aSource->getUser(), false);
	removeDownload(d, false);
	checkDownloads(aSource);
}

// UserConnectionListener
void DownloadManager::onAction(UserConnectionListener::Types type, UserConnection* conn) {
	switch(type) {
	case UserConnectionListener::MAXED_OUT: onMaxedOut(conn); break;
	case UserConnectionListener::FILE_NOT_AVAILABLE: onFileNotAvailabe(conn); break;
	}
}
void DownloadManager::onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line) {
	switch(type) {
	case UserConnectionListener::FILE_LENGTH:
		onFileLength(conn, line); break;
	case UserConnectionListener::FAILED:
		onFailed(conn, line); break;
	}
}
void DownloadManager::onAction(UserConnectionListener::Types type, UserConnection* conn, const u_int8_t* data, int len) {
	switch(type) {
	case UserConnectionListener::DATA:
		onData(conn, data, len); break;
	}
}

void DownloadManager::onAction(UserConnectionListener::Types type, UserConnection* conn, int mode) {
	switch(type) {
	case UserConnectionListener::MODE_CHANGE:
		onModeChange(conn, mode); break;
	}
}

// TimerManagerListener
void DownloadManager::onAction(TimerManagerListener::Types type, u_int32_t aTick) {
	switch(type) {
	case TimerManagerListener::SECOND:
		onTimerSecond(aTick); break;
	}
}

/**
 * @file DownloadManger.cpp
 * $Id: DownloadManager.cpp,v 1.66 2002/06/13 18:46:59 arnetheduck Exp $
 */
