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

#include "User.h"
#include "Client.h"

void User::connect() {
	RLock l(cs);
	if(client) {
		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
			client->connectToMe(this);
		} else {
			client->revConnectToMe(this);
		}
	}
}

const string& User::getClientNick() {
	RLock l(cs);
	if(client) {
		return client->getNick();
	} else {
		return SETTING(NICK);
	}
}

void User::update() {
	RLock l(cs);
	if(client) {
		client->getInfo(this);
	}
}

void User::updated(User::Ptr& aUser) {
	RLock l(aUser->cs);
	if(aUser->client) {
		aUser->client->updated(aUser);
	}
}

const string& User::getClientName() {
	RLock l(cs);
	if(client) {
		return client->getName();
	} else {
		return STRING(OFFLINE);
	}
}

void User::privateMessage(const string& aMsg) {
	RLock l(cs);
	if(client) {
		client->privateMessage(this, aMsg);
	}
}

bool User::isClientOp() {
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

void User::redirect(const string& aTarget, const string& aReason) {
	RLock l(cs);
	if(client) {
		client->opForceMove(this, aTarget, aReason);
	}
}

void User::clientMessage(const string& aMsg) {
	RLock l(cs);
	if(client) {
		client->sendMessage(aMsg);
	}
}

void User::setClient(Client* aClient) { 
	WLock l(cs); 
	client = aClient; 
	if(client == NULL)
		unsetFlag(ONLINE);
	else {
		setLastHubIp(aClient->getIp());
		setLastHubName(aClient->getName());
		setFlag(ONLINE);
	}
};

/**
 * @file User.cpp
 * $Id: User.cpp,v 1.14 2002/04/16 16:45:54 arnetheduck Exp $
 */

