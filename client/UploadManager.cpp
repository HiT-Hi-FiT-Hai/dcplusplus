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
#include "HashManager.h"
#include "AdcCommand.h"

static const string UPLOAD_AREA = "Uploads";

UploadManager::UploadManager() throw() : running(0), extra(0), lastGrant(0) { 
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

bool UploadManager::prepareFile(UserConnection* aSource, const string& aType, const string& aFile, int64_t aStartPos, int64_t aBytes) {
	if(aSource->getState() != UserConnection::STATE_GET) {
		dcdebug("UM:prepFile Wrong state, ignoring\n");
		return false;
	}
	
	dcassert(aFile.size() > 0);

	InputStream* is = NULL;
	int64_t size = 0;

	bool userlist = false;
	bool free = false;
	bool leaves = false;

	string file;
	try {
		file = ShareManager::getInstance()->translateFileName(aFile);
	} catch(const ShareException&) {
		aSource->fileNotAvail();
		return false;
	}

	if(aType == "file") {
		userlist = (Util::stricmp(aFile.c_str(), "files.xml.bz2") == 0);

		try {
			File* f = new File(file, File::READ, File::OPEN);

			size = f->getSize();

			free = userlist || (size <= (int64_t)(64 * 1024) );

			if(aBytes == -1) {
				aBytes = size - aStartPos;
			}

			if((aBytes < 0) || ((aStartPos + aBytes) > size)) {
				aSource->fileNotAvail();
				delete f;
				return false;
			}

			f->setPos(aStartPos);

			is = f;

			if((aStartPos + aBytes) < size) {
				is = new LimitedInputStream<true>(is, aBytes);
			}

		} catch(const Exception&) {
			aSource->fileNotAvail();
			return false;
		}

	} else if(aType == "tthl") {
		// TTH Leaves...
		TigerTree tree;
		if(!HashManager::getInstance()->getTree(file, tree)) {
			aSource->fileNotAvail();
			return false;
		}

		size = tree.getLeaves().size() * TTHValue::SIZE;
		aStartPos = 0;

		is = new TreeInputStream<TigerHash>(tree);	
		leaves = true;

	} else {
		aSource->fileNotAvail();
		return false;
	}

	Lock l(cs);

	bool extraSlot = false;

	if(!aSource->isSet(UserConnection::FLAG_HASSLOT)) {
		bool hasReserved = (reservedSlots.find(aSource->getUser()) != reservedSlots.end());
		bool isFavorite = aSource->getUser()->getFavoriteGrantSlot();

		if(!(hasReserved || isFavorite || getFreeSlots() > 0 || getAutoSlot())) {
			bool supportsFree = aSource->getUser()->isSet(User::DCPLUSPLUS) || aSource->isSet(UserConnection::FLAG_SUPPORTS_MINISLOTS);
			bool allowedFree = aSource->isSet(UserConnection::FLAG_HASEXTRASLOT) || aSource->getUser()->isSet(User::OP) || getFreeExtraSlots() > 0;
			if(free && supportsFree && allowedFree) {
				extraSlot = true;
			} else {
				aSource->maxedOut();
				aSource->disconnect();
				return false;
			}
		}

		setLastGrant(GET_TICK());
	}

	Upload* u = new Upload();
	u->setUserConnection(aSource);
	u->setFile(is);
	u->setSize(size);
	u->setStartPos(aStartPos);
	u->setFileName(aFile);
	u->setLocalFileName(file);

	if(userlist)
		u->setFlag(Upload::FLAG_USER_LIST);
	if(leaves)
		u->setFlag(Upload::FLAG_TTH_LEAVES);

	dcassert(aSource->getUpload() == NULL);
	aSource->setUpload(u);
	uploads.push_back(u);

	if(!aSource->isSet(UserConnection::FLAG_HASSLOT)) {
		if(extraSlot) {
			if(!aSource->isSet(UserConnection::FLAG_HASEXTRASLOT)) {
				aSource->setFlag(UserConnection::FLAG_HASEXTRASLOT);
				extra++;
			}
		} else {
			if(aSource->isSet(UserConnection::FLAG_HASEXTRASLOT)) {
				aSource->unsetFlag(UserConnection::FLAG_HASEXTRASLOT);
				extra--;
			}
			aSource->setFlag(UserConnection::FLAG_HASSLOT);
			running++;
		}
	}

	return true;
}

void UploadManager::on(UserConnectionListener::Get, UserConnection* aSource, const string& aFile, int64_t aResume) {
	if(prepareFile(aSource, "file", aFile, aResume, -1)) {
		aSource->setState(UserConnection::STATE_SEND);
		aSource->fileLength(Util::toString(aSource->getUpload()->getSize()));
	}
}

void UploadManager::onGetBlock(UserConnection* aSource, const string& aFile, int64_t aStartPos, int64_t aBytes, bool z) {
	if(!z || BOOLSETTING(COMPRESS_TRANSFERS)) {
		if(prepareFile(aSource, "file", aFile, aStartPos, aBytes)) {
			Upload* u = aSource->getUpload();
			dcassert(u != NULL);
			if(aBytes == -1)
				aBytes = u->getSize() - aStartPos;

			dcassert(aBytes >= 0);

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
	u->addPos(aBytes, aActual);
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

//void UploadManager::on(Command::STA, UserConnection* conn, const Command& c) throw() {

//}

void UploadManager::on(Command::GET, UserConnection* aSource, const Command& c) throw() {
	int64_t aBytes = Util::toInt64(c.getParam(3));
	int64_t aStartPos = Util::toInt64(c.getParam(2));

	if(prepareFile(aSource, c.getParam(0), Util::toNmdcFile(c.getParam(1)), aStartPos, aBytes)) {
		Upload* u = aSource->getUpload();
		dcassert(u != NULL);
		if(aBytes == -1)
			aBytes = u->getSize() - aStartPos;

		dcassert(aBytes >= 0);

		u->setStart(GET_TICK());

		Command cmd = Command(Command::SND());
		cmd.addParam(c.getParam(0));
		cmd.addParam(c.getParam(1));
		cmd.addParam(c.getParam(2));
		cmd.addParam(Util::toString(u->getSize()));

		if(c.hasFlag("ZL", 4)) {
			u->setFile(new FilteredInputStream<ZFilter, true>(u->getFile()));
			u->setFlag(Upload::FLAG_ZUPLOAD);
			cmd.addParam("ZL1");
		}

		aSource->send(cmd);
		aSource->setState(UserConnection::STATE_DONE);
		aSource->transmitFile(u->getFile());
		fire(UploadManagerListener::Starting(), u);
	}
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
 * $Id: UploadManager.cpp,v 1.61 2004/05/22 15:28:07 arnetheduck Exp $
 */
