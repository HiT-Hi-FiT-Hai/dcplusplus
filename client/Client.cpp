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

#include "Client.h"

#include "BufferedSocket.h"

#include "FavoriteManager.h"

Client::Counts Client::counts;

Client::Client(const string& hubURL, char separator) : 
	socket(BufferedSocket::getSocket(separator)), reconnDelay(120), registered(false), port(0), countType(COUNT_UNCOUNTED)
{
	string file;
	Util::decodeUrl(hubURL, address, port, file);
	addressPort = hubURL;
	socket->addListener(this);
}

Client::~Client() throw() {
	socket->removeListener(this);

	updateCounts(true);
}

void Client::reloadSettings() {
	FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(getHubURL());
	if(hub) {
		setNick(checkNick(hub->getNick(true)));
		setDescription(hub->getUserDescription());
		setPassword(hub->getPassword());
	} else {
		setNick(checkNick(SETTING(NICK)));
	}
}

void Client::connect() {
	reloadSettings();
	socket->connect(address, port);
}

void Client::updateCounts(bool aRemove) {
	// We always remove the count and then add the correct one if requested...
	if(countType == COUNT_NORMAL) {
		Thread::safeDec(counts.normal);
	} else if(countType == COUNT_REGISTERED) {
		Thread::safeDec(counts.registered);
	} else if(countType == COUNT_OP) {
		Thread::safeDec(counts.op);
	}
	countType = COUNT_UNCOUNTED;

	if(!aRemove) {
		if(getOp()) {
			Thread::safeInc(counts.op);
			countType = COUNT_OP;
		} else if(registered) {
			Thread::safeInc(counts.registered);
			countType = COUNT_REGISTERED;
		} else {
			Thread::safeInc(counts.normal);
			countType = COUNT_NORMAL;
		}
	}
}

string Client::getLocalIp() const { 
	if(!SETTING(SERVER).empty()) {
		return Socket::resolve(SETTING(SERVER));
	}
	/// @todo if(getMe() && !getMe()->getIp().empty())
		//return getMe()->getIp();

	if(socket == NULL)
		return Util::getLocalIp();
	string lip = socket->getLocalIp();
	if(lip.empty())
		return Util::getLocalIp();
	return lip;
}

/**
 * @file
 * $Id: Client.cpp,v 1.83 2005/04/12 23:24:12 arnetheduck Exp $
 */
