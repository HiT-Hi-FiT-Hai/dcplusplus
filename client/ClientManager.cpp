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
#include "SimpleXML.h"
#include "UserCommand.h"
#include "ResourceManager.h"

#include "AdcHub.h"
#include "NmdcHub.h"


Client* ClientManager::getClient(const string& aHubURL) {
	Client* c;
	if(Util::strnicmp("adc://", aHubURL.c_str(), 6) == 0) {
		c = new AdcHub(aHubURL, false);
	} else if(Util::strnicmp("adcs://", aHubURL.c_str(), 7) == 0) {
		c = new AdcHub(aHubURL, true);
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
	aClient->disconnect(true);
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
	delete aClient;
}

size_t ClientManager::getUserCount() {
	Lock l(cs);
	return onlineUsers.size();
}

StringList ClientManager::getHubs(const CID& cid) {
	Lock l(cs);
	StringList lst;
	OnlinePair op = onlineUsers.equal_range(cid);
	for(OnlineIter i = op.first; i != op.second; ++i) {
		lst.push_back(i->second->getClient().getHubUrl());
	}
	return lst;
}

StringList ClientManager::getHubNames(const CID& cid) {
	Lock l(cs);
	StringList lst;
	OnlinePair op = onlineUsers.equal_range(cid);
	for(OnlineIter i = op.first; i != op.second; ++i) {
		lst.push_back(i->second->getClient().getHubName());
	}
	return lst;
}

StringList ClientManager::getNicks(const CID& cid) {
	Lock l(cs);
	StringList lst;
	OnlinePair op = onlineUsers.equal_range(cid);
	for(OnlineIter i = op.first; i != op.second; ++i) {
		lst.push_back(i->second->getIdentity().getNick());
	}
	if(lst.empty()) {
		// Offline perhaps?
		UserIter i = users.find(cid);
		if(i != users.end())
			lst.push_back(i->second->getFirstNick());
	}
	return lst;
}

string ClientManager::getConnection(const CID& cid) {
	Lock l(cs);
	OnlineIter i = onlineUsers.find(cid);
	if(i != onlineUsers.end()) {
		return i->second->getIdentity().getConnection();
	}
	return STRING(OFFLINE);
}

int64_t ClientManager::getAvailable() {
	Lock l(cs);
	int64_t bytes = 0;
	for(OnlineIter i = onlineUsers.begin(); i != onlineUsers.end(); ++i) {
		bytes += i->second->getIdentity().getBytesShared();
	}

	return bytes;
}

bool ClientManager::isConnected(const string& aUrl) {
	Lock l(cs);

	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->getHubUrl() == aUrl) {
			return true;
		}
	}
	return false;
}

string ClientManager::findHub(const string& ipPort) {
	Lock l(cs);

	string ip;
	short port = 411;
	string::size_type i = ipPort.find(':');
	if(i == string::npos) {
		ip = ipPort;
	} else {
		ip = ip.substr(0, i);
		port = (short)Util::toInt(ipPort.substr(i+1));
	}

	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		Client* c = *i;
		if(c->getPort() == port && c->getIp() == ip)
			return c->getHubUrl();
	}
	return Util::emptyString;
}

User::Ptr ClientManager::getLegacyUser(const string& aNick) throw() {
	Lock l(cs);
	dcassert(aNick.size() > 0);

	for(UserIter i = users.begin(); i != users.end(); ++i) {
		User::Ptr& p = i->second;
		if(p->isSet(User::NMDC) && Util::stricmp(p->getFirstNick(), aNick) == 0)
			return p;
	}

	LegacyIter li = legacyUsers.find(Text::toLower(aNick));
	if(li != legacyUsers.end())
		return li->second;

	return legacyUsers.insert(make_pair(Text::toLower(aNick), new User(aNick))).first->second;
}

