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

#include "DownloadManager.h"
#include "ConnectionManager.h"
#include "User.h"
#include "QueueManager.h"
#include "LogManager.h"
#include "CryptoManager.h"
#include "SFVReader.h"

DownloadManager* Singleton<DownloadManager>::instance = NULL;

static const string DOWNLOAD_AREA = "Downloads";
static const string ANTI_FRAG_EXT = ".antifrag";

Download::Download(QueueItem* qi) throw() : source(qi->getCurrent()->getPath()),
	target(qi->getTarget()), tempTarget(qi->getTempTarget()), 
	comp(NULL), bytesLeft(0), rollbackBuffer(NULL), rollbackSize(0) { 
	
	setSize(qi->getSize());
	if(qi->isSet(QueueItem::FLAG_USER_LIST))
		setFlag(Download::FLAG_USER_LIST);
	if(qi->isSet(QueueItem::FLAG_RESUME))
		setFlag(Download::FLAG_RESUME);
};

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

void DownloadManager::FileMover::moveFile(const string& source, const string& target) {
	Lock l(cs);
	files.push_back(make_pair(source, target));
	if(!active) {
		active = true;
		start();
	}
}

int DownloadManager::FileMover::run() {
	for(;;) {
		FilePair next;
		{
			Lock l(cs);
			if(files.empty()) {
				active = false;
				return 0;
			}
			next = files.back();
			files.pop_back();
		}
		try {
			File::renameFile(next.first, next.second);
		} catch(const FileException&) {
			// Too bad...
		}
	}
}

void DownloadManager::removeConnection(UserConnection::Ptr aConn, bool reuse /* = false */) {
	dcassert(aConn->getDownload() == NULL);
	aConn->removeListener(this);
	ConnectionManager::getInstance()->putDownloadConnection(aConn, reuse);
}

void DownloadManager::checkDownloads(UserConnection* aConn) {

	if( ((SETTING(DOWNLOAD_SLOTS) != 0) && getDownloads() >= SETTING(DOWNLOAD_SLOTS)) ||
		((SETTING(MAX_DOWNLOAD_SPEED) != 0 && getAverageSpeed() >= (SETTING(MAX_DOWNLOAD_SPEED)*1024)) ) ) {
		
		if(!QueueManager::getInstance()->hasDownload(aConn->getUser(), QueueItem::HIGHEST)) {
			removeConnection(aConn);
			return;
		}
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
		
		if(d->isSet(Download::FLAG_RESUME)) {
			int64_t size = File::getSize(d->getTempTarget().empty() ? d->getTarget() : d->getTempTarget());
			int rollback = SETTING(ROLLBACK);
			int cutoff = max(SETTING(ROLLBACK), SETTING(BUFFER_SIZE)*1024);

			// dcassert(d->getSize() != -1);
			if( (rollback + cutoff) > min(size, d->getSize()) ) {
				d->setPos(0);
			} else {
				d->setPos(size - rollback - cutoff);
				d->setRollbackBuffer(rollback);
				d->setFlag(Download::FLAG_ROLLBACK);
			}
		} else {
			d->setPos(0);
		}
		if(d->isSet(Download::FLAG_USER_LIST) && aConn->isSet(UserConnection::FLAG_SUPPORTS_BZLIST)) {
			d->setSource("MyList.bz2");
		}

		if(BOOLSETTING(COMPRESS_TRANSFERS) && !d->isSet(Download::FLAG_USER_LIST) && 
			aConn->isSet(UserConnection::FLAG_SUPPORTS_GETZBLOCK)) {

			// This one, we'll download with a bzblock download instead...
			d->setFlag(Download::FLAG_ZDOWNLOAD);
			d->bytesLeft = d->getSize() - d->getPos();
			d->setComp(new ZDecompressor());
			aConn->getZBlock(d->getSource(), d->getPos(), d->bytesLeft);
		} else {
			aConn->get(d->getSource(), d->getPos());
		}
		return;
	}

	// Connection not needed any more, return it to the ConnectionManager...
	removeConnection(aConn, true);
}

void DownloadManager::onSending(UserConnection* aSource) {
	if(aSource->getState() != UserConnection::STATE_FILELENGTH) {
		dcdebug("DM::onFileLength Bad state, ignoring\n");
		return;
	}
	
	if(prepareFile(aSource)) {
		aSource->setDataMode();
	}
}

