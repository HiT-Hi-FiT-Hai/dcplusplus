/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#include "ClientManager.h"

ClientManager* ClientManager::instance = NULL;

Client* ClientManager::getClient() {
	cs.enter();
	Client* c = new Client();

	c->addListener(this);
	clients.push_back(c);
	cs.leave();
	return c;
}

void ClientManager::putClient(Client* aClient) {
	cs.enter();
	aClient->removeListeners();
	aClient->disconnect();

	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if( (*i) == aClient) {
			clients.erase(i);
			break;
		}
	}
	aClient->cs.enter();		// Just make sure noone's using it still...
	aClient->cs.leave();	
	delete aClient;
	cs.leave();
}

/**
 * @file ClientManager.cpp
 * $Id: ClientManager.cpp,v 1.1 2001/12/21 18:46:18 arnetheduck Exp $
 * @if LOG
 * $Log: ClientManager.cpp,v $
 * Revision 1.1  2001/12/21 18:46:18  arnetheduck
 * Replaces ProtocolHandler with enhanced functionality
 *
 * @endif
 */

