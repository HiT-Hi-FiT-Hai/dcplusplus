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

#if !defined(AFX_PROTOCOLHANDLER_H__1F906161_663A_4D66_BF9E_A571E67DB0F1__INCLUDED_)
#define AFX_PROTOCOLHANDLER_H__1F906161_663A_4D66_BF9E_A571E67DB0F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ClientListener.h"
#include "DCClient.h"
#include "CryptoManager.h"
#include "UserConnection.h"
#include "IncomingManager.h"

/**
 * This is it, this is the class that controls what is sent when.
 * Add it to any DCClient created, or nothing will ever be sent to the
 * hub.
 */
class ProtocolHandler : public ClientListener
{
public:
	virtual void onLock(const string& aLock, const string& aPk)  {
		client->key(CryptoManager::makeKey(aLock));
		client->validateNick(Settings::getNick());
	}
	virtual void onHello(const string& aNick) {
		if(aNick.compare(Settings::getNick()) == 0) {
			client->version("1,0091");
			client->getNickList();
			client->myInfo(Settings::getNick(), Settings::getDescription(), Settings::getConnection(), Settings::getEmail(), "10000");
		} else {
			client->getInfo(aNick);
		}
	}
	virtual void onNickList(StringList& aNicks) {
		for(StringIter i = aNicks.begin(); i != aNicks.end(); ++i) {
			client->getInfo(*i);
		}
	}

	virtual void onOpList(StringList& aNicks) {
		for(StringIter i = aNicks.begin(); i != aNicks.end(); ++i) {
			client->getInfo(*i);
		}
	}

	virtual void onConnectToMe(const string& aServer, const string& aPort) {
		IncomingManager::getInstance()->connect(aServer, atoi(aPort.c_str()));
	}
	
	virtual void onRevConnectoToMe(const string& aNick) {
		client->connectToMe(aNick);
	}
	
	ProtocolHandler(DCClient::Ptr aClient) : client(aClient) { client->addListener(this); };
	virtual ~ProtocolHandler() { client->removeListener(this); };

private:
	DCClient::Ptr client;
};

#endif // !defined(AFX_PROTOCOLHANDLER_H__1F906161_663A_4D66_BF9E_A571E67DB0F1__INCLUDED_)

/**
 * @file ProtocolHandler.h
 * $Id: ProtocolHandler.h,v 1.4 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: ProtocolHandler.h,v $
 * Revision 1.4  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.3  2001/11/24 10:39:00  arnetheduck
 * New BufferedSocket creates reader threads and reports inbound data through a listener.
 *
 * Revision 1.2  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */

