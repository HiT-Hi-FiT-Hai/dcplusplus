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

#include "DownloadManager.h"
#include "ConnectionManager.h"
#include "User.h"
#include "QueueManager.h"
#include "LogManager.h"
#include "ResourceManager.h"

DownloadManager* DownloadManager::instance = NULL;

static string DOWNLOAD_AREA = "Downloads";

void DownloadManager::onTimerSecond(DWORD /*aTick*/) {
	Lock l(cs);

	Download::List tickList;
	// Tick each ongoing download
	for(Download::Iter i = downloads.begin(); i != downloads.end(); ++i) {
		if((*i)->getPos() > 0) {
			tickList.push_back(*i);
		}
	}

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
	
	Download* d = QueueManager::getInstance()->getDownload(aConn->getUser(), ConnectionManager::getInstance()->getQueueItem(aConn));
	
	if(d) {
		dcassert(aConn->getDownload() == NULL);
		aConn->setDownload(d);
		aConn->setState(UserConnection::STATE_FILELENGTH);
		{
			Lock l(cs);
			downloads.push_back(d);
		}

		if(d->isSet(Download::RESUME)) {
			LONGLONG x = File::getSize(d->getTarget());

			dcassert( (d->getSize() != -1));
			if( ( (SETTING(ROLLBACK)*2) > x) || (d->getSize() < (SETTING(ROLLBACK)*2) ) ) {
				d->setPos(0);
			} else {
				d->setPos(x - (SETTING(ROLLBACK)*2) );
				d->setRollbackBuffer(SETTING(ROLLBACK));
				d->setFlag(Download::ROLLBACK);
			}
		} else {
			d->setPos(0);
		}
		
		
		aConn->get(d->getSource(), d->getPos());
		return;
	}

	// Connection not needed any more, return it to the ConnectionManager...
	removeConnection(aConn, true);
}

bool DownloadManager::checkRollback(Download* d, const BYTE* aData, int aLen) throw(FileException) {
	
	dcassert(d->getRollbackBuffer());

	if(d->getTotal() + aLen >= d->getRollbackSize()) {
		BYTE* buf = new BYTE[d->getRollbackSize()];
		int len = d->getRollbackSize() - (int)d->getTotal();
		dcassert(len > 0);
		dcassert(len <= d->getRollbackSize());
		memcpy(d->getRollbackBuffer() + d->getTotal(), aData, len);
		
		d->getFile()->read(buf, d->getRollbackSize());
		
		int cmp = memcmp(d->getRollbackBuffer(), buf, d->getRollbackSize());

		delete buf;
		d->unsetFlag(Download::ROLLBACK);
		d->setRollbackBuffer(0);

		if(cmp != 0) {
			return false;
		}
		
		// Write the rest...the file pointer should have been moved to the correct position by now...
		d->getFile()->write(aData+len, aLen - len);
		
	} else {
		memcpy(d->getRollbackBuffer() + d->getTotal(), aData, aLen);
	}

	return true;
}

void DownloadManager::onFileLength(UserConnection* aSource, const string& aFileLength) {
	if(aSource->getState() != UserConnection::STATE_FILELENGTH) {
		dcdebug("DM::onFileLength Bad state, ignoring\n");
		return;
	}

	LONGLONG fileLength = Util::toInt64(aFileLength);
	
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	Util::ensureDirectory(d->getTarget());
	
	File* file;
	try {
		file = new BufferedFile(d->getTarget(), File::WRITE | File::READ, File::OPEN | File::CREATE | (d->isSet(Download::RESUME) ? 0 : File::TRUNCATE));
	} catch(FileException e) {
		fire(DownloadManagerListener::FAILED, d, "Could not open target file:" + e.getError());
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
		d->setStart(TimerManager::getTick());
		aSource->setState(UserConnection::STATE_DONE);
		
		fire(DownloadManagerListener::STARTING, d);
		
		aSource->setDataMode(d->getSize() - d->getPos());
		aSource->startSend();
	}
}

