/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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

#include "ResourceManager.h"
#include "QueueManager.h"
#include "Download.h"
#include "LogManager.h"
#include "SFVReader.h"
#include "User.h"
#include "File.h"
#include "FilteredFile.h"
#include "MerkleCheckOutputStream.h"
#include "UserConnection.h"

#include <limits>

// some strange mac definition
#ifdef ff
#undef ff
#endif

namespace dcpp {

static const string DOWNLOAD_AREA = "Downloads";
const string Download::ANTI_FRAG_EXT = ".antifrag";

DownloadManager::DownloadManager() {
	TimerManager::getInstance()->addListener(this);
}

DownloadManager::~DownloadManager() throw() {
	TimerManager::getInstance()->removeListener(this);
	while(true) {
		{
			Lock l(cs);
			if(downloads.empty())
				break;
		}
		Thread::sleep(100);
	}
}

void DownloadManager::on(TimerManagerListener::Second, uint32_t aTick) throw() {
	typedef vector<pair<string, UserPtr> > TargetList;
	TargetList dropTargets;

	{
		Lock l(cs);

		DownloadList tickList;
		// Tick each ongoing download
		for(DownloadList::iterator i = downloads.begin(); i != downloads.end(); ++i) {
			if((*i)->getTotal() > 0) {
				tickList.push_back(*i);
			}
		}

		if(tickList.size() > 0)
			fire(DownloadManagerListener::Tick(), tickList);


		// Automatically remove or disconnect slow sources
		if((uint32_t)(aTick / 1000) % SETTING(AUTODROP_INTERVAL) == 0) {
			for(DownloadList::iterator i = downloads.begin(); i != downloads.end(); ++i) {
				Download* d = *i;
				uint64_t timeElapsed = GET_TICK() - d->getStart();
				uint64_t timeInactive = GET_TICK() - d->getUserConnection().getLastActivity();
				uint64_t bytesDownloaded = d->getTotal();
				bool timeElapsedOk = timeElapsed >= (uint32_t)SETTING(AUTODROP_ELAPSED) * 1000;
				bool timeInactiveOk = timeInactive <= (uint32_t)SETTING(AUTODROP_INACTIVITY) * 1000;
				bool speedTooLow = timeElapsedOk && timeInactiveOk && bytesDownloaded > 0 ?
					bytesDownloaded / timeElapsed * 1000 < (uint32_t)SETTING(AUTODROP_SPEED) : false;
				bool isUserList = d->getType() == Transfer::TYPE_FULL_LIST;
				bool onlineSourcesOk = isUserList ?
					true : QueueManager::getInstance()->countOnlineSources(d->getPath()) >= SETTING(AUTODROP_MINSOURCES);
				bool filesizeOk = !isUserList && d->getSize() >= ((int64_t)SETTING(AUTODROP_FILESIZE)) * 1024;
				bool dropIt = (isUserList && BOOLSETTING(AUTODROP_FILELISTS)) ||
					(filesizeOk && BOOLSETTING(AUTODROP_ALL));
				if(speedTooLow && onlineSourcesOk && dropIt) {
					if(BOOLSETTING(AUTODROP_DISCONNECT) && isUserList) {
						d->getUserConnection().disconnect();
					} else {
						dropTargets.push_back(make_pair(d->getPath(), d->getUser()));
					}
				}
			}
		}
	}
	for(TargetList::iterator i = dropTargets.begin(); i != dropTargets.end(); ++i) {
		QueueManager::getInstance()->removeSource(i->first, i->second, QueueItem::Source::FLAG_SLOW_SOURCE);
	}
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
			try {
				// Try to just rename it to the correct name at least
				string newTarget = Util::getFilePath(next.first) + Util::getFileName(next.second);
				File::renameFile(next.first, newTarget);
				LogManager::getInstance()->message(next.first + STRING(RENAMED_TO) + newTarget);
			} catch(const FileException& e) {
				LogManager::getInstance()->message(STRING(UNABLE_TO_RENAME) + next.first + ": " + e.getError());
			}
		}
	}
}

