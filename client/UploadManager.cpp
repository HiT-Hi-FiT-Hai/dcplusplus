/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "FavoriteManager.h"

#include <functional>

static const string UPLOAD_AREA = "Uploads";

UploadManager::UploadManager() throw() : running(0), extra(0), lastGrant(0) { 
	ClientManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
}

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

bool UploadManager::prepareFile(UserConnection* aSource, const string& aType, const string& aFile, int64_t aStartPos, int64_t aBytes, bool listRecursive) {
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
	bool partList = false;

	string file;
	try {
		if(aType == "file") {
			file = ShareManager::getInstance()->translateFileName(aFile);
			userlist = (aFile == "files.xml.bz2");

			try {
				File* f = new File(file, File::READ, File::OPEN);

				size = f->getSize();

				free = userlist || (size <= (int64_t)(SETTING(SET_MINISLOT_SIZE) * 1024) );

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
			MemoryInputStream* mis = ShareManager::getInstance()->getTree(aFile);
			file = ShareManager::getInstance()->translateFileName(aFile);
			if(mis == NULL) {
				aSource->fileNotAvail();
				return false;
			}

			size = mis->getSize();
			aStartPos = 0;
			is = mis;
			leaves = true;
			free = true;
		} else if(aType == "list") {
			// Partial file list
			MemoryInputStream* mis = ShareManager::getInstance()->generatePartialList(aFile, listRecursive);
			if(mis == NULL) {
				aSource->fileNotAvail();
				return false;
			}
			// Some old dc++ clients err here...
			aBytes = -1;
			size = mis->getSize();
			aStartPos = 0;
			is = mis;
			free = true;
			partList = true;
		} else {
			aSource->fileNotAvail();
			return false;
		}
	} catch(const ShareException&) {
		aSource->fileNotAvail();
		return false;
	}

	Lock l(cs);

	bool extraSlot = false;

	if(!aSource->isSet(UserConnection::FLAG_HASSLOT)) {
		bool hasReserved = (reservedSlots.find(aSource->getUser()) != reservedSlots.end());
		bool isFavorite = FavoriteManager::getInstance()->hasSlot(aSource->getUser());

		if(!(hasReserved || isFavorite || getFreeSlots() > 0 || getAutoSlot())) {
			bool supportsFree = aSource->isSet(UserConnection::FLAG_SUPPORTS_MINISLOTS) || !aSource->isSet(UserConnection::FLAG_NMDC);
			bool allowedFree = aSource->isSet(UserConnection::FLAG_HASEXTRASLOT) || aSource->isSet(UserConnection::FLAG_OP) || getFreeExtraSlots() > 0;
			if(free && supportsFree && allowedFree) {
				extraSlot = true;
			} else {
				delete is;
				aSource->maxedOut();

				// Check for tth root identifier
				string tFile = aFile;
				if (tFile.compare(0, 4, "TTH/") == 0)
					tFile = ShareManager::getInstance()->translateTTH(aFile.substr(4));

				addFailedUpload(aSource, tFile +
					" (" + Util::toString((aStartPos*1000/(File::getSize(file)+10))/10.0)+"% of " + Util::formatBytes(File::getSize(file)) + " done)");
				aSource->disconnect();
				return false;
			}
		} else {
			clearUserFiles(aSource->getUser());	// this user is using a full slot, nix them.
		}

		setLastGrant(GET_TICK());
	}

	Upload* u = new Upload();
	u->setUserConnection(aSource);
	u->setFile(is);
	if(aBytes == -1)
		u->setSize(size);
	else
		u->setSize(aStartPos + aBytes);

	u->setStartPos(aStartPos);
	u->setFileName(file);
	u->setLocalFileName(file);

	if(userlist)
		u->setFlag(Upload::FLAG_USER_LIST);
	if(leaves)
		u->setFlag(Upload::FLAG_TTH_LEAVES);
	if(partList)
		u->setFlag(Upload::FLAG_PARTIAL_LIST);

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

		reservedSlots.erase(aSource->getUser());
	}

	return true;
}

void UploadManager::removeUpload(Upload* aUpload) {
	Lock l(cs);
	dcassert(find(uploads.begin(), uploads.end(), aUpload) != uploads.end());
	uploads.erase(find(uploads.begin(), uploads.end(), aUpload));
	aUpload->setUserConnection(NULL);
	delete aUpload;
}

void UploadManager::reserveSlot(const User::Ptr& aUser) {
	{
		Lock l(cs);
		reservedSlots.insert(aUser);
	}
	if(aUser->isOnline())
		ClientManager::getInstance()->connect(aUser);
}

void UploadManager::on(UserConnectionListener::Get, UserConnection* aSource, const string& aFile, int64_t aResume) throw() {
	if(prepareFile(aSource, "file", Util::toAdcFile(aFile), aResume, -1)) {
		aSource->setState(UserConnection::STATE_SEND);
		aSource->fileLength(Util::toString(aSource->getUpload()->getSize()));
	}
}

void UploadManager::onGetBlock(UserConnection* aSource, const string& aFile, int64_t aStartPos, int64_t aBytes, bool z) {
	if(!z || BOOLSETTING(COMPRESS_TRANSFERS)) {
		if(prepareFile(aSource, "file", Util::toAdcFile(aFile), aStartPos, aBytes)) {
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

void UploadManager::on(UserConnectionListener::Send, UserConnection* aSource) throw() {
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

void UploadManager::on(UserConnectionListener::BytesSent, UserConnection* aSource, size_t aBytes, size_t aActual) throw() {
	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Upload* u = aSource->getUpload();
	dcassert(u != NULL);
	u->addPos(aBytes, aActual);
}

void UploadManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& aError) throw() {
	Upload* u = aSource->getUpload();

	if(u) {
		aSource->setUpload(NULL);
		fire(UploadManagerListener::Failed(), u, aError);

		dcdebug("UM::onFailed: Removing upload\n");
		removeUpload(u);
	}

	removeConnection(aSource, false);
}

void UploadManager::on(UserConnectionListener::TransmitDone, UserConnection* aSource) throw() {
	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Upload* u = aSource->getUpload();
	dcassert(u != NULL);

	aSource->setUpload(NULL);
	aSource->setState(UserConnection::STATE_GET);

	if(BOOLSETTING(LOG_UPLOADS) && !u->isSet(Upload::FLAG_TTH_LEAVES) && (BOOLSETTING(LOG_FILELIST_TRANSFERS) || !u->isSet(Upload::FLAG_USER_LIST))) {
		StringMap params;
		params["source"] = u->getFileName();
		params["userNI"] = Util::toString(ClientManager::getInstance()->getNicks(aSource->getUser()->getCID()));
		params["userI4"] = aSource->getRemoteIp();
		StringList hubNames = ClientManager::getInstance()->getHubNames(aSource->getUser()->getCID());
		if(hubNames.empty())
			hubNames.push_back(STRING(OFFLINE));
		params["hub"] = Util::toString(hubNames);
		StringList hubs = ClientManager::getInstance()->getHubs(aSource->getUser()->getCID());
		if(hubs.empty())
			hubs.push_back(STRING(OFFLINE));
		params["hubURL"] = Util::toString(hubs);
		params["fileSI"] = Util::toString(u->getSize());
		params["fileSIshort"] = Util::formatBytes(u->getSize());
		params["fileSIchunk"] = Util::toString(u->getTotal());
		params["fileSIchunkshort"] = Util::formatBytes(u->getTotal());
		params["fileSIactual"] = Util::toString(u->getActual());
		params["fileSIactualshort"] = Util::formatBytes(u->getActual());
		params["speed"] = Util::formatBytes(u->getAverageSpeed()) + "/s";
		params["time"] = Util::formatSeconds((GET_TICK() - u->getStart()) / 1000);

		if(u->getTTH() != NULL) {
			params["tth"] = u->getTTH()->toBase32();
		}
		LOG(LogManager::UPLOAD, params);
	}

	fire(UploadManagerListener::Complete(), u);
	removeUpload(u);
}

void UploadManager::addFailedUpload(UserConnection::Ptr source, string filename) {
	{
		Lock l(cs);
		if (!count_if(waitingUsers.begin(), waitingUsers.end(), UserMatch(source->getUser())))
			waitingUsers.push_back(WaitingUser(source->getUser(), GET_TICK()));
		waitingFiles[source->getUser()].insert(filename);		//files for which user's asked
	}

	fire(UploadManagerListener::WaitingAddFile(), source->getUser(), filename);
}

void UploadManager::clearUserFiles(const User::Ptr& source) {
	Lock l(cs);
	//run this when a user's got a slot or goes offline.
	UserDeque::iterator sit = find_if(waitingUsers.begin(), waitingUsers.end(), UserMatch(source));
	if (sit == waitingUsers.end()) return;

	FilesMap::iterator fit = waitingFiles.find(sit->first);
	if (fit != waitingFiles.end()) waitingFiles.erase(fit);
	fire(UploadManagerListener::WaitingRemoveUser(), sit->first);

	waitingUsers.erase(sit);
}

vector<User::Ptr> UploadManager::getWaitingUsers() {
	Lock l(cs);
	vector<User::Ptr> u;
	transform(waitingUsers.begin(), waitingUsers.end(), back_inserter(u), select1st<WaitingUser>());
	return u;
}

const UploadManager::FileSet& UploadManager::getWaitingUserFiles(const User::Ptr &u) {
	Lock l(cs);
	return waitingFiles.find(u)->second;
}

void UploadManager::removeConnection(UserConnection::Ptr aConn, bool ntd) {
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
}

void UploadManager::on(TimerManagerListener::Minute, u_int32_t /* aTick */) throw() {
	Lock l(cs);

	UserDeque::iterator i = stable_partition(waitingUsers.begin(), waitingUsers.end(), WaitingUserFresh());
	for (UserDeque::iterator j = i; j != waitingUsers.end(); ++j) {
		FilesMap::iterator fit = waitingFiles.find(j->first);
		if (fit != waitingFiles.end()) waitingFiles.erase(fit);
		fire(UploadManagerListener::WaitingRemoveUser(), j->first);
	}

	waitingUsers.erase(i, waitingUsers.end());
}

void UploadManager::on(GetListLength, UserConnection* conn) throw() { 
	conn->listLen(ShareManager::getInstance()->getListLenString()); 
}

void UploadManager::on(AdcCommand::NTD, UserConnection* aConn, const AdcCommand&) throw() {
	removeConnection(aConn, true);
}

void UploadManager::on(AdcCommand::GET, UserConnection* aSource, const AdcCommand& c) throw() {
	int64_t aBytes = Util::toInt64(c.getParam(3));
	int64_t aStartPos = Util::toInt64(c.getParam(2));
	const string& fname = c.getParam(1);
	const string& type = c.getParam(0);
	string tmp;

	if(prepareFile(aSource, type, fname, aStartPos, aBytes, c.hasFlag("RE", 4))) {
		Upload* u = aSource->getUpload();
		dcassert(u != NULL);
		if(aBytes == -1)
			aBytes = u->getSize() - aStartPos;

		dcassert(aBytes >= 0);

		u->setStart(GET_TICK());

		AdcCommand cmd(AdcCommand::CMD_SND);
		cmd.addParam(c.getParam(0));
		cmd.addParam(c.getParam(1));
		cmd.addParam(Util::toString(u->getPos()));
		cmd.addParam(Util::toString(u->getSize() - u->getPos()));

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

void UploadManager::on(AdcCommand::GFI, UserConnection* aSource, const AdcCommand& c) throw() {
	if(c.getParameters().size() < 2) {
		aSource->sta(AdcCommand::SEV_RECOVERABLE, AdcCommand::ERROR_PROTOCOL_GENERIC, "Missing parameters");
		return;
	}

	const string& type = c.getParam(0);
	const string& ident = c.getParam(1);

	if(type == "file") {
		SearchResult::List l;
		StringList sl;

		if(ident.compare(0, 4, "TTH/") != 0) {
			aSource->sta(AdcCommand::SEV_RECOVERABLE, AdcCommand::ERROR_PROTOCOL_GENERIC, "Invalid identifier");
			return;
		}
		sl.push_back("TH" + ident.substr(4));
		ShareManager::getInstance()->search(l, sl, 1);
		if(l.empty()) {
			aSource->sta(AdcCommand::SEV_RECOVERABLE, AdcCommand::ERROR_FILE_NOT_AVAILABLE, "Not found");
		} else {
			aSource->send(l[0]->toRES(AdcCommand::TYPE_CLIENT));
			l[0]->decRef();
		}
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

void UploadManager::on(ClientManagerListener::UserDisconnected, const User::Ptr& aUser) throw() {

	/// @todo Don't kick when /me disconnects
	if( BOOLSETTING(AUTO_KICK) ) {

		Lock l(cs);
		for(Upload::Iter i = uploads.begin(); i != uploads.end(); ++i) {
			Upload* u = *i;
			if(u->getUser() == aUser) {
				// Oops...adios...
				u->getUserConnection()->disconnect();
				// But let's grant him/her a free slot just in case...
				if (!u->getUserConnection()->isSet(UserConnection::FLAG_HASEXTRASLOT))
					reserveSlot(aUser);
				LogManager::getInstance()->message(STRING(DISCONNECTED_USER) + aUser->getFirstNick());
			}
		}
	}

	//Remove references to them.
	if(!aUser->isOnline()) {
		clearUserFiles(aUser);
	}
}

/**
 * @file
 * $Id: UploadManager.cpp,v 1.102 2006/01/15 18:40:39 arnetheduck Exp $
 */
