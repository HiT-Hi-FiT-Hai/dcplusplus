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

#include "User.h"
#include "Client.h"

User::Ptr User::nuser = User::Ptr(NULL);

void User::connect() {
	Lock l(cs);
	if(client) {
		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
			client->connectToMe(this);
		} else {
			client->revConnectToMe(this);
		}
	}
}

string User::getClientNick() {
	Lock l(cs);
	if(client) {
		return client->getNick();
	} else {
		return SETTING(NICK);
	}
}

void User::update() {
	Lock l(cs);
	if(client) {
		client->getInfo(this);
	}
}

void User::updated(User::Ptr& aUser) {
	Lock l(aUser->cs);
	if(aUser->client) {
		aUser->client->updated(aUser);
	}
}
string User::getClientName() {
	Lock l(cs);
	if(client) {
		return client->getName();
	} else {
		return "Offline";
	}
}

void User::privateMessage(const string& aMsg) {
	Lock l(cs);
	if(client) {
		client->privateMessage(this, aMsg);
	}
}

bool User::isClientOp() {
	Lock l(cs);
	if(client) {
		return client->getOp();
	}
	return false;
}

void User::kick(const string& aMsg) {
	Lock l(cs);
	if(client) {
		client->privateMessage(this, "You are being kicked because: " + aMsg);
		// Short, short break to allow the message to reach the client...
		Sleep(10);
		client->sendMessage(client->getNick() + " is kicking " + getNick() + " because: " + aMsg);
		client->kick(this);
	}
}

void User::redirect(const string& aTarget, const string& aReason) {
	Lock l(cs);
	if(client) {
		client->opForceMove(this, aTarget, aReason);
	}
}

void User::clientMessage(const string& aMsg) {
	Lock l(cs);
	if(client) {
		client->sendMessage(aMsg);
	}
}

/**
 * @file User.cpp
 * $Id: User.cpp,v 1.7 2002/02/18 23:48:32 arnetheduck Exp $
 * @if LOG
 * $Log: User.cpp,v $
 * Revision 1.7  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.6  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.5  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.4  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.3  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.2  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.1  2001/12/01 17:17:22  arnetheduck
 * New additions to the reworked connection manager and huffman encoder
 *
 * @endif
 */