void DownloadManager::removeConnection(UserConnectionPtr aConn) {
	dcassert(aConn->getDownload() == NULL);
	aConn->removeListener(this);
	aConn->disconnect();

	Lock l(cs);
	idlers.erase(remove(idlers.begin(), idlers.end(), aConn), idlers.end());
}

void DownloadManager::checkIdle(const UserPtr& user) {
	Lock l(cs);
	for(UserConnectionList::iterator i = idlers.begin(); i != idlers.end(); ++i) {
		UserConnection* uc = *i;
		if(uc->getUser() == user) {
			idlers.erase(i);
			checkDownloads(uc);
			return;
		}
	}
}

void DownloadManager::addConnection(UserConnectionPtr conn) {
	if(!conn->isSet(UserConnection::FLAG_SUPPORTS_TTHF) || !conn->isSet(UserConnection::FLAG_SUPPORTS_ADCGET)) {
		// Can't download from these...
		conn->getUser()->setFlag(User::OLD_CLIENT);
		QueueManager::getInstance()->removeSource(conn->getUser(), QueueItem::Source::FLAG_NO_TTHF);
		removeConnection(conn);
	}

	conn->addListener(this);
	checkDownloads(conn);
}

bool DownloadManager::startDownload(QueueItem::Priority prio) {
	size_t downloadCount = getDownloadCount();

	bool full = (SETTING(DOWNLOAD_SLOTS) != 0) && (downloadCount >= (size_t)SETTING(DOWNLOAD_SLOTS));
	full = full || ((SETTING(MAX_DOWNLOAD_SPEED) != 0) && (getRunningAverage() >= (SETTING(MAX_DOWNLOAD_SPEED)*1024)));

	if(full) {
		bool extraFull = (SETTING(DOWNLOAD_SLOTS) != 0) && (getDownloadCount() >= (size_t)(SETTING(DOWNLOAD_SLOTS)+3));
		if(extraFull) {
			return false;
		}
		return prio == QueueItem::HIGHEST;
	}

	if(downloadCount > 0) {
		return prio != QueueItem::LOWEST;
	}

	return true;
}

void DownloadManager::checkDownloads(UserConnection* aConn) {
	dcassert(aConn->getDownload() == NULL);

	QueueItem::Priority prio = QueueManager::getInstance()->hasDownload(aConn->getUser());
	if(!startDownload(prio)) {
		removeConnection(aConn);
		return;
	}

	Download* d = QueueManager::getInstance()->getDownload(*aConn, aConn->isSet(UserConnection::FLAG_SUPPORTS_TTHL));

	if(!d) {
		Lock l(cs);
		idlers.push_back(aConn);
		return;
	}

	aConn->setState(UserConnection::STATE_SND);

	if(d->getType() == Transfer::TYPE_FULL_LIST) {
		if(aConn->isSet(UserConnection::FLAG_SUPPORTS_XML_BZLIST)) {
			d->setPath(Transfer::USER_LIST_NAME_BZ);
		} else {
			d->setPath(Transfer::USER_LIST_NAME);
		}
	}

	{
		Lock l(cs);
		downloads.push_back(d);
	}

	aConn->send(d->getCommand(aConn->isSet(UserConnection::FLAG_SUPPORTS_ZLIB_GET)));
}

void DownloadManager::on(AdcCommand::SND, UserConnection* aSource, const AdcCommand& cmd) throw() {
	if(aSource->getState() != UserConnection::STATE_SND) {
		dcdebug("DM::onFileLength Bad state, ignoring\n");
		return;
	}

	const string& type = cmd.getParam(0);
	int64_t start = Util::toInt64(cmd.getParam(2));
	int64_t bytes = Util::toInt64(cmd.getParam(3));

	if(type != Transfer::names[aSource->getDownload()->getType()]) {
		// Uhh??? We didn't ask for this...
		aSource->disconnect();
		return;
	}

	if(prepareFile(aSource, start, bytes, cmd.hasFlag("ZL", 4))) {
		aSource->setDataMode();
	}
}

