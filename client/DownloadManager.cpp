/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#include "QueueManager.h"
#include "CryptoManager.h"
#include "HashManager.h"

#include "LogManager.h"
#include "SFVReader.h"
#include "User.h"
#include "File.h"
#include "FilteredFile.h"

#include <limits>

static const string DOWNLOAD_AREA = "Downloads";
const string Download::ANTI_FRAG_EXT = ".antifrag";

Download::Download() throw() : file(NULL),
crcCalc(NULL), treeValid(false), oldDownload(false), tth(NULL) { 
}

Download::Download(QueueItem* qi) throw() : source(qi->getCurrent()->getPath()),
	target(qi->getTarget()), tempTarget(qi->getTempTarget()), file(NULL),
	crcCalc(NULL), treeValid(false), oldDownload(false), tth(qi->getTTH()) { 
	
	setSize(qi->getSize());
	if(qi->isSet(QueueItem::FLAG_USER_LIST))
		setFlag(Download::FLAG_USER_LIST);
	if(qi->isSet(QueueItem::FLAG_RESUME))
		setFlag(Download::FLAG_RESUME);
	if(qi->getCurrent()->isSet(QueueItem::Source::FLAG_UTF8))
		setFlag(Download::FLAG_UTF8);
};

void DownloadManager::on(TimerManagerListener::Second, u_int32_t /*aTick*/) throw() {
	Lock l(cs);

	Download::List tickList;
	// Tick each ongoing download
	for(Download::Iter i = downloads.begin(); i != downloads.end(); ++i) {
		if((*i)->getTotal() > 0) {
			tickList.push_back(*i);
		}
	}

	if(tickList.size() > 0)
		fire(DownloadManagerListener::Tick(), tickList);
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

class TreeOutputStream : public OutputStream {
public:
	TreeOutputStream(TigerTree& aTree) : tree(aTree), bufPos(0) {
	}

	virtual size_t write(const void* xbuf, size_t len) throw(Exception) {
		size_t pos = 0;
		u_int8_t* b = (u_int8_t*)xbuf;
		while(pos < len) {
			size_t left = len - pos;
			if(bufPos == 0 && left >= TigerTree::HASH_SIZE) {
				tree.getLeaves().push_back(TTHValue(b + pos));
				pos += TigerTree::HASH_SIZE;
			} else {
				size_t bytes = min(TigerTree::HASH_SIZE - bufPos, left);
				memcpy(buf + bufPos, b + pos, bytes);
				bufPos += bytes;
				pos += bytes;
				if(bufPos == TigerTree::HASH_SIZE) {
					tree.getLeaves().push_back(TTHValue(buf));
					bufPos = 0;
				}
			}
		}
		return len;
	}

	virtual size_t flush() throw(Exception) {
		return 0;
	}
private:
	TigerTree& tree;
	u_int8_t buf[TigerTree::HASH_SIZE];
	size_t bufPos;
};

void DownloadManager::checkDownloads(UserConnection* aConn) {

	Download* d = aConn->getDownload();

	bool firstTry = false;
	if(d == NULL) {
		firstTry = true;

		bool slotsFull = (SETTING(DOWNLOAD_SLOTS) != 0) && (getDownloads() >= (size_t)SETTING(DOWNLOAD_SLOTS));
		bool speedFull = (SETTING(MAX_DOWNLOAD_SPEED) != 0) && (getAverageSpeed() >= (SETTING(MAX_DOWNLOAD_SPEED)*1024));

		if( slotsFull || speedFull ) {
			bool extraFull = (SETTING(DOWNLOAD_SLOTS) != 0) && (getDownloads() >= (size_t)(SETTING(DOWNLOAD_SLOTS)+3));
			if(extraFull || !QueueManager::getInstance()->hasDownload(aConn->getUser(), QueueItem::HIGHEST)) {
				removeConnection(aConn);
				return;
			}
		}

		d = QueueManager::getInstance()->getDownload(aConn->getUser());

		if(d == NULL) {
			removeConnection(aConn, true);
			return;
		}

		{
			Lock l(cs);
			downloads.push_back(d);
		}

		d->setUserConnection(aConn);
		aConn->setDownload(d);
	}

	if(firstTry && !d->getTreeValid() && 
		!d->isSet(Download::FLAG_USER_LIST) && d->getTTH() != NULL)
	{
		if(HashManager::getInstance()->getTree(d->getTarget(), d->getTTH(), d->getTigerTree())) {
			d->setTreeValid(true);
		} else if(!d->isSet(Download::FLAG_TREE_TRIED) && 
			aConn->isSet(UserConnection::FLAG_SUPPORTS_TTHL)) 
		{
			// So, we need to download the tree...
			Download* tthd = new Download();
			tthd->setOldDownload(d);
			tthd->setFlag(Download::FLAG_TREE_DOWNLOAD);
			tthd->setTarget(d->getTarget());
			tthd->setSource(d->getSource());

			tthd->setUserConnection(aConn);
			aConn->setDownload(tthd);

			aConn->setState(UserConnection::STATE_TREE);
			// Hack to get by TTH if possible
			tthd->setTTH(d->getTTH());
			aConn->send(tthd->getCommand(false, aConn->isSet(UserConnection::FLAG_SUPPORTS_TTHF)));
			tthd->setTTH(NULL);
			return;
		}
	}

	aConn->setState(UserConnection::STATE_FILELENGTH);
	
	if(d->isSet(Download::FLAG_RESUME)) {
		dcassert(d->getSize() != -1);

		const string& target = (d->getTempTarget().empty() ? d->getTarget() : d->getTempTarget());
		int64_t start = File::getSize(target);

		// Only use antifrag if we don't have a previous non-antifrag part
		if( BOOLSETTING(ANTI_FRAG) && (start == -1) && (d->getSize() != -1) ) {
			int64_t aSize = File::getSize(target + Download::ANTI_FRAG_EXT);

			if(aSize == d->getSize())
				start = d->getPos();
			else
				start = 0;

			d->setFlag(Download::FLAG_ANTI_FRAG);
		}
		
		int rollback = SETTING(ROLLBACK);
		if(rollback > start) {
			d->setStartPos(0);
		} else {
			d->setStartPos(start - rollback);
			d->setFlag(Download::FLAG_ROLLBACK);
		}
	} else {
		d->setStartPos(0);
	}

	if(d->isSet(Download::FLAG_USER_LIST)) {
		if(aConn->isSet(UserConnection::FLAG_SUPPORTS_XML_BZLIST)) {
			d->setSource("files.xml.bz2");
		}
	}

	if(aConn->isSet(UserConnection::FLAG_SUPPORTS_ADCGET) && d->isSet(Download::FLAG_UTF8)) {
		aConn->send(d->getCommand(
			aConn->isSet(UserConnection::FLAG_SUPPORTS_ZLIB_GET),
			aConn->isSet(UserConnection::FLAG_SUPPORTS_TTHF)
			));
	} else {
		if(BOOLSETTING(COMPRESS_TRANSFERS) && aConn->isSet(UserConnection::FLAG_SUPPORTS_GETZBLOCK) && d->getSize() != -1 ) {
			// This one, we'll download with a zblock download instead...
			d->setFlag(Download::FLAG_ZDOWNLOAD);
			aConn->getZBlock(d->getSource(), d->getPos(), d->getBytesLeft(), d->isSet(Download::FLAG_UTF8));
		} else if(d->isSet(Download::FLAG_UTF8)) {
			aConn->getBlock(d->getSource(), d->getPos(), d->getBytesLeft(), true);
		} else {
			aConn->get(d->getSource(), d->getPos());
		}
	}
}

void DownloadManager::on(UserConnectionListener::Sending, UserConnection* aSource, int64_t aBytes) throw() {
	if(aSource->getState() != UserConnection::STATE_FILELENGTH) {
		dcdebug("DM::onFileLength Bad state, ignoring\n");
		return;
	}

	if(prepareFile(aSource, (aBytes == -1) ? -1 : aSource->getDownload()->getPos() + aBytes)) {
		aSource->setDataMode();
	}
}

void DownloadManager::on(UserConnectionListener::FileLength, UserConnection* aSource, int64_t aFileLength) throw() {

	if(aSource->getState() != UserConnection::STATE_FILELENGTH) {
		dcdebug("DM::onFileLength Bad state, ignoring\n");
		return;
	}

	if(prepareFile(aSource, aFileLength)) {
		aSource->setDataMode();
		aSource->startSend();
	}
}

void DownloadManager::on(Command::SND, UserConnection* aSource, const Command& cmd) throw() {
	int64_t bytes = Util::toInt64(cmd.getParam(3));

	if(cmd.getParam(0) == "tthl") {
		if(aSource->getState() != UserConnection::STATE_TREE) {
			dcdebug("DM::SND Bad state, ignoring\n");
			return;
		}
		Download* d = aSource->getDownload();
		d->setFile(new TreeOutputStream(d->getOldDownload()->getTigerTree()));
		d->setSize(bytes);
		d->setPos(0);
		dcassert(d->isSet(Download::FLAG_TREE_DOWNLOAD));
		aSource->setState(UserConnection::STATE_DONE);

		if(cmd.hasFlag("ZL", 4)) {
			d->setFile(new FilteredOutputStream<UnZFilter, true>(d->getFile()));
		}

		aSource->setDataMode();
	} else if(cmd.getParam(0) == "file") {
		if(aSource->getState() != UserConnection::STATE_FILELENGTH) {
			dcdebug("DM::onFileLength Bad state, ignoring\n");
			return;
		}

		if(prepareFile(aSource, (bytes == -1) ? -1 : aSource->getDownload()->getPos() + bytes)) {
			aSource->setDataMode();
		}
	}
}

class RollbackException : public FileException {
public:
	RollbackException (const string& aError) : FileException(aError) { };
	virtual ~RollbackException() { };
};

template<bool managed>
class RollbackOutputStream : public OutputStream {
public:
	RollbackOutputStream(File* f, OutputStream* aStream, size_t bytes) : s(aStream), pos(0), bufSize(bytes), buf(new u_int8_t[bytes]) {
		size_t n = bytes;
		f->read(buf, n);
		f->movePos(-((int64_t)bytes));
	}
	virtual ~RollbackOutputStream() { if(managed) delete s; };

	virtual size_t flush() throw(FileException) {
		return s->flush();
	}

	virtual size_t write(const void* b, size_t len) throw(FileException) {
		u_int8_t* wb = (u_int8_t*)b;
		if(buf != NULL) {
			size_t n = len < (bufSize - pos) ? len : bufSize - pos;

			if(memcmp(buf + pos, wb, n) != 0) {
				throw RollbackException(STRING(ROLLBACK_INCONSISTENCY));
			}
			pos += n;
			if(pos == bufSize) {
				delete buf;
				buf = NULL;
			}
		}
		return s->write(wb, len);
	}

private:
	OutputStream* s;
	size_t pos;
	size_t bufSize;
	u_int8_t* buf;
};

template<bool managed>
class TigerCheckOutputStream : public OutputStream {
public:
	TigerCheckOutputStream(const TigerTree& aTree, OutputStream* aStream) : s(aStream), real(aTree), cur(aTree.getBlockSize()), verified(0), bufPos(0) {
	}
	virtual ~TigerCheckOutputStream() { if(managed) delete s; };

	virtual size_t flush() throw(FileException) {
		if (bufPos != 0)
			cur.update(buf, bufPos);
		bufPos = 0;

		cur.finalize();
		checkTrees();
		return s->flush();
	}

	virtual size_t write(const void* b, size_t len) throw(FileException) {
		u_int8_t* xb = (u_int8_t*)b;
		size_t pos = 0;

		if(bufPos != 0) {
			size_t bytes = min(TigerTree::BASE_BLOCK_SIZE - bufPos, len);
			memcpy(buf + bufPos, xb, bytes);
			pos = bytes;
			bufPos += bytes;

			if(bufPos == TigerTree::BASE_BLOCK_SIZE) {
				cur.update(buf, TigerTree::BASE_BLOCK_SIZE);
				bufPos = 0;
			}
		}

		if(pos < len) {
			dcassert(bufPos == 0);
			size_t left = len - pos;
			size_t part = left - (left %  TigerTree::BASE_BLOCK_SIZE);
			if(part > 0) {
				cur.update(xb + pos, part);
				pos += part;
			}
			left = len - pos;
			memcpy(buf, xb + pos, left);
			bufPos = left;
		}

		checkTrees();
		return s->write(b, len);
	}
	
	virtual int64_t verifiedBytes() {
		return min(real.getFileSize(), (int64_t)(cur.getBlockSize() * cur.getLeaves().size()));
	}
private:
	OutputStream* s;
	const TigerTree& real;
	TigerTree cur;
	size_t verified;

	u_int8_t buf[TigerTree::BASE_BLOCK_SIZE];
	size_t bufPos;

	void checkTrees() throw(FileException) {
		while(cur.getLeaves().size() > verified) {
			if(cur.getLeaves().size() > real.getLeaves().size() ||
				!(cur.getLeaves()[verified] == real.getLeaves()[verified])) 
			{
				throw FileException(STRING(TTH_INCONSISTENCY));
			}
			verified++;
		}
	}
};

bool DownloadManager::prepareFile(UserConnection* aSource, int64_t newSize /* = -1 */) {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	if(newSize != -1) {
		d->setSize(newSize);
	}
	if(d->getPos() >= d->getSize()) {
		// Already finished?
		aSource->setDownload(NULL);
		removeDownload(d, true, true);
		removeConnection(aSource);
		return false;
	}

	dcassert(d->getSize() != -1);

	string target = d->getDownloadTarget();
	File::ensureDirectory(target);
	if(d->isSet(Download::FLAG_USER_LIST)) {
		if(aSource->isSet(UserConnection::FLAG_SUPPORTS_XML_BZLIST)) {
			target += ".xml.bz2";
		} else {
			target += ".DcLst";
		}
	}

	File* file = NULL;
	try {
		// Let's check if we can find this file in a any .SFV...
		int trunc = d->isSet(Download::FLAG_RESUME) ? 0 : File::TRUNCATE;
		file = new File(target, File::RW, File::OPEN | File::CREATE | trunc);
		if(d->isSet(Download::FLAG_ANTI_FRAG)) {
			file->setSize(d->getSize());
		}
		file->setPos(d->getPos());
	} catch(const FileException& e) {
		delete file;
		fire(DownloadManagerListener::Failed(), d, STRING(COULD_NOT_OPEN_TARGET_FILE) + e.getError());
		aSource->setDownload(NULL);
		removeDownload(d, true);
		removeConnection(aSource);
		return false;
	} catch(const Exception& e) {
		delete file;
		fire(DownloadManagerListener::Failed(), d, e.getError());
		aSource->setDownload(NULL);
		removeDownload(d, true);
		removeConnection(aSource);
		return false;
	}

	d->setFile(file);

	if(d->isSet(Download::FLAG_ROLLBACK)) {
		d->setFile(new RollbackOutputStream<true>(file, d->getFile(), (size_t)min((int64_t)SETTING(ROLLBACK), d->getSize() - d->getPos())));
	}

	if(SETTING(BUFFER_SIZE) != 0) {
		d->setFile(new BufferedOutputStream<true>(d->getFile()));
	}

	bool sfvcheck = BOOLSETTING(SFV_CHECK) && (d->getPos() == 0) && (SFVReader(d->getTarget()).hasCRC());

	if(sfvcheck) {
		d->setFlag(Download::FLAG_CALC_CRC32);
		Download::CrcOS* crc = new Download::CrcOS(d->getFile());
		d->setCrcCalc(crc);
		d->setFile(crc);
	}

	if(d->getPos() == 0) {
		if(!d->getTreeValid() && d->getTTH() != NULL && d->getSize() < numeric_limits<size_t>::max()) {
			// We make a single node tree...
			d->getTigerTree().setFileSize(d->getSize());
			d->getTigerTree().setBlockSize((size_t)d->getSize());

			d->getTigerTree().getLeaves().push_back(*d->getTTH());
			d->getTigerTree().calcRoot();
			d->setTreeValid(true);
		}
		if(d->getTreeValid()) {
			d->setFile(new TigerCheckOutputStream<true>(d->getTigerTree(), d->getFile()));
		}
	}

	if(d->isSet(Download::FLAG_ZDOWNLOAD)) {
		d->setFile(new FilteredOutputStream<UnZFilter, true>(d->getFile()));
	}
	dcassert(d->getPos() != -1);
	d->setStart(GET_TICK());
	aSource->setState(UserConnection::STATE_DONE);
	
	fire(DownloadManagerListener::Starting(), d);
	
	return true;
}	

void DownloadManager::on(UserConnectionListener::Data, UserConnection* aSource, const u_int8_t* aData, size_t aLen) throw() {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	try {
		d->addPos(d->getFile()->write(aData, aLen), aLen);

		if(d->getPos() > d->getSize()) {
			throw Exception(STRING(TOO_MUCH_DATA));
		} else if(d->getPos() == d->getSize()) {
			handleEndData(aSource);
			aSource->setLineMode();
		}
	} catch(const RollbackException& e) {
		string target = d->getTarget();
		QueueManager::getInstance()->removeSource(target, aSource->getUser(), QueueItem::Source::FLAG_ROLLBACK_INCONSISTENCY);
		fire(DownloadManagerListener::Failed(), d, e.getError());

		d->resetPos();
		aSource->setDownload(NULL);
		removeDownload(d, true);
		removeConnection(aSource);
		return;
	} catch(const FileException& e) {
		fire(DownloadManagerListener::Failed(), d, e.getError());

		d->resetPos();
		aSource->setDownload(NULL);
		removeDownload(d, true);
		removeConnection(aSource);
		return;
	} catch(const Exception& e) {
		fire(DownloadManagerListener::Failed(), d, e.getError());
		// Nuke the bytes we have written, this is probably a compression error
		d->resetPos();
		aSource->setDownload(NULL);
		removeDownload(d, true);
		removeConnection(aSource);
		return;
	}
}

/** Download finished! */
void DownloadManager::handleEndData(UserConnection* aSource) {

	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	if(d->isSet(Download::FLAG_TREE_DOWNLOAD)) {
		d->getFile()->flush();
		delete d->getFile();
		d->setFile(NULL);

		Download* old = d->getOldDownload();

		size_t bl = 1024;
		while(bl * old->getTigerTree().getLeaves().size() < old->getSize())
			bl *= 2;
		old->getTigerTree().setBlockSize(bl);
		dcassert(old->getSize() != -1);
		old->getTigerTree().setFileSize(old->getSize());

		old->getTigerTree().calcRoot();

		if(!(*old->getTTH() == old->getTigerTree().getRoot())) {
			// This tree is for a different file, remove from queue...
			fire(DownloadManagerListener::Failed(), old, STRING(INVALID_TREE));

			string target = old->getTarget();

			aSource->setDownload(NULL);
			removeDownload(old, true);

			QueueManager::getInstance()->removeSource(target, aSource->getUser(), QueueItem::Source::FLAG_BAD_TREE, false);
			checkDownloads(aSource);
			return;
		}

		d->getOldDownload()->setTreeValid(true);

		HashManager::getInstance()->addTree(old->getTarget(), old->getTigerTree());

		aSource->setDownload(d->getOldDownload());

		delete d;

		// Ok, now we can continue to the actual file...
		checkDownloads(aSource);
		return;
	}

	u_int32_t crc = 0;
	bool hasCrc = (d->getCrcCalc() != NULL);

	// First, finish writing the file (flushing the buffers and closing the file...)
	try {
		d->getFile()->flush();
		if(hasCrc)
			crc = d->getCrcCalc()->getFilter().getValue();
		delete d->getFile();
		d->setFile(NULL);
		d->setCrcCalc(NULL);

		// Check if we're anti-fragging...
		if(d->isSet(Download::FLAG_ANTI_FRAG)) {
			// Ok, rename the file to what we expect it to be...
			try {
				const string& tgt = d->getTempTarget().empty() ? d->getTarget() : d->getTempTarget();
				File::renameFile(d->getDownloadTarget(), tgt);
				d->unsetFlag(Download::FLAG_ANTI_FRAG);
			} catch(const FileException& e) {
				dcdebug("AntiFrag: %s\n", e.getError().c_str());
				// Now what?
			}
		}
	} catch(const FileException& e) {
		fire(DownloadManagerListener::Failed(), d, e.getError());
		
		aSource->setDownload(NULL);
		removeDownload(d, true);
		removeConnection(aSource);
		return;
	}
	
	dcassert(d->getPos() == d->getSize());
	dcdebug("Download finished: %s, size " I64_FMT ", downloaded " I64_FMT "\n", d->getTarget().c_str(), d->getSize(), d->getTotal());

	// Check if we have some crc:s...
	if(BOOLSETTING(SFV_CHECK)) {
		SFVReader sfv(d->getTarget());
		if(sfv.hasCRC()) {
			bool crcMatch;
			string tgt = d->getDownloadTarget();
			if(hasCrc) {
				crcMatch = (crc == sfv.getCRC());
			} else {
				// More complicated, we have to reread the file
				try {
					
					File ff(tgt, File::READ, File::OPEN);
					CalcInputStream<CRC32Filter, false> f(&ff);

					const size_t BUF_SIZE = 16 * 65536;
					AutoArray<u_int8_t> b(BUF_SIZE);
					size_t n = BUF_SIZE;
					while(f.read((u_int8_t*)b, n) > 0)
						;		// Keep on looping...

					crcMatch = (f.getFilter().getValue() == sfv.getCRC());
				} catch (FileException&) {
					// Nope; read failed...
					goto noCRC;
				}
			}

			if(!crcMatch) {
				File::deleteFile(tgt);
				dcdebug("DownloadManager: CRC32 mismatch for %s\n", d->getTarget().c_str());
				LogManager::getInstance()->message(STRING(SFV_INCONSISTENCY) + " (" + STRING(FILE) + ": " + d->getTarget() + ")");
				fire(DownloadManagerListener::Failed(), d, STRING(SFV_INCONSISTENCY));
				
				string target = d->getTarget();
				
				aSource->setDownload(NULL);
				removeDownload(d, true);				
				
				QueueManager::getInstance()->removeSource(target, aSource->getUser(), QueueItem::Source::FLAG_CRC_WARN, false);
				checkDownloads(aSource);
				return;
			} 

			d->setFlag(Download::FLAG_CRC32_OK);
			
			dcdebug("DownloadManager: CRC32 match for %s\n", d->getTarget().c_str());
		}
	}
noCRC:
	if(BOOLSETTING(LOG_DOWNLOADS) && (BOOLSETTING(LOG_FILELIST_TRANSFERS) || !d->isSet(Download::FLAG_USER_LIST))) {
		StringMap params;
		params["target"] = d->getTarget();
		params["user"] = aSource->getUser()->getNick();
		params["hub"] = aSource->getUser()->getLastHubName();
		params["hubip"] = aSource->getUser()->getLastHubAddress();
		params["size"] = Util::toString(d->getSize());
		params["sizeshort"] = Util::formatBytes(d->getSize());
		params["chunksize"] = Util::toString(d->getTotal());
		params["chunksizeshort"] = Util::formatBytes(d->getTotal());
		params["actualsize"] = Util::toString(d->getActual());
		params["actualsizeshort"] = Util::formatBytes(d->getActual());
		params["speed"] = Util::formatBytes(d->getAverageSpeed()) + "/s";
		params["time"] = Util::formatSeconds((GET_TICK() - d->getStart()) / 1000);
		params["sfv"] = Util::toString(d->isSet(Download::FLAG_CRC32_OK) ? 1 : 0);
		LOG(DOWNLOAD_AREA, Util::formatParams(SETTING(LOG_FORMAT_POST_DOWNLOAD), params));
	}

	// Check if we need to move the file
	if( !d->getTempTarget().empty() && (Util::stricmp(d->getTarget().c_str(), d->getTempTarget().c_str()) != 0) ) {
		try {
			File::ensureDirectory(d->getTarget());
			if(File::getSize(d->getTempTarget()) > MOVER_LIMIT) {
				mover.moveFile(d->getTempTarget(), d->getTarget());
			} else {
				File::renameFile(d->getTempTarget(), d->getTarget());
			}
			d->setTempTarget(Util::emptyString);
		} catch(const FileException&) {
			try {
				if(!SETTING(DOWNLOAD_DIRECTORY).empty()) {
					File::renameFile(d->getTempTarget(), SETTING(DOWNLOAD_DIRECTORY) + d->getTargetFileName());
				} else {
					File::renameFile(d->getTempTarget(), Util::getFilePath(d->getTempTarget()) + d->getTargetFileName());
				}
			} catch(const FileException&) {
				try {
					File::renameFile(d->getTempTarget(), Util::getFilePath(d->getTempTarget()) + d->getTargetFileName());
				} catch(const FileException&) {
					// Ignore...
				}
			}
		}
	}

	fire(DownloadManagerListener::Complete(), d);
	
	aSource->setDownload(NULL);
	removeDownload(d, true, true);
	checkDownloads(aSource);
}

void DownloadManager::on(UserConnectionListener::MaxedOut, UserConnection* aSource) throw() { 
	if(aSource->getState() != UserConnection::STATE_FILELENGTH && aSource->getState() != UserConnection::STATE_TREE) {
		dcdebug("DM::onMaxedOut Bad state, ignoring\n");
		return;
	}

	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	fire(DownloadManagerListener::Failed(), d, STRING(NO_SLOTS_AVAILABLE));

	aSource->setDownload(NULL);
	removeDownload(d, true);
	removeConnection(aSource);
}

void DownloadManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& aError) throw() {
	Download* d = aSource->getDownload();

	if(d == NULL) {
		removeConnection(aSource);
		return;
	}
	
	fire(DownloadManagerListener::Failed(), d, aError);

	string target = d->getTarget();
	aSource->setDownload(NULL);
	removeDownload(d, true);
	
	if(aError.find("File Not Available") != string::npos || aError.find(" no more exists") != string::npos) { //solved DCTC
		QueueManager::getInstance()->removeSource(target, aSource->getUser(), QueueItem::Source::FLAG_FILE_NOT_AVAILABLE, false);
	}

	removeConnection(aSource);
}

void DownloadManager::removeDownload(Download* d, bool full, bool finished /* = false */) {
	if(d->getOldDownload() != NULL) {
		if(d->getFile()) {
			try {
				d->getFile()->flush();
			} catch(const Exception&) {
				finished = false;
			}
			delete d->getFile();
			d->setFile(NULL);
			d->setCrcCalc(NULL);

			if(d->isSet(Download::FLAG_ANTI_FRAG)) {
				// Ok, set the pos to whereever it was last writing and hope for the best...
				d->unsetFlag(Download::FLAG_ANTI_FRAG);
			} 
		}

		Download* old = d;
		d = d->getOldDownload();
		if(!full) {
			old->getUserConnection()->setDownload(d);
		}

		old->setUserConnection(NULL);
		delete old;

		if(!full) {
			return;
		}
	}

	if(d->getFile()) {
		if(d->getActual() > 0) {
			try {
				d->getFile()->flush();
			} catch(const Exception&) {
				finished = false;
			}
		}
		delete d->getFile();
		d->setFile(NULL);
		d->setCrcCalc(NULL);

		if(d->isSet(Download::FLAG_ANTI_FRAG)) {
			// Ok, set the pos to whereever it was last writing and hope for the best...
			d->unsetFlag(Download::FLAG_ANTI_FRAG);
		} 
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

void DownloadManager::on(UserConnectionListener::FileNotAvailable, UserConnection* aSource) throw() {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	if(d->isSet(Download::FLAG_TREE_DOWNLOAD)) {
		// No tree, too bad...
		aSource->setDownload(d->getOldDownload());
		delete d->getFile();
		d->setFile(NULL);
		delete d;
		checkDownloads(aSource);
		return;
	}

	dcdebug("File Not Available: %s\n", d->getTarget().c_str());

	if(d->getFile()) {
		delete d->getFile();
		d->setFile(NULL);
		d->setCrcCalc(NULL);
	}

	fire(DownloadManagerListener::Failed(), d, d->getTargetFileName() + ": " + STRING(FILE_NOT_AVAILABLE));

	aSource->setDownload(NULL);

	QueueManager::getInstance()->removeSource(d->getTarget(), aSource->getUser(), QueueItem::Source::FLAG_FILE_NOT_AVAILABLE, false);
	removeDownload(d, false, false);
	checkDownloads(aSource);
}


/**
 * @file
 * $Id: DownloadManager.cpp,v 1.120 2004/09/25 20:40:40 arnetheduck Exp $
 */
