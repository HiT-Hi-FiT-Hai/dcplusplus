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

#include "UploadManager.h"

#include "ConnectionManager.h"
#include "LogManager.h"
#include "ShareManager.h"
#include "ClientManager.h"
#include "FilteredFile.h"
#include "ZUtils.h"
#include "ResourceManager.h"

static const string UPLOAD_AREA = "Uploads";

UploadManager::UploadManager() throw() : running(0), extra(0), lastAutoGrant(0) { 
	ClientManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
};

UploadManager::~UploadManager() throw() {
	TimerManager::getInstance()->removeListener(this);
	ClientManager::getInstance()->removeListener(this);
	while(true) {
		{
			Lock l(cs);
			if(uploads.empty())
				break;
		}
		Thread::sleep(100);
	}
}

bool UploadManager::prepareFile(UserConnection* aSource, const string& aFile, int64_t aResume, int64_t aBytes) {
	if(aSource->getState() != UserConnection::STATE_GET) {
		dcdebug("UM:onGet Wrong state, ignoring\n");
		return false;
	}
	
	dcassert(aFile.size() > 0);
	
	bool userlist = false;

	string file;
	try {
		file = ShareManager::getInstance()->translateFileName(aFile);
	} catch(const ShareException&) {
		aSource->error("File Not Available");
		return false;
	}
	
	if( (Util::stricmp(aFile.c_str(), "MyList.bz2") == 0) || Util::stricmp(aFile.c_str(), "files.xml.bz2") == 0) {
		userlist = true;
	}

	int64_t sz = File::getSize(file);
	if(sz == -1) {
		aSource->error("File Not Available");
		return false;
	} else if(aResume >= sz) {
		// Can't do...
		aSource->disconnect();
		return false;
	}

	bool smallfile = (sz < (int64_t)(32 * 1024) );

	cs.enter();
	SlotIter ui = reservedSlots.find(aSource->getUser());

	if( (!aSource->isSet(UserConnection::FLAG_HASSLOT)) && 
		(getFreeSlots()<=0) && 
		(ui == reservedSlots.end()) &&
		(!aSource->getUser()->getFavoriteGrantSlot())) 
	{
		dcdebug("Average speed: %s/s\n", Util::formatBytes(UploadManager::getInstance()->getAverageSpeed()).c_str());
		if( ((getLastAutoGrant() + 30*1000) > GET_TICK()) || (SETTING(MIN_UPLOAD_SPEED) == 0) || ( (SETTING(MIN_UPLOAD_SPEED)*1024) < UploadManager::getInstance()->getAverageSpeed() ) ) {
			if( !(smallfile || userlist) ||
				!(aSource->isSet(UserConnection::FLAG_HASEXTRASLOT) || (getFreeExtraSlots() > 0) || (aSource->getUser()->isSet(User::OP)) ) || 
				!(aSource->getUser()->isSet(User::DCPLUSPLUS) || aSource->isSet(UserConnection::FLAG_SUPPORTS_MINISLOTS)) 
				) 
			{
				cs.leave();
				aSource->maxedOut();
				removeConnection(aSource);
				return false;
			}
		}
		setLastAutoGrant(GET_TICK());
	}

	File* f;
	try {
		f = new File(file, File::READ, File::OPEN);
	} catch(const FileException&) {
		// Why isn't this possible?...let's reload the directory listing...
		if(Util::stricmp(aFile.c_str(), "MyList.DcLst") == 0 || userlist) {
			
			ShareManager::getInstance()->refresh(true, true, true);
			try {
				f = new File(file, File::READ, File::OPEN);
			} catch(const FileException&) {
				// Still not...very strange...?
				cs.leave();
				aSource->error("File Not Available");
				return false;
			}
		} else {
			cs.leave();
			aSource->error("File Not Available");
			return false;
		}
	}
	
	Upload* u = new Upload();
	u->setUserConnection(aSource);
	u->setFile(f);
	u->setSize(f->getSize());
	f->setPos(aResume);
	u->setPos(aResume);
	u->setFileName(aFile);
	u->setLocalFileName(file);

	if(aBytes != -1 && (aResume + aBytes) < f->getSize()) {
		u->setFile(new LimitedInputStream<true>(u->getFile(), aBytes));
	}

	if(smallfile)
		u->setFlag(Upload::FLAG_SMALL_FILE);
	if(userlist)
		u->setFlag(Upload::FLAG_USER_LIST);

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
	
	cs.leave();
	return true;
}

void UploadManager::on(UserConnectionListener::Get, UserConnection* aSource, const string& aFile, int64_t aResume) {
	if(prepareFile(aSource, aFile, aResume, -1)) {
		aSource->setState(UserConnection::STATE_SEND);
		aSource->fileLength(Util::toString(aSource->getUpload()->getSize()));
	}
}

void UploadManager::onGetBlock(UserConnection* aSource, const string& aFile, int64_t aResume, int64_t aBytes, bool z) {
	if(!z || BOOLSETTING(COMPRESS_TRANSFERS)) {
		if(prepareFile(aSource, aFile, aResume, aBytes)) {
			Upload* u = aSource->getUpload();
			dcassert(u != NULL);
			if(aBytes == -1)
				aBytes = aResume - u->getSize();

			if(aBytes < 0 || (u->getPos() + aBytes) > u->getSize()) {
				// Can't do...
				aSource->disconnect();
				return;
			}

			u->setStart(GET_TICK());
			if(z) {
				u->setFile(new FilteredInputStream<ZFilter, true>(u->getFile()));
				u->setFlag(Upload::FLAG_ZUPLOAD);
			}

			aSource->sending(aBytes);
			aSource->setState(UserConnection::STATE_DONE);
			aSource->transmitFile(u->getFile());
			fire(UploadManagerListener::Starting(), u);
		}
	}
}

void UploadManager::on(UserConnectionListener::Send, UserConnection* aSource) {
	if(aSource->getState() != UserConnection::STATE_SEND) {
		dcdebug("UM::onSend Bad state, ignoring\n");
		return;
	}

	Upload* u = aSource->getUpload();
	dcassert(u != NULL);

	u->setStart(GET_TICK());
	aSource->setState(UserConnection::STATE_DONE);
	aSource->transmitFile(u->getFile());
	fire(UploadManagerListener::Starting(), u);
}

void UploadManager::on(UserConnectionListener::BytesSent, UserConnection* aSource, size_t aBytes, size_t aActual) {
	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Upload* u = aSource->getUpload();
	dcassert(u != NULL);
	u->addPos(aBytes);
	u->addActual(aActual);
}

void UploadManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& aError) {
	Upload* u = aSource->getUpload();

	if(u) {
		aSource->setUpload(NULL);
		fire(UploadManagerListener::Failed(), u, aError);

		dcdebug("UM::onFailed: Removing upload\n");
		removeUpload(u);
	}

	removeConnection(aSource);
}

void UploadManager::on(UserConnectionListener::TransmitDone, UserConnection* aSource) {
	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Upload* u = aSource->getUpload();
	dcassert(u != NULL);

	aSource->setUpload(NULL);
	aSource->setState(UserConnection::STATE_GET);

	if(BOOLSETTING(LOG_UPLOADS)  && (BOOLSETTING(LOG_FILELIST_TRANSFERS) || !u->isSet(Upload::FLAG_USER_LIST))) {
		StringMap params;
		params["source"] = u->getFileName();
		params["user"] = aSource->getUser()->getNick();
		params["hub"] = aSource->getUser()->getLastHubName();
		params["hubip"] = aSource->getUser()->getLastHubAddress();
		params["size"] = Util::toString(u->getSize());
		params["sizeshort"] = Util::formatBytes(u->getSize());
		params["chunksize"] = Util::toString(u->getTotal());
		params["chunksizeshort"] = Util::formatBytes(u->getTotal());
		params["actualsize"] = Util::toString(u->getActual());
		params["actualsizeshort"] = Util::formatBytes(u->getActual());
		params["speed"] = Util::formatBytes(u->getAverageSpeed()) + "/s";
		params["time"] = Util::formatSeconds((GET_TICK() - u->getStart()) / 1000);
		LOG(UPLOAD_AREA, Util::formatParams(SETTING(LOG_FORMAT_POST_UPLOAD), params));
	}

	fire(UploadManagerListener::Complete(), u);
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

void UploadManager::on(TimerManagerListener::Minute, u_int32_t aTick) throw() {
	Lock l(cs);
	for(SlotIter j = reservedSlots.begin(); j != reservedSlots.end();) {
		if(j->second + 600 * 1000 < aTick) {
			reservedSlots.erase(j++);
		} else {
			++j;
		}
	}
}	

void UploadManager::on(GetListLength, UserConnection* conn) throw() { 
	conn->listLen(ShareManager::getInstance()->getListLenString()); 
}

void UploadManager::on(Command::STA, UserConnection* conn, const Command& c) throw() {

}

void UploadManager::on(Command::GET, UserConnection* conn, const Command& c) throw() {
	
}
// TimerManagerListener
void UploadManager::on(TimerManagerListener::Second, u_int32_t) throw() {
	Lock l(cs);
	Upload::List ticks;
	
	for(Upload::Iter i = uploads.begin(); i != uploads.end(); ++i) {
		ticks.push_back(*i);
	}
	
	if(ticks.size() > 0)
		fire(UploadManagerListener::Tick(), ticks);

}

void UploadManager::on(ClientManagerListener::UserUpdated, const User::Ptr& aUser) throw() {
	if( (!aUser->isOnline()) && 
		(aUser->isSet(User::QUIT_HUB)) && 
		(BOOLSETTING(AUTO_KICK)) ){

		Lock l(cs);
		for(Upload::Iter i = uploads.begin(); i != uploads.end(); ++i) {
			Upload* u = *i;
			if(u->getUser() == aUser) {
				// Oops...adios...
				u->getUserConnection()->disconnect();
				// But let's grant him/her a free slot just in case...
				if (!u->getUserConnection()->isSet(UserConnection::FLAG_HASEXTRASLOT))
					reserveSlot(aUser);
				LogManager::getInstance()->message(STRING(DISCONNECTED_USER) + aUser->getFullNick());
			}
		}
	}
}

/**
 * @file
 * $Id: UploadManager.cpp,v 1.57 2004/04/24 09:40:58 arnetheduck Exp $
 */