bool DownloadManager::prepareFile(UserConnection* aSource, int64_t start, int64_t bytes, bool z) {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	if(bytes != -1 && d->getSize() == -1) {
		d->setSize(bytes);
	}
	
	if(d->getPos() >= d->getSize()) {
		// Already finished?
		removeDownload(d);
		QueueManager::getInstance()->putDownload(d, true);
		removeConnection(aSource);
		return false;
	}

	dcassert(d->getSize() != -1);

	try {
		QueueManager::getInstance()->setFile(d);
	} catch(const FileException& e) {
		failDownload(aSource, STRING(COULD_NOT_OPEN_TARGET_FILE) + e.getError());
		return false;
	} catch(const Exception& e) {
		failDownload(aSource, e.getError());
		return false;
	}

	if((d->getType() == Transfer::TYPE_FILE || d->getType() == Transfer::TYPE_FULL_LIST) && SETTING(BUFFER_SIZE) > 0 ) {
		d->setFile(new BufferedOutputStream<true>(d->getFile()));
	}
		
	bool sfvcheck = BOOLSETTING(SFV_CHECK) && d->getType() == Transfer::TYPE_FILE && (d->getPos() == 0) && (SFVReader(d->getPath()).hasCRC());

	if(sfvcheck) {
		d->setFlag(Download::FLAG_CALC_CRC32);
		Download::CrcOS* crc = new Download::CrcOS(d->getFile());
		d->setCrcCalc(crc);
		d->setFile(crc);
	}

	if(d->getType() == Transfer::TYPE_FILE) {
		typedef MerkleCheckOutputStream<TigerTree, true> MerkleStream;
		
		d->setFile(new MerkleStream(d->getTigerTree(), d->getFile(), d->getStartPos()));
		d->setFlag(Download::FLAG_TTH_CHECK);
	}

	if(z) {
		d->setFlag(Download::FLAG_ZDOWNLOAD);
		d->setFile(new FilteredOutputStream<UnZFilter, true>(d->getFile()));
	}

	dcassert(d->getPos() != -1);
	d->setStart(GET_TICK());
	aSource->setState(UserConnection::STATE_RUNNING);

	fire(DownloadManagerListener::Starting(), d);

	return true;
}

void DownloadManager::on(UserConnectionListener::Data, UserConnection* aSource, const uint8_t* aData, size_t aLen) throw() {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	try {
		d->addPos(d->getFile()->write(aData, aLen), aLen);

		if(d->getPos() > d->getSize()) {
			throw Exception(STRING(TOO_MUCH_DATA));
		} else if(d->getPos() == d->getSize()) {
			handleEndData(aSource);
			aSource->setLineMode(0);
		}
	} catch(const FileException& e) {
		failDownload(aSource, e.getError());
	} catch(const Exception& e) {
		// Nuke the bytes we have written, this is probably a compression error
		d->resetPos();
		failDownload(aSource, e.getError());
	}
}

/** Download finished! */
void DownloadManager::handleEndData(UserConnection* aSource) {

	dcassert(aSource->getState() == UserConnection::STATE_RUNNING);
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	if(d->getType() == Transfer::TYPE_TREE) {
		d->getFile()->flush();
		delete d->getFile();
		d->setFile(NULL);

		int64_t bl = 1024;
		while(bl * (int64_t)d->getTigerTree().getLeaves().size() < d->getTigerTree().getFileSize())
			bl *= 2;
		d->getTigerTree().setBlockSize(bl);
		d->getTigerTree().calcRoot();

		if(!(d->getTTH() == d->getTigerTree().getRoot())) {
			// This tree is for a different file, remove from queue...
			removeDownload(d);
			fire(DownloadManagerListener::Failed(), d, STRING(INVALID_TREE));

			QueueManager::getInstance()->removeSource(d->getPath(), aSource->getUser(), QueueItem::Source::FLAG_BAD_TREE, false);

			QueueManager::getInstance()->putDownload(d, false);

			checkDownloads(aSource);
			return;
		}
		d->setTreeValid(true);
	} else {

		// Hm, if the real crc == 0, we'll get a file reread extra, but what the heck...
		uint32_t crc = 0;

		// First, finish writing the file (flushing the buffers and closing the file...)
		try {
			d->getFile()->flush();
			if(d->getCrcCalc() != NULL)
				crc = d->getCrcCalc()->getFilter().getValue();
			delete d->getFile();
			d->setFile(NULL);
			d->setCrcCalc(NULL);

			// Check if we're anti-fragging...
			if(d->isSet(Download::FLAG_ANTI_FRAG)) {
				// Ok, rename the file to what we expect it to be...
				try {
					const string& tgt = d->getTempTarget().empty() ? d->getPath() : d->getTempTarget();
					File::renameFile(d->getDownloadTarget(), tgt);
					d->unsetFlag(Download::FLAG_ANTI_FRAG);
				} catch(const FileException& e) {
					dcdebug("AntiFrag: %s\n", e.getError().c_str());
					// Now what?
				}
			}
		} catch(const FileException& e) {
			failDownload(aSource, e.getError());
			return;
		}

		dcassert(d->getPos() == d->getSize());
		dcdebug("Download finished: %s, size " I64_FMT ", downloaded " I64_FMT "\n", d->getPath().c_str(), d->getSize(), d->getTotal());

		// Check if we have some crc:s...
		if(BOOLSETTING(SFV_CHECK)) {
			if(!checkSfv(aSource, d, crc))
				return;
		}

		if(BOOLSETTING(LOG_DOWNLOADS) && (BOOLSETTING(LOG_FILELIST_TRANSFERS) || d->getType() == Transfer::TYPE_FILE)) {
			logDownload(aSource, d);
		}

		// Check if we need to move the file
		if( !d->getTempTarget().empty() && (Util::stricmp(d->getPath().c_str(), d->getTempTarget().c_str()) != 0) ) {
			moveFile(d->getTempTarget(), d->getPath());
		}
	}

	removeDownload(d);
	fire(DownloadManagerListener::Complete(), d);

	QueueManager::getInstance()->putDownload(d, true);
	checkDownloads(aSource);
}

uint32_t DownloadManager::calcCrc32(const string& file) throw(FileException) {
	File ff(file, File::READ, File::OPEN);
	CalcInputStream<CRC32Filter, false> f(&ff);

	const size_t BUF_SIZE = 1024*1024;
	AutoArray<uint8_t> b(BUF_SIZE);
	size_t n = BUF_SIZE;
	while(f.read((uint8_t*)b, n) > 0)
		;		// Keep on looping...

	return f.getFilter().getValue();
}

bool DownloadManager::checkSfv(UserConnection* aSource, Download* d, uint32_t crc) {
	SFVReader sfv(d->getPath());
	if(sfv.hasCRC()) {
		bool crcMatch = (crc == sfv.getCRC());
		if(!crcMatch && crc == 0) {
			// Blah. We have to reread the file...
			try {
				crcMatch = (calcCrc32(d->getDownloadTarget()) == sfv.getCRC());
			} catch(const FileException& ) {
				// Couldn't read the file to get the CRC(!!!)
				crcMatch = false;
			}
		}

		if(!crcMatch) {
			File::deleteFile(d->getDownloadTarget());
			dcdebug("DownloadManager: CRC32 mismatch for %s\n", d->getPath().c_str());
			LogManager::getInstance()->message(STRING(SFV_INCONSISTENCY) + " (" + STRING(FILE) + ": " + d->getPath() + ")");
			removeDownload(d);
			fire(DownloadManagerListener::Failed(), d, STRING(SFV_INCONSISTENCY));

			QueueManager::getInstance()->removeSource(d->getPath(), aSource->getUser(), QueueItem::Source::FLAG_CRC_WARN, false);
			QueueManager::getInstance()->putDownload(d, false);

			checkDownloads(aSource);
			return false;
		}

		d->setFlag(Download::FLAG_CRC32_OK);

		dcdebug("DownloadManager: CRC32 match for %s\n", d->getPath().c_str());
	}
	return true;
}

int64_t DownloadManager::getRunningAverage() {
	Lock l(cs);
	int64_t avg = 0;
	for(DownloadList::iterator i = downloads.begin(); i != downloads.end(); ++i) {
		Download* d = *i;
		avg += d->getAverageSpeed();
	}
	return avg;
}

void DownloadManager::logDownload(UserConnection* aSource, Download* d) {
	StringMap params;
	d->getParams(*aSource, params);
	LOG(LogManager::DOWNLOAD, params);
}