bool DownloadManager::prepareFile(UserConnection* aSource, int64_t newSize /* = -1 */) {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	if(newSize != -1)
		d->setSize(newSize);

	dcassert(d->getSize() != -1);

	string target = d->getTempTarget().empty() ? d->getTarget() : d->getTempTarget();
	Util::ensureDirectory(target);
	if(d->isSet(Download::FLAG_USER_LIST) && aSource->isSet(UserConnection::FLAG_SUPPORTS_BZLIST)) {
		target.replace(target.size() - 5, 5, "bz2");
	}
	File* file;
	try {
		// Let's check if we can find this file in a any .SFV...
		int trunc = d->isSet(Download::FLAG_RESUME) ? 0 : File::TRUNCATE;
		bool sfvcheck = BOOLSETTING(SFV_CHECK) && (d->getPos() == 0) && (SFVReader(d->getTarget()).hasCRC());
		
		if(BOOLSETTING(ANTI_FRAG) && !d->isSet(Download::FLAG_USER_LIST)) {
			// Anti-frag file...First, remove any old attempt that might have existed
			// and rename any partial file alread downloaded...
			string atarget = target + ANTI_FRAG_EXT;
			try {
				File::deleteFile(atarget);
				File::renameFile(target, atarget);
			} catch(const FileException& e) {
				dcdebug("AntiFrag: %s\n", e.getError().c_str());
			}
			file = new SizedFile(d->getSize(), atarget, File::RW, File::OPEN | File::CREATE | trunc, sfvcheck);

			d->setFlag(Download::FLAG_ANTI_FRAG);
		} else {
			file = new BufferedFile(target, File::RW, File::OPEN | File::CREATE | trunc, sfvcheck);			
		}

		file->setPos(d->getPos());
		
	} catch(const FileException& e) {
		fire(DownloadManagerListener::FAILED, d, STRING(COULD_NOT_OPEN_TARGET_FILE) + e.getError());
		aSource->setDownload(NULL);
		removeDownload(d);
		removeConnection(aSource);
		return false;
	}

	dcassert(d->getPos() != -1);
	d->setFile(file);
	
	if(d->getSize() <= d->getPos()) {
		aSource->setDownload(NULL);
		removeDownload(d, true);
		removeConnection(aSource);
		return false;
	} else {
		d->setStart(GET_TICK());
		aSource->setState(UserConnection::STATE_DONE);
		
		fire(DownloadManagerListener::STARTING, d);
		
	}
	
	return true;
}	

void DownloadManager::onFileLength(UserConnection* aSource, const string& aFileLength) {

	if(aSource->getState() != UserConnection::STATE_FILELENGTH) {
		dcdebug("DM::onFileLength Bad state, ignoring\n");
		return;
	}

	int64_t fileLength = Util::toInt64(aFileLength);
	if(prepareFile(aSource, fileLength)) {
		aSource->setDataMode(aSource->getDownload()->getSize() - aSource->getDownload()->getPos());
		aSource->startSend();
	}
}

bool DownloadManager::checkRollback(Download* d, const u_int8_t* aData, int aLen) throw(FileException) {
	dcassert(d->getRollbackBuffer());
	
	if(d->getTotal() + aLen >= d->getRollbackSize()) {
		AutoArray<u_int8_t> buf(d->getRollbackSize());
		int len = d->getRollbackSize() - (int)d->getTotal();
		dcassert(len > 0);
		dcassert(len <= d->getRollbackSize());
		memcpy(d->getRollbackBuffer() + d->getTotal(), aData, len);
		
		d->getFile()->read((u_int8_t*)buf, d->getRollbackSize());
		
		int cmp = memcmp(d->getRollbackBuffer(), buf, d->getRollbackSize());
		
		d->unsetFlag(Download::FLAG_ROLLBACK);
		d->setRollbackBuffer(0);
		
		if(cmp != 0) {
			return false;
		}
		if(!d->isSet(Download::FLAG_ANTI_FRAG))
			d->getFile()->setEOF();
		// Write the rest...the file pointer should have been moved to the correct position by now...
		d->getFile()->write(aData+len, aLen - len);
	} else {
		memcpy(d->getRollbackBuffer() + d->getTotal(), aData, aLen);
	}
	
	return true;
}

void DownloadManager::onData(UserConnection* aSource, const u_int8_t* aData, int aLen) {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	if(d->isSet(Download::FLAG_ZDOWNLOAD)) {
		// Oops, this is a bit more work...
		dcassert(d->getComp() != NULL);
		dcassert(d->bytesLeft > 0);
		int l = aLen;
		while(l > 0) {
			const u_int8_t* data = aData + (aLen - l);
			try {
				u_int32_t b = d->getComp()->decompress(data, l);
				if(!handleData(aSource, d->getComp()->getOutbuf(), b))
					break;
				d->bytesLeft -= b;
				if(d->bytesLeft == 0) {
					aSource->setLineMode();
					handleEndData(aSource);
					if(l != 0) {
						// Uhm, this client must be sending junk data after the compressed block...
						aSource->disconnect();
						return;
					}
				}
			} catch(const CryptoException&) {
				// Oops, decompression error...could happen for many reasons
				// but the most probable is that we received bad data...
				// We remove whatever we managed to download in this sessions
				// as it might be bad all of it...
				try {
					d->getFile()->movePos(-d->getTotal());
					d->getFile()->setEOF();
				} catch(const FileException&) {
					// Ignore...
				}

				fire(DownloadManagerListener::FAILED, d, STRING(DECOMPRESSION_ERROR));

				aSource->setDownload(NULL);
				removeDownload(d);
				removeConnection(aSource);
				return;
			}
		}
	} else {
		handleData(aSource, aData, aLen);
	}
}

