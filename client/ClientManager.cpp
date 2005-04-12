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

#include "ClientManager.h"

#include "ShareManager.h"
#include "SearchManager.h"
#include "CryptoManager.h"
#include "ConnectionManager.h"
#include "FavoriteManager.h"

#include "AdcHub.h"
#include "NmdcHub.h"

Client* ClientManager::getClient(const string& aHubURL) {
	Client* c;
	if(Util::strnicmp("adc://", aHubURL.c_str(), 6) == 0) {
		c = new AdcHub(aHubURL);
	} else {
		c = new NmdcHub(aHubURL);
	}

	{
		Lock l(cs);
		clients.push_back(c);
	}

	c->addListener(this);
	return c;
}

void ClientManager::putClient(Client* aClient) {
	aClient->disconnect();
	fire(ClientManagerListener::ClientDisconnected(), aClient);
	aClient->removeListeners();

	{
		Lock l(cs);

		// Either I'm stupid or the msvc7 optimizer is doing something _very_ strange here...
		// STL-port -D_STL_DEBUG complains that .begin() and .end() don't have the same owner (!)
		//		dcassert(find(clients.begin(), clients.end(), aClient) != clients.end());
		//		clients.erase(find(clients.begin(), clients.end(), aClient));
		
		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			if(*i == aClient) {
				clients.erase(i);
				break;
			}
		}
	}
	aClient->scheduleDestruction();
}

User::Ptr ClientManager::getUser(const string& aNick, const string& aHubUrl) throw() {
	string n = Text::toLower(aNick);
	TigerHash th;
	th.update(n.c_str(), n.length());
	th.update(Text::toLower(aHubUrl).c_str(), aHubUrl.length());
	// Construct hybrid CID from the first 64 bits of the tiger hash - should be
	// fairly random, and hopefully low-collision
	CID cid(*(u_int64_t*)th.finalize());

	Lock l(cs);

	UserIter ui = users.find(cid);
	if(ui != users.end()) {
		return ui->second;
	}

	LegacyIter li = legacyUsers.find(n);
	if(li != legacyUsers.end()) {
		User::Ptr p = li->second;
		p->setCID(cid);
		dcassert(users.find(cid) == users.end());
		users.insert(make_pair(cid, p));
		return p;
	}

	User::Ptr p(new User(aNick));

	p->setCID(cid);
	users.insert(make_pair(cid, p));

	return p;
}

User::Ptr ClientManager::getUser(const CID& cid) throw() {
	Lock l(cs);
	UserIter ui = users.find(cid);
	if(ui != users.end()) {
		return ui->second;
	}

	User::Ptr p(new User(cid));
	users.insert(make_pair(cid, p));
	return p;
}

User::Ptr ClientManager::findUser(const CID& cid) throw() {
	Lock l(cs);
	UserIter ui = users.find(cid);
	if(ui != users.end()) {
		return ui->second;
	}
	return NULL;
}

void ClientManager::putOnline(OnlineUser& ou) throw() {
	{
		Lock l(cs);
		dcassert(!ou.getUser()->getCID().isZero());
		onlineUsers.insert(make_pair(ou.getUser()->getCID(), &ou));
	}

	if(!ou.getUser()->isOnline()) {
		ou.getUser()->setFlag(User::ONLINE);
		fire(ClientManagerListener::UserConnected(), ou.getUser());
	}
}

void ClientManager::putOffline(OnlineUser& ou) throw() {
	bool lastUser = false;
	{
		Lock l(cs);
		OnlinePair op = onlineUsers.equal_range(ou.getUser()->getCID());
		dcassert(op.first != op.second);
		for(OnlineIter i = op.first; i != op.second; ++i) {
			OnlineUser* ou2 = i->second;
			/// @todo something nicer to compare with...
			if(&ou.getClient() == &ou2->getClient()) {
				onlineUsers.erase(i);
				lastUser = (distance(op.first, op.second) == 1);
			}
		}
	}

	if(lastUser) {
		ou.getUser()->unsetFlag(User::ONLINE);
		fire(ClientManagerListener::UserDisconnected(), ou.getUser());
	}
}

void ClientManager::connect(const User::Ptr& p) {
	Lock l(cs);
	OnlineIter i = onlineUsers.find(p->getCID());
	if(i != onlineUsers.end()) {
		OnlineUser* u = i->second;
		u->getClient().connect(*u);
	}
}