void DownloadManager::moveFile(const string& source, const string& target) {
	try {
		File::ensureDirectory(target);
		if(File::getSize(source) > MOVER_LIMIT) {
			mover.moveFile(source, target);
		} else {
			File::renameFile(source, target);
		}
	} catch(const FileException&) {
		try {
			if(!SETTING(DOWNLOAD_DIRECTORY).empty()) {
				File::renameFile(source, SETTING(DOWNLOAD_DIRECTORY) + Util::getFileName(target));
			} else {
				File::renameFile(source, Util::getFilePath(source) + Util::getFileName(target));
			}
		} catch(const FileException&) {
			try {
				File::renameFile(source, Util::getFilePath(source) + Util::getFileName(target));
			} catch(const FileException&) {
				// Ignore...
			}
		}
	}

}

void DownloadManager::on(UserConnectionListener::MaxedOut, UserConnection* aSource) throw() {
	noSlots(aSource);
}
void DownloadManager::noSlots(UserConnection* aSource) {
	if(aSource->getState() != UserConnection::STATE_SND && aSource->getState() != UserConnection::STATE_TREE) {
		dcdebug("DM::onMaxedOut Bad state, ignoring\n");
		return;
	}

	failDownload(aSource, STRING(NO_SLOTS_AVAILABLE));
}

void DownloadManager::on(UserConnectionListener::Error, UserConnection* aSource, const string& aError) throw() {
	failDownload(aSource, aError);
}

void DownloadManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& aError) throw() {
	failDownload(aSource, aError);
}

void DownloadManager::failDownload(UserConnection* aSource, const string& reason) {
	Download* d = aSource->getDownload();

	if(d) {
		removeDownload(d);
		fire(DownloadManagerListener::Failed(), d, reason);

		QueueManager::getInstance()->putDownload(d, false);
	}
	removeConnection(aSource);
}

void DownloadManager::removeDownload(Download* d) {
	if(d->getFile()) {
		if(d->getActual() > 0) {
			try {
				d->getFile()->flush();
			} catch(const Exception&) {
			}
		}
		delete d->getFile();
		d->setFile(NULL);
		d->setCrcCalc(NULL);

		if(d->isSet(Download::FLAG_ANTI_FRAG)) {
			d->unsetFlag(Download::FLAG_ANTI_FRAG);
		}
	}

	{
		Lock l(cs);
		dcassert(find(downloads.begin(), downloads.end(), d) != downloads.end());

		downloads.erase(remove(downloads.begin(), downloads.end(), d), downloads.end());
	}
}

void DownloadManager::on(UserConnectionListener::FileNotAvailable, UserConnection* aSource) throw() {
	fileNotAvailable(aSource);
}

/** @todo Handle errors better */
void DownloadManager::on(AdcCommand::STA, UserConnection* aSource, const AdcCommand& cmd) throw() {
	if(cmd.getParameters().size() < 2) {
		aSource->disconnect();
		return;
	}

	const string& err = cmd.getParameters()[0];
	if(err.length() < 3) {
		aSource->disconnect();
		return;
	}

	switch(Util::toInt(err.substr(0, 1))) {
	case AdcCommand::SEV_FATAL:
		aSource->disconnect();
		return;
	case AdcCommand::SEV_RECOVERABLE:
		switch(Util::toInt(err.substr(1))) {
		case AdcCommand::ERROR_FILE_NOT_AVAILABLE:
			fileNotAvailable(aSource);
			return;
		case AdcCommand::ERROR_SLOTS_FULL:
			noSlots(aSource);
			return;
		}
	}
	aSource->disconnect();
}

void DownloadManager::fileNotAvailable(UserConnection* aSource) {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);
	dcdebug("File Not Available: %s\n", d->getPath().c_str());

	removeDownload(d);
	fire(DownloadManagerListener::Failed(), d, d->getTargetFileName() + ": " + STRING(FILE_NOT_AVAILABLE));

	QueueManager::getInstance()->removeSource(d->getPath(), aSource->getUser(), d->getType() == Transfer::TYPE_TREE ? QueueItem::Source::FLAG_NO_TREE : QueueItem::Source::FLAG_FILE_NOT_AVAILABLE, false);

	QueueManager::getInstance()->putDownload(d, false);
	checkDownloads(aSource);
}

} // namespace dcpp
