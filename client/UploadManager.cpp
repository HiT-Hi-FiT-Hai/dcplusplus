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

#include "UploadManager.h"
#include "ConnectionManager.h"
#include "LogManager.h"

UploadManager* Singleton<UploadManager>::instance = NULL;

static const string UPLOAD_AREA = "Uploads";

void UploadManager::onGet(UserConnection* aSource, const string& aFile, int64_t aResume) {
	if(aSource->getState() != UserConnection::STATE_GET) {
		dcdebug("UM:onGet Wrong state, ignoring\n");
		return;
	}
	
	Upload* u;
	dcassert(aFile.size() > 0);
	
	bool userlist = false;
	bool smallfile = false;

	string file;
	try {
		file = ShareManager::getInstance()->translateFileName(aFile);
	} catch(ShareException e) {
		aSource->error("File Not Available");
		return;
	}
	
	if( (Util::stricmp(aFile.c_str(), "MyList.bz2") == 0) ) {
		userlist = true;
	}

	if( File::getSize(file) < (int64_t)(16 * 1024) ) {
		smallfile = true;
	}

	cs.enter();
	SlotIter ui = reservedSlots.find(aSource->getUser());

	if( (!aSource->isSet(UserConnection::FLAG_HASSLOT)) && 
		(getFreeSlots()<=0) && 
		(ui == reservedSlots.end()) ) 
	{
		dcdebug("Average speed: %s/s\n", Util::formatBytes(UploadManager::getInstance()->getAverageSpeed()).c_str());
		if( ((getlastAutoGrant() + 30*1000) > GET_TICK()) || (SETTING(MIN_UPLOAD_SPEED) == 0) || ( (SETTING(MIN_UPLOAD_SPEED)*1024) < UploadManager::getInstance()->getAverageSpeed() ) ) {
			if( !(smallfile || userlist) ||
				!(aSource->isSet(UserConnection::FLAG_HASEXTRASLOT) || (getFreeExtraSlots() > 0) || (aSource->getUser()->isSet(User::OP)) ) || 
				!(aSource->getUser()->isSet(User::DCPLUSPLUS)) 
				) 
			{

				cs.leave();
				aSource->maxedOut();
				removeConnection(aSource);
				return;
			}
		}
		setlastAutoGrant(GET_TICK());
	}

	File* f;
	try {
		f = new File(file, File::READ, File::OPEN);
	} catch(FileException e) {
		cs.leave();
		aSource->error("File Not Available");
		return;
	}
	
	u = new Upload();
	u->setUserConnection(aSource);
	u->setFile(f);
	u->setSize(f->getSize());
	u->setPos(aResume, true);
	u->setFileName(aFile);

	if(smallfile)
		u->setFlag(Upload::SMALL_FILE);
	if(userlist)
		u->setFlag(Upload::USER_LIST);

	dcassert(aSource->getUpload() == NULL);
	aSource->setUpload(u);
	uploads.push_back(u);
	if(!aSource->isSet(UserConnection::FLAG_HASSLOT)) {
		if(ui != reservedSlots.end()) {
			aSource->setFlag(UserConnection::FLAG_HASSLOT);
			running++;
			reservedSlots.erase(ui);
		} else {
			if( (getFreeSlots() == 0) && (smallfile || userlist)) {
				if(!aSource->isSet(UserConnection::FLAG_HASEXTRASLOT)) {
					extra++;
					aSource->setFlag(UserConnection::FLAG_HASEXTRASLOT);
				}
			} else {
				running++;
				aSource->setFlag(UserConnection::FLAG_HASSLOT);
			}
		}
	}
	
	if(aSource->isSet(UserConnection::FLAG_HASEXTRASLOT) && aSource->isSet(UserConnection::FLAG_HASSLOT)) {
		extra--;
		aSource->unsetFlag(UserConnection::FLAG_HASEXTRASLOT);
	}
	
	aSource->setState(UserConnection::STATE_SEND);
	cs.leave();

	aSource->fileLength(Util::toString(u->getSize()));
}

void UploadManager::onSend(UserConnection* aSource) {
	if(aSource->getState() != UserConnection::STATE_SEND) {
		dcdebug("UM::onSend Bad state, ignoring\n");
		return;
	}

	Upload* u = aSource->getUpload();
	dcassert(u != NULL);

	u->setStart(GET_TICK());
	aSource->setState(UserConnection::STATE_DONE);
	aSource->transmitFile(u->getFile(), u->getSize() - u->getPos());
	fire(UploadManagerListener::STARTING, u);
}