bool DownloadManager::handleData(UserConnection* aSource, const u_int8_t* aData, int aLen) {
	dcassert(aSource->getState() == UserConnection::STATE_DONE);

	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	try {
		if(d->isSet(Download::FLAG_ROLLBACK)) {
			if(!checkRollback(d, aData, aLen)) {
				fire(DownloadManagerListener::FAILED, d, STRING(ROLLBACK_INCONSISTENCY));
				
				string target = d->getTarget();
				
				aSource->setDownload(NULL);
				removeDownload(d);				

				QueueManager::getInstance()->removeSource(target, aSource->getUser(), QueueItem::Source::FLAG_ROLLBACK_INCONSISTENCY);
				removeConnection(aSource);
				return false;
			} 
		} else {
			d->getFile()->write(aData, aLen);
		}
		d->addPos(aLen);
	} catch(const FileException& e) {
		fire(DownloadManagerListener::FAILED, d, e.getError());
		
		aSource->setDownload(NULL);
		removeDownload(d);
		removeConnection(aSource);
		return false;
	}
	return true;
}

void DownloadManager::onModeChange(UserConnection* aSource, int /*aNewMode*/) {
	handleEndData(aSource);
}

/** Download finished! */
void DownloadManager::handleEndData(UserConnection* aSource) {

	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	// First, finish writing the file (flushing the buffers and closing the file...)
	try {
		d->getFile()->close();

		// Check if we're anti-fragging...
		if(d->isSet(Download::FLAG_ANTI_FRAG)) {
			// Ok, rename the file to what we expect it to be...
			try {
				const string& tgt = d->getTempTarget().empty() ? d->getTarget() : d->getTempTarget();
				File::renameFile(tgt + ANTI_FRAG_EXT, tgt);
				d->unsetFlag(Download::FLAG_ANTI_FRAG);
			} catch(const FileException& e) {
				dcdebug("AntiFrag: %s\n", e.getError().c_str());
				// Now what?
			}
		}
	} catch(const FileException& e) {
		fire(DownloadManagerListener::FAILED, d, e.getError());
		
		aSource->setDownload(NULL);
		removeDownload(d);
		removeConnection(aSource);
		return;
	}
	
	dcassert(d->getPos() == d->getSize());
	dcdebug("Download finished: %s, size %I64d\n", d->getTarget().c_str(), d->getSize());

	// Check if we have some crc:s...
	dcassert(d->getFile() != NULL);

	if(BOOLSETTING(SFV_CHECK)) {
		d->getFile()->close();
		SFVReader sfv(d->getTarget());
		if(sfv.hasCRC()) {
			bool crcMatch;
			const string& tgt = d->getTempTarget().empty() ? d->getTarget() : d->getTempTarget();
			if(d->getFile()->hasCRC32()) {
				crcMatch = (d->getFile()->getCRC32() == sfv.getCRC());
			} else {
				// More complicated, we have to reread the file
				try {
					
					File f(tgt, File::READ, File::OPEN, true);
					const u_int32_t BUF_SIZE = 16 * 65536;
					AutoArray<u_int8_t> b(BUF_SIZE);
					while(f.read((u_int8_t*)b, BUF_SIZE) > 0)
						;		// Keep on looping...

					crcMatch = (f.getCRC32() == sfv.getCRC());
				} catch (FileException&) {
					// Nope; read failed...
					goto noCRC;
				}
			}

			if(!crcMatch) {
				File::deleteFile(tgt);
				dcdebug("DownloadManager: CRC32 mismatch for %s\n", d->getTarget().c_str());
				fire(DownloadManagerListener::FAILED, d, STRING(SFV_INCONSISTENCY));
				
				string target = d->getTarget();
				
				aSource->setDownload(NULL);
				removeDownload(d);				
				
				QueueManager::getInstance()->removeSource(target, aSource->getUser(), QueueItem::Source::FLAG_CRC_WARN, false);
				checkDownloads(aSource);
				return;
			} 

			d->setFlag(Download::FLAG_CRC32_OK);
			
			dcdebug("DownloadManager: CRC32 match for %s\n", d->getTarget().c_str());
		}
	}
noCRC:
	delete d->getFile();
	d->setFile(NULL);
	
	if(BOOLSETTING(LOG_DOWNLOADS)) {
		StringMap params;
		params["target"] = d->getTarget();
		params["user"] = aSource->getUser()->getNick();
		params["hub"] = aSource->getUser()->getLastHubName();
		params["hubip"] = aSource->getUser()->getLastHubAddress();
		params["size"] = Util::toString(d->getSize());
		params["sizeshort"] = Util::formatBytes(d->getSize());
		params["chunksize"] = Util::toString(d->getTotal());
		params["chunksizeshort"] = Util::formatBytes(d->getTotal());
		params["speed"] = Util::formatBytes(d->getAverageSpeed()) + "/s";
		params["time"] = Util::formatSeconds((GET_TICK() - d->getStart()) / 1000);
		params["sfv"] = Util::toString(d->isSet(Download::FLAG_CRC32_OK) ? 1 : 0);
		LOG(DOWNLOAD_AREA, Util::formatParams(SETTING(LOG_FORMAT_POST_DOWNLOAD), params));
	}

	// Check if we need to move the file
	if( !d->getTempTarget().empty() && (Util::stricmp(d->getTarget().c_str(), d->getTempTarget().c_str()) != 0) ) {
		try {
			Util::ensureDirectory(d->getTarget());
			if(File::getSize(d->getTempTarget()) > MOVER_LIMIT) {
				mover.moveFile(d->getTempTarget(), d->getTarget());
			} else {
				File::renameFile(d->getTempTarget(), d->getTarget());
			}
			d->setTempTarget(Util::emptyString);
		} catch(const FileException&) {
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
	
	if(aError.find("File Not Available") != string::npos) {
		QueueManager::getInstance()->removeSource(target, aSource->getUser(), QueueItem::Source::FLAG_FILE_NOT_AVAILABLE, false);
	}

	removeConnection(aSource);
}

void DownloadManager::removeDownload(Download* d, bool finished /* = false */) {
	if(d->getFile()) {
		try {
			if(d->isSet(Download::FLAG_ANTI_FRAG)) {
				// Ok, set the pos to whereever it was last writing and hope for the best...
				d->getFile()->close();
				const string& tgt = d->getTempTarget().empty() ? d->getTarget() : d->getTempTarget();
				File::renameFile(tgt + ANTI_FRAG_EXT, tgt);
				d->unsetFlag(Download::FLAG_ANTI_FRAG);
			} else {
				d->getFile()->close();
			}
			delete d->getFile();
		} catch(const FileException&) {
			finished = false;
		}

		d->setFile(NULL);
	}

	if(d->getComp()) {
		delete d->getComp();
		d->setComp(NULL);
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

void DownloadManager::onFileNotAvailable(UserConnection* aSource) throw() {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	dcdebug("File Not Available: %s\n", d->getTarget().c_str());

	if(d->getFile()) {
		delete d->getFile();
		d->setFile(NULL);
	}

	fire(DownloadManagerListener::FAILED, d, d->getTargetFileName() + ": " + STRING(FILE_NOT_AVAILABLE));

	aSource->setDownload(NULL);

	QueueManager::getInstance()->removeSource(d->getTarget(), aSource->getUser(), QueueItem::Source::FLAG_FILE_NOT_AVAILABLE, false);
	removeDownload(d, false);
	checkDownloads(aSource);
}

// UserConnectionListener
void DownloadManager::onAction(UserConnectionListener::Types type, UserConnection* conn) throw() {
	switch(type) {
	case UserConnectionListener::MAXED_OUT: onMaxedOut(conn); break;
	case UserConnectionListener::FILE_NOT_AVAILABLE: onFileNotAvailable(conn); break;
	case UserConnectionListener::SENDING: onSending(conn); break;
	default: break;
	}
}
void DownloadManager::onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line) throw() {
	switch(type) {
	case UserConnectionListener::FILE_LENGTH:
		onFileLength(conn, line); break;
	case UserConnectionListener::FAILED:
		onFailed(conn, line); break;
	default:
		break;
	}
}
void DownloadManager::onAction(UserConnectionListener::Types type, UserConnection* conn, const u_int8_t* data, int len) throw() {
	switch(type) {
	case UserConnectionListener::DATA:
		onData(conn, data, len); break;
	default:
		break;
	}
}

void DownloadManager::onAction(UserConnectionListener::Types type, UserConnection* conn, int mode) throw() {
	switch(type) {
	case UserConnectionListener::MODE_CHANGE:
		onModeChange(conn, mode); break;
	default:
		break;
	}
}

// TimerManagerListener
void DownloadManager::onAction(TimerManagerListener::Types type, u_int32_t aTick) throw() {
	switch(type) {
	case TimerManagerListener::SECOND:
		onTimerSecond(aTick); break;
	default:
		break;
	}
}

/**
 * @file
 * $Id: DownloadManager.cpp,v 1.75 2003/09/22 13:17:22 arnetheduck Exp $
 */
