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
#include "Client.h"
#include "CryptoManager.h"
#include "UserConnection.h"
#include "ConnectionManager.h"

/**
 * This is it, this is the class that controls what is sent when.
 * Add it to any Client created, or nothing will ever be sent to the
 * hub.
 */
class ProtocolHandler : public ClientListener
{
public:
	virtual void onClientLock(Client* aClient, const string& aLock, const string& aPk)  {
		aClient->key(CryptoManager::getInstance()->makeKey(aLock));
		aClient->validateNick(Settings::getNick());
	}
	virtual void onClientHello(Client* aClient, User* aUser) {
		if(aUser->getNick() == Settings::getNick()) {
			aClient->version("1,0091");
			aClient->getNickList();
			aClient->myInfo(Settings::getNick(), Settings::getDescription(), Settings::getConnection(), Settings::getEmail(), "100000000");
		} else {
			aClient->getInfo(aUser);
		}
	}
	virtual void onClientNickList(Client* aClient, StringList& aNicks) {
		for(StringIter i = aNicks.begin(); i != aNicks.end(); ++i) {
			aClient->getInfo(*i);
		}
	}

	virtual void onClientOpList(Client* aClient, StringList& aNicks) {
		for(StringIter i = aNicks.begin(); i != aNicks.end(); ++i) {
			aClient->getInfo(*i);
		}
	}

	virtual void onClientConnectToMe(Client* aClient, const string& aServer, const string& aPort) {
		ConnectionManager::getInstance()->connect(aServer, atoi(aPort.c_str()));
	}
	
	virtual void onClientRevConnectoToMe(Client* aClient, User* aUser) {
		aClient->connectToMe(aUser);
	}
	
	static ProtocolHandler* getInstance() {
		dcassert(instance);
		return instance;
	}
	static void newInstance() {
		if(instance)
			delete instance;
		
		instance = new ProtocolHandler();
	}
	static void deleteInstance() {
		delete instance;
		instance = NULL;
	}
	

private:
	ProtocolHandler() { Client::addStaticListener(this); };
	virtual ~ProtocolHandler() { Client::removeStaticListener(this); };
	
	static ProtocolHandler* instance;
};

#endif // !defined(AFX_PROTOCOLHANDLER_H__1F906161_663A_4D66_BF9E_A571E67DB0F1__INCLUDED_)

/**
 * @file ProtocolHandler.h
 * $Id: ProtocolHandler.h,v 1.6 2001/11/29 19:10:55 arnetheduck Exp $
 * @if LOG
 * $Log: ProtocolHandler.h,v $
 * Revision 1.6  2001/11/29 19:10:55  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.5  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
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