void UploadManager::onBytesSent(UserConnection* aSource, u_int32_t aBytes) {
	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Upload* u = aSource->getUpload();
	dcassert(u != NULL);
	u->addPos(aBytes);
}

void UploadManager::onFailed(UserConnection* aSource, const string& aError) {
	Upload* u = aSource->getUpload();

	if(u) {
		aSource->setUpload(NULL);
		fire(UploadManagerListener::FAILED, u, aError);

		dcdebug("UM::onFailed: Removing upload\n");
		removeUpload(u);
	}

	removeConnection(aSource);
}

void UploadManager::onTransmitDone(UserConnection* aSource) {
	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Upload* u = aSource->getUpload();
	dcassert(u != NULL);

	aSource->setUpload(NULL);
	aSource->setState(UserConnection::STATE_GET);

	if(BOOLSETTING(LOG_UPLOADS)) {
		StringMap params;
		params["source"] = u->getFileName();
		params["user"] = aSource->getUser()->getNick();
		params["size"] = Util::toString(u->getSize());
		params["sizeshort"] = Util::formatBytes(u->getSize());
		params["chunksize"] = Util::toString(u->getTotal());
		params["chunksizeshort"] = Util::formatBytes(u->getTotal());
		params["speed"] = Util::formatBytes(u->getAverageSpeed()) + "/s";
		params["time"] = Util::formatSeconds((GET_TICK() - u->getStart()) / 1000);
		LOG(UPLOAD_AREA, Util::formatParams(SETTING(LOG_FORMAT_POST_UPLOAD), params));
	}

	fire(UploadManagerListener::COMPLETE, u);
	removeUpload(u);
}

void UploadManager::removeConnection(UserConnection::Ptr aConn) {
	dcassert(aConn->getUpload() == NULL);
	aConn->removeListener(this);
	if(aConn->isSet(UserConnection::FLAG_HASSLOT)) {
		running--;
		aConn->unsetFlag(UserConnection::FLAG_HASSLOT);
	} 
	if(aConn->isSet(UserConnection::FLAG_HASEXTRASLOT)) {
		extra--;
		aConn->unsetFlag(UserConnection::FLAG_HASEXTRASLOT);
	}
	ConnectionManager::getInstance()->putUploadConnection(aConn);
}

void UploadManager::onTimerMinute(u_int32_t aTick) {
	Lock l(cs);
	for(SlotIter j = reservedSlots.begin(); j != reservedSlots.end();) {
		if(j->second + 600 * 1000 < aTick) {
			reservedSlots.erase(j++);
		} else {
			++j;
		}
	}
}	

// TimerManagerListener
void UploadManager::onAction(TimerManagerListener::Types type, u_int32_t aTick) {
	switch(type) {
	case TimerManagerListener::SECOND: 
		{
			Lock l(cs);
			Upload::List ticks;
			
			for(Upload::Iter i = uploads.begin(); i != uploads.end(); ++i) {
				ticks.push_back(*i);
			}
			
			if(ticks.size() > 0)
				fire(UploadManagerListener::TICK, ticks);
		}
		break;
	case TimerManagerListener::MINUTE: onTimerMinute(aTick);	break;
		break;
	}
}

// UserConnectionListener
void UploadManager::onAction(UserConnectionListener::Types type, UserConnection* conn) {
	switch(type) {
	case UserConnectionListener::TRANSMIT_DONE:
		onTransmitDone(conn); break;
	case UserConnectionListener::SEND:
		onSend(conn); break;
	case UserConnectionListener::GET_LIST_LENGTH:
		conn->listLen(ShareManager::getInstance()->getListLenString()); break;
	}
}
void UploadManager::onAction(UserConnectionListener::Types type, UserConnection* conn, u_int32_t bytes) {
	switch(type) {
	case UserConnectionListener::BYTES_SENT:
		onBytesSent(conn, bytes); break;
	}
}
void UploadManager::onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line) {
	switch(type) {
	case UserConnectionListener::FAILED:
		onFailed(conn, line); break;
	}
}
void UploadManager::onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line, int64_t resume) {
	switch(type) {
	case UserConnectionListener::GET:
		onGet(conn, line, resume); break;
	}
}

/**
 * @file UploadManager.cpp
 * $Id: UploadManager.cpp,v 1.34 2002/06/28 20:53:48 arnetheduck Exp $
 */
