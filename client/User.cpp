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

#include "SettingsManager.h"
#include "ResourceManager.h"
#include "TimerManager.h"

#include "User.h"

#include "Client.h"
#include "FavoriteUser.h"

User::~User() throw() {
	delete favoriteUser;
}

void User::connect() {
	RLock l(cs);
	if(client) {
		client->connect(this);
	}
}

const string& User::getClientNick() const {
	RLock l(cs);
	if(client) {
		return client->getNick();
	} else {
		return SETTING(NICK);
	}
}

const CID User::getClientCID() const {
	RLock l(cs);
	if(client) {
		return client->getMe()->getCID();
	} else {
		return CID(SETTING(CLIENT_ID));
	}
}

void User::updated(User::Ptr& aUser) {
	RLock l(aUser->cs);
	if(aUser->client) {
		aUser->client->updated(aUser);
	}
}

const string& User::getClientName() const {
	RLock l(cs);
	if(client) {
		return client->getName();
	} else if(!getLastHubName().empty()) {
		return getLastHubName();
	} else {
		return STRING(OFFLINE);
	}
}

string User::getClientAddressPort() const {
	RLock l(cs);
	if(client) {
		return client->getAddressPort();
	} else {
		return Util::emptyString;
	}
}

void User::privateMessage(const string& aMsg) {
	RLock l(cs);
	if(client) {
		client->privateMessage(this, aMsg);
	}
}

bool User::isClientOp() const {
	RLock l(cs);
	if(client) {
		return client->getOp();
	}
	return false;
}

void User::kick(const string& aMsg) {
	RLock l(cs);
	if(client) {
		client->kick(this, aMsg);
	}
}

void User::send(const string& aMsg) {
	RLock l(cs);
	if(client) {
		client->send(aMsg);
	}
}

void User::redirect(const string& aTarget, const string& aReason) {
	RLock l(cs);
	if(client) {
		client->redirect(this, aTarget, aReason);
	}
}

void User::clientMessage(const string& aMsg) {
	RLock l(cs);
	if(client) {
		client->hubMessage(aMsg);
	}
}

void User::setClient(Client* aClient) { 
	WLock l(cs); 
	client = aClient; 
	if(client == NULL) {
		if (isSet(ONLINE) && isFavoriteUser())
			setFavoriteLastSeen();
		unsetFlag(ONLINE);
	}
	else {
		setLastHubAddress(aClient->getIpPort());
		setLastHubName(aClient->getName());
		setFlag(ONLINE);
		unsetFlag(QUIT_HUB);
	}
};

void User::getParams(StringMap& ucParams) {
	ucParams["nick"] = getNick();
	ucParams["cid"] = getCID().toBase32();
	ucParams["tag"] = getTag();
	ucParams["description"] = getDescription();
	ucParams["email"] = getEmail();
	ucParams["share"] = Util::toString(getBytesShared());
	ucParams["shareshort"] = Util::formatBytes(getBytesShared());
	ucParams["ip"] = getIp();
}

// favorite user stuff
void User::setFavoriteUser(FavoriteUser* aUser) {
	WLock l(cs);
	delete favoriteUser;
	favoriteUser = aUser;
}

bool User::isFavoriteUser() const {
	RLock l(cs);
	return (favoriteUser != NULL);
}

bool User::getFavoriteGrantSlot() const {
	RLock l(cs);
	return (favoriteUser != NULL && favoriteUser->isSet(FavoriteUser::FLAG_GRANTSLOT));
}

void User::setFavoriteGrantSlot(bool grant) {
	WLock l(cs);
	if (favoriteUser == NULL)
		return;

	if (grant)
		favoriteUser->setFlag(FavoriteUser::FLAG_GRANTSLOT);
	else
		favoriteUser->unsetFlag(FavoriteUser::FLAG_GRANTSLOT);
}

void User::setFavoriteLastSeen(u_int32_t anOfflineTime) {
	WLock l(cs);
	if (favoriteUser != NULL) {
		if (anOfflineTime != 0)
			favoriteUser->setLastSeen(anOfflineTime);
		else
			favoriteUser->setLastSeen(GET_TIME());
	}
}

u_int32_t User::getFavoriteLastSeen() const {
	RLock l(cs);
	if (favoriteUser != NULL)
		return favoriteUser->getLastSeen();
	else
		return 0;
}

const string& User::getUserDescription() const {
	RLock l(cs);
	if (favoriteUser != NULL)
		return favoriteUser->getDescription();
	else
		return Util::emptyString;
}

void User::setUserDescription(const string& aDescription) {
	WLock l(cs);
	if (favoriteUser != NULL)
		favoriteUser->setDescription(aDescription);
}

StringMap& User::clientEscapeParams(StringMap& sm) const {
	return client->escapeParams(sm);
}

/**
 * @file
 * $Id: User.cpp,v 1.33 2004/09/07 01:36:52 arnetheduck Exp $
 */