User::Ptr ClientManager::getUser(const string& aNick, const string& aHubUrl) throw() {
	CID cid = makeCid(aNick, aHubUrl);
	Lock l(cs);

	UserIter ui = users.find(cid);
	if(ui != users.end()) {
		if(ui->second->getFirstNick().empty())		// Could happen on bad queue loads etc...
			ui->second->setFirstNick(aNick);	
		ui->second->setFlag(User::NMDC);
		return ui->second;
	}

	LegacyIter li = legacyUsers.find(Text::toLower(aNick));
	if(li != legacyUsers.end()) {
		User::Ptr p = li->second;
		p->setCID(cid);
		if(p->getFirstNick().empty())
			p->setFirstNick(aNick);
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

bool ClientManager::isOp(const User::Ptr& user, const string& aHubUrl) {
	Lock l(cs);
	pair<OnlineIter, OnlineIter> p = onlineUsers.equal_range(user->getCID());
	for(OnlineIter i = p.first; i != p.second; ++i) {
		if(i->second->getClient().getHubUrl() == aHubUrl) {
			return i->second->getClient().getMyIdentity().isOp();
		}
	}
	return false;
}

CID ClientManager::makeCid(const string& aNick, const string& aHubUrl) throw() {
	string n = Text::toLower(aNick);
	TigerHash th;
	th.update(n.c_str(), n.length());
	th.update(Text::toLower(aHubUrl).c_str(), aHubUrl.length());
	// Construct hybrid CID from the first 64 bits of the tiger hash - should be
	// fairly random, and hopefully low-collision
	return CID(th.finalize());
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
				lastUser = (distance(op.first, op.second) == 1);
				onlineUsers.erase(i);
				break;
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

void ClientManager::privateMessage(const User::Ptr& p, const string& msg) {
	Lock l(cs);
	OnlineIter i = onlineUsers.find(p->getCID());
	if(i != onlineUsers.end()) {
		OnlineUser* u = i->second;
		u->getClient().privateMessage(*u, msg);
	}
}

void ClientManager::send(AdcCommand& cmd, const CID& cid) {
	Lock l(cs);
	OnlineIter i = onlineUsers.find(cid);
	if(i != onlineUsers.end()) {
		OnlineUser* u = i->second;
		if(cmd.getType() == AdcCommand::TYPE_UDP && !u->getIdentity().isUdpActive()) {
			cmd.setType(AdcCommand::TYPE_DIRECT);
		}
		cmd.setTo(u->getSID());
		u->getClient().send(cmd);
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
	if(isPassive && !ClientManager::getInstance()->isActive()) {
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

void ClientManager::userCommand(const User::Ptr& p, const ::UserCommand& uc, StringMap& params, bool compatibility) {
	OnlineIter i = onlineUsers.find(p->getCID());
	if(i == onlineUsers.end())
		return;

	OnlineUser& ou = *i->second;
	ou.getIdentity().getParams(params, "user", compatibility);
	ou.getClient().getHubIdentity().getParams(params, "hub", false);
	ou.getClient().getMyIdentity().getParams(params, "my", compatibility);
	ou.getClient().escapeParams(params);
	ou.getClient().sendUserCmd(Util::formatParams(uc.getCommand(), params));
}

void ClientManager::on(AdcSearch, Client*, const AdcCommand& adc, const CID& from) throw() {
	SearchManager::getInstance()->respond(adc, from);
}

Identity ClientManager::getIdentity(const User::Ptr& aUser) {
	OnlineIter i = onlineUsers.find(aUser->getCID());
	if(i != onlineUsers.end()) {
		return i->second->getIdentity();
	}
	return Identity(aUser, Util::emptyString);
}

void ClientManager::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) {
	Lock l(cs);
	
	updateCachedIp(); // no point in doing a resolve for every single hub we're searching on

	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->search(aSizeMode, aSize, aFileType, aString, aToken);
		}
	}
}

void ClientManager::search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) {
	Lock l(cs);

	updateCachedIp(); // no point in doing a resolve for every single hub we're searching on

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

void ClientManager::on(Save, SimpleXML*) throw() {
	Lock l(cs);

	try {

#define CHECKESCAPE(n) SimpleXML::escape(n, tmp, true)

		File ff(getUsersFile() + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&ff);

		f.write(SimpleXML::utf8Header);
		f.write(LIT("<Users Version=\"1\">\r\n"));
		for(UserIter i = users.begin(); i != users.end(); ++i) {
			User::Ptr& p = i->second;
			if(p->isSet(User::SAVE_NICK) && !p->getCID().isZero() && !p->getFirstNick().empty()) {
				f.write(LIT("\t<User CID=\""));
				f.write(p->getCID().toBase32());
				f.write(LIT("\">\r\n\t\t<Nick>"));
				f.write(p->getFirstNick());
				f.write(LIT("</Nick>\r\n\t</User>\r\n"));
			}
		}

		f.write("</Users>\r\n");
		f.flush();
		ff.close();
		File::deleteFile(getUsersFile());
		File::renameFile(getUsersFile() + ".tmp", getUsersFile());

	} catch(const FileException&) {
		// ...
	}
}

User::Ptr& ClientManager::getMe() {
	if(!me) {
		Lock l(cs);
		if(!me) {
			me = new User(CID(SETTING(CLIENT_ID)));
			me->setFirstNick(SETTING(NICK));
		}
	}
	return me;
}

void ClientManager::on(Load, SimpleXML*) throw() {
	users.insert(make_pair(getMe()->getCID(), getMe()));

	try {
		SimpleXML xml;
		xml.fromXML(File(getUsersFile(), File::READ, File::OPEN).read());
		if(xml.findChild("Users") && xml.getChildAttrib("Version") == "1") {
			xml.stepIn();
			while(xml.findChild("User")) {
				string c = xml.getChildAttrib("CID");
				if(c.empty())
					continue;

				xml.stepIn();
				if(xml.findChild("Nick")) {
					User::Ptr p(new User(CID(c)));
					p->setFirstNick(xml.getChildData());
					users.insert(make_pair(p->getCID(), p));
				}
			}
		}
	} catch(const Exception& e) {
		dcdebug("Error loading Users.xml: %s\n", e.getError().c_str());
	}
}

void ClientManager::on(Failed, Client* client, const string&) throw() { 
	FavoriteManager::getInstance()->removeUserCommand(client->getHubUrl());
	fire(ClientManagerListener::ClientDisconnected(), client);
}

void ClientManager::on(UserCommand, Client* client, int aType, int ctx, const string& name, const string& command) throw() { 
	if(BOOLSETTING(HUB_USER_COMMANDS)) {
 		if(aType == ::UserCommand::TYPE_CLEAR) {
 			FavoriteManager::getInstance()->removeHubUserCommands(ctx, client->getHubUrl());
 		} else {
 			FavoriteManager::getInstance()->addUserCommand(aType, ctx, ::UserCommand::FLAG_NOSAVE, name, command, client->getHubUrl());
 		}
	}
}

void ClientManager::updateCachedIp() {
	// Best case - the server detected it
	if((!BOOLSETTING(NO_IP_OVERRIDE) || SETTING(EXTERNAL_IP).empty())) {
		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			if(!(*i)->getMyIdentity().getIp().empty()) {
				cachedIp = (*i)->getMyIdentity().getIp();
				return;
			}
		}
	}

	if(!SETTING(EXTERNAL_IP).empty()) {
		cachedIp = Socket::resolve(SETTING(EXTERNAL_IP));
		return;
	}

	//if we've come this far just use the first client to get the ip.
	if(clients.size() > 0)
		cachedIp = (*clients.begin())->getLocalIp();
}

/**
 * @file
 * $Id: ClientManager.cpp,v 1.90 2006/01/29 18:48:25 arnetheduck Exp $
 */