void ClientManager::send(AdcCommand& cmd) {
	Lock l(cs);
	OnlineIter i = onlineUsers.find(cmd.getTo());
	if(i != onlineUsers.end()) {
		OnlineUser* u = i->second;
		if(cmd.getType() == AdcCommand::TYPE_UDP && !u->getIdentity().isUdpActive()) {
			cmd.setType(AdcCommand::TYPE_DIRECT);
		}

		if(cmd.getType() == AdcCommand::TYPE_UDP) {
			/// @todo ugly cast...
			AdcHub* h = (AdcHub*)&u->getClient();
			h->sendUDP(cmd);
		} else {
			u->getClient().send(cmd.toString());
		}
	}
}

void ClientManager::infoUpdated() {
	Lock l(cs);
	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->info(false);
		}
	}
}

void ClientManager::on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize, 
									int aFileType, const string& aString) throw() 
{
	Speaker<ClientManagerListener>::fire(ClientManagerListener::IncomingSearch(), aString);

	bool isPassive = (aSeeker.compare(0, 4, "Hub:") == 0);

	// We don't wan't to answer passive searches if we're in passive mode...
	if(isPassive && SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE) {
		return;
	}
	
	SearchResult::List l;
	ShareManager::getInstance()->search(l, aString, aSearchType, aSize, aFileType, aClient, isPassive ? 5 : 10);
//		dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
	if(l.size() > 0) {
		if(isPassive) {
			string name = aSeeker.substr(4);
			// Good, we have a passive seeker, those are easier...
			string str;
			for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
				SearchResult* sr = *i;
				str += sr->toSR(*aClient);
				str[str.length()-1] = 5;
				str += name;
				str += '|';

				sr->decRef();
			}
			
			if(str.size() > 0)
				aClient->send(str);
			
		} else {
			try {
				string ip, file;
				u_int16_t port = 0;
				Util::decodeUrl(aSeeker, ip, port, file);
				ip = Socket::resolve(ip);
				if(port == 0) port = 412;
				for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
					SearchResult* sr = *i;
					s.writeTo(ip, port, sr->toSR(*aClient));
					sr->decRef();
				}
			} catch(const SocketException& /* e */) {
				dcdebug("Search caught error\n");
			}
		}
	}
}

void ClientManager::on(AdcSearch, Client*, const AdcCommand& adc) throw() {
	SearchManager::getInstance()->respond(adc);
}

User::Ptr ClientManager::getLegacyUser(const string& aNick) {
	Lock l(cs);
	dcassert(aNick.size() > 0);

	LegacyIter i = legacyUsers.find(aNick);
	if(i != legacyUsers.end())
		return i->second;

	for(UserIter i = users.begin(); i != users.end(); ++i) {
		User::Ptr& p = i->second;
		if(p->isSet(User::NMDC) && p->getFirstNick() == aNick)
			return p;
	}

	return users.insert(make_pair(aNick, new User(aNick))).first->second;
}

void ClientManager::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) {
	Lock l(cs);

	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->search(aSizeMode, aSize, aFileType, aString, aToken);
		}
	}
}

void ClientManager::search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) {
	Lock l(cs);

	for(StringIter it = who.begin(); it != who.end(); ++it) {
		string& client = *it;
		for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
			Client* c = *j;
			if(c->isConnected() && c->getIpPort() == client) {
				c->search(aSizeMode, aSize, aFileType, aString, aToken);
			}
		}
	}
}

void ClientManager::on(TimerManagerListener::Minute, u_int32_t /* aTick */) throw() {
	Lock l(cs);

	// Collect some garbage...
	UserIter i = users.begin();
	while(i != users.end()) {
		if(i->second->unique()) {
			users.erase(i++);
		} else {
			++i;
		}
	}

	for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
		(*j)->info(false);
	}
}

void ClientManager::on(Failed, Client* client, const string&) throw() { 
	FavoriteManager::getInstance()->removeUserCommand(client->getAddressPort());
	fire(ClientManagerListener::ClientDisconnected(), client);
}

void ClientManager::on(UserCommand, Client* client, int aType, int ctx, const string& name, const string& command) throw() { 
	if(BOOLSETTING(HUB_USER_COMMANDS)) {
 		if(aType == ::UserCommand::TYPE_CLEAR) {
 			FavoriteManager::getInstance()->removeHubUserCommands(ctx, client->getAddressPort());
 		} else {
 			FavoriteManager::getInstance()->addUserCommand(aType, ctx, ::UserCommand::FLAG_NOSAVE, name, command, client->getAddressPort());
 		}
	}
}

/**
 * @file
 * $Id: ClientManager.cpp,v 1.68 2005/04/12 23:24:11 arnetheduck Exp $
 */