void DownloadManager::onData(UserConnection* aSource, const BYTE* aData, int aLen) {
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
	d->setFile(NULL);
	
	dcdebug("Download finished: %s, size %I64d\n", d->getTarget().c_str(), d->getSize());

	if(BOOLSETTING(LOG_DOWNLOADS)) {
		LOGDT(DOWNLOAD_AREA, d->getTarget() + STRING(DOWNLOADED_FROM) + aSource->getUser()->getNick() + 
			", " + Util::toString(d->getSize()) + " b, : " + Util::formatBytes(d->getAverageSpeed()) + 
			"/s, " + Util::formatSeconds(d->getSecondsLeft()));
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
	
	d->setFile(NULL);
	fire(DownloadManagerListener::FAILED, d, aError);

	string target = d->getTarget();
	aSource->setDownload(NULL);
	removeDownload(d);
	
	if(aError == "File Not Available") {
		if(SETTING(REMOVE_NOT_AVAILABLE))
			QueueManager::getInstance()->removeSource(target, aSource->getUser(), false);
	}

	removeConnection(aSource);
}

void DownloadManager::removeDownload(Download* d, bool finished /* = false */) {
	{
		Lock l(cs);
		dcassert(find(downloads.begin(), downloads.end(), d) != downloads.end());
		downloads.erase(find(downloads.begin(), downloads.end(), d));
	}
	QueueManager::getInstance()->putDownload(d, finished);
	
}

void DownloadManager::abortDownload(const string& aTarget) {
	Lock l(cs);
	for(Download::Iter i = downloads.begin(); i != downloads.end(); ++i) {
		Download* d = *i;
		if(d->getTarget() == aTarget) {
			d->getCQI()->getConnection()->disconnect();
			break;
		}
	}
}

/**
 * @file DownloadManger.cpp
 * $Id: DownloadManager.cpp,v 1.53 2002/04/03 23:20:35 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.cpp,v $
 * Revision 1.53  2002/04/03 23:20:35  arnetheduck
 * ...
 *
 * Revision 1.52  2002/03/25 22:23:24  arnetheduck
 * Lots of minor updates
 *
 * Revision 1.51  2002/03/11 22:58:54  arnetheduck
 * A step towards internationalization
 *
 * Revision 1.50  2002/03/04 23:52:30  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.49  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.48  2002/02/26 23:25:22  arnetheduck
 * Minor updates and fixes
 *
 * Revision 1.47  2002/02/25 15:39:28  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.46  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.45  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.44  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.43  2002/02/07 22:12:22  arnetheduck
 * Last fixes before 0.152
 *
 * Revision 1.42  2002/02/04 01:10:29  arnetheduck
 * Release 0.151...a lot of things fixed
 *
 * Revision 1.41  2002/02/01 02:00:27  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.40  2002/01/26 14:59:22  arnetheduck
 * Fixed disconnect crash
 *
 * Revision 1.39  2002/01/26 12:06:39  arnetheduck
 * Småsaker
 *
 * Revision 1.38  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
 * Revision 1.37  2002/01/22 00:10:37  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
 *
 * Revision 1.36  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.35  2002/01/19 19:07:39  arnetheduck
 * Last fixes before 0.13
 *
 * Revision 1.34  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.33  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.32  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.31  2002/01/15 00:41:54  arnetheduck
 * late night fixes...
 *
 * Revision 1.30  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.29  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.28  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.27  2002/01/08 00:24:10  arnetheduck
 * Last bugs fixed before 0.11
 *
 * Revision 1.26  2002/01/07 23:05:48  arnetheduck
 * Resume rollback implemented
 *
 * Revision 1.25  2002/01/06 00:14:54  arnetheduck
 * Incoming searches almost done, just need some testing...
 *
 * Revision 1.24  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.22  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.21  2002/01/02 16:55:56  arnetheduck
 * Time for 0.09
 *
 * Revision 1.20  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.19  2001/12/30 17:41:16  arnetheduck
 * Fixed some XML parsing bugs
 *
 * Revision 1.18  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.17  2001/12/21 23:52:30  arnetheduck
 * Last commit for five days
 *
 * Revision 1.16  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.15  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.14  2001/12/18 12:32:18  arnetheduck
 * Stability fixes
 *
 * Revision 1.13  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.12  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.11  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.10  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.9  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.8  2001/12/07 20:03:06  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.7  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
 * Revision 1.6  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.3  2001/11/29 19:10:54  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */
