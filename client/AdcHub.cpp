/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#include "AdcHub.h"
#include "ClientManager.h"
#include "ShareManager.h"
#include "StringTokenizer.h"
#include "AdcCommand.h"
#include "ConnectionManager.h"
#include "version.h"
#include "Util.h"
#include "UserCommand.h"
#include "FavoriteManager.h"
#include "SSLSocket.h"

const string AdcHub::CLIENT_PROTOCOL("ADC/0.10");
const string AdcHub::SECURE_CLIENT_PROTOCOL("ADCS/0.10");
const string AdcHub::ADCS_FEATURE("ADC0");
const string AdcHub::TCP4_FEATURE("TCP4");
const string AdcHub::UDP4_FEATURE("UDP4");

AdcHub::AdcHub(const string& aHubURL, bool secure) : Client(aHubURL, '\n', secure), state(STATE_PROTOCOL), sid(0), reconnect(true) {
	TimerManager::getInstance()->addListener(this);
}

AdcHub::~AdcHub() throw() {
	TimerManager::getInstance()->removeListener(this);
	clearUsers();
}

OnlineUser& AdcHub::getUser(const u_int32_t aSID, const CID& aCID) {
	OnlineUser* u = NULL;
	{
		Lock l(cs);

		SIDIter i = users.find(aSID);
		if(i != users.end())
			return *i->second;
	}


	User::Ptr p = ClientManager::getInstance()->getUser(aCID);

	{
		Lock l(cs);
		u = users.insert(make_pair(aSID, new OnlineUser(p, *this, aSID))).first->second;
	}

	if(!aCID.isZero())
		ClientManager::getInstance()->putOnline(*u);
	return *u;
}

OnlineUser* AdcHub::findUser(const u_int32_t aSID) {
	Lock l(cs);
	SIDIter i = users.find(aSID);
	return i == users.end() ? NULL : i->second;
}

void AdcHub::putUser(const u_int32_t aSID) {
	Lock l(cs);
	SIDIter i = users.find(aSID);
	if(i == users.end())
		return;
	if(aSID != AdcCommand::HUB_SID)
		ClientManager::getInstance()->putOffline(*i->second);
	fire(ClientListener::UserRemoved(), this, *i->second);
	delete i->second;
	users.erase(i);
}

void AdcHub::clearUsers() {
	Lock l(cs);
	for(SIDIter i = users.begin(); i != users.end(); ++i) {
		if(i->first != AdcCommand::HUB_SID)
			ClientManager::getInstance()->putOffline(*i->second);
		delete i->second;
	}
	users.clear();
}


void AdcHub::handle(AdcCommand::INF, AdcCommand& c) throw() {
	if(c.getParameters().empty())
		return;

	string cid;
	
	OnlineUser* u = 0;
	if(c.getParam("ID", 0, cid))
		u = &getUser(c.getFrom(), CID(cid));
	else if(c.getFrom() == AdcCommand::HUB_SID)
		u = &getUser(c.getFrom(), CID());
	else
		u = findUser(c.getFrom());

	if(!u) {
		dcdebug("AdcHub::INF Unknown user / no ID\n");
		return;
	}

	for(StringIterC i = c.getParameters().begin(); i != c.getParameters().end(); ++i) {
		if(i->length() < 2)
			continue;
			
		u->getIdentity().set(i->c_str(), i->substr(2));
	}

	if(u->getUser()->getFirstNick().empty()) {
		u->getUser()->setFirstNick(u->getIdentity().getNick());
	}

	if(u->getIdentity().supports(ADCS_FEATURE)) {
		u->getUser()->setFlag(User::SSL);
	}

	if(u->getIdentity().isHub()) {
		setHubIdentity(u->getIdentity());
		fire(ClientListener::HubUpdated(), this);
	}

	if(u->getUser() == ClientManager::getInstance()->getMe()) {
		state = STATE_NORMAL;
		setMyIdentity(u->getIdentity());
	}
	fire(ClientListener::UserUpdated(), this, *u);
}

void AdcHub::handle(AdcCommand::SUP, AdcCommand& c) throw() {
	if(state != STATE_PROTOCOL) /** @todo SUP changes */
		return;
	if(find(c.getParameters().begin(), c.getParameters().end(), "ADBASE") == c.getParameters().end()
		&& find(c.getParameters().begin(), c.getParameters().end(), "ADBAS0") == c.getParameters().end())
	{
		fire(ClientListener::StatusMessage(), this, "Failed to negotiate base protocol"); // @todo internationalize
		socket->disconnect(false);
		return;
	}
}

void AdcHub::handle(AdcCommand::SID, AdcCommand& c) throw() {
	if(state != STATE_PROTOCOL) {
		dcdebug("Invalid state for SID\n");
		return;
	}

	if(c.getParameters().empty())
		return;

	sid = AdcCommand::toSID(c.getParam(0));

	state = STATE_IDENTIFY;
	info(true);
}

void AdcHub::handle(AdcCommand::MSG, AdcCommand& c) throw() {
	if(c.getParameters().empty())
		return;

	OnlineUser* from = findUser(c.getFrom());
	if(!from)
		return;

	string pmFrom;
	if(c.getParam("PM", 1, pmFrom)) { // add PM<group-cid> as well
		OnlineUser* to = findUser(c.getTo());
		if(!to)
			return;

		OnlineUser* replyTo = findUser(AdcCommand::toSID(pmFrom));
		if(!replyTo)
			return;

		fire(ClientListener::PrivateMessage(), this, *from, *to, *replyTo, c.getParam(0));
	} else {
		fire(ClientListener::Message(), this, *from, c.getParam(0));
	}		
}

void AdcHub::handle(AdcCommand::GPA, AdcCommand& c) throw() {
	if(c.getParameters().empty())
		return;
	salt = c.getParam(0);
	state = STATE_VERIFY;

	fire(ClientListener::GetPassword(), this);
}

void AdcHub::handle(AdcCommand::QUI, AdcCommand& c) throw() {
	putUser(AdcCommand::toSID(c.getParam(0)));
}

void AdcHub::handle(AdcCommand::CTM, AdcCommand& c) throw() {
	OnlineUser* u = findUser(c.getFrom());
	if(!u || u->getUser() == ClientManager::getInstance()->getMe())
		return;
	if(c.getParameters().size() < 3)
		return;

	bool secure;
	if(c.getParam(0) == CLIENT_PROTOCOL) {
		secure = false;
	} else if(c.getParam(0) == SECURE_CLIENT_PROTOCOL) {
		secure = true;
	} else {
		send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_UNSUPPORTED, "Protocol unknown", AdcCommand::TYPE_DIRECT).setTo(c.getFrom()));
		return;
	}

	if(!u->getIdentity().isTcpActive()) {
		send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC, "IP unknown", AdcCommand::TYPE_DIRECT).setTo(c.getFrom()));
		return;
	}

	string token;
	c.getParam("TO", 2, token);
	ConnectionManager::getInstance()->adcConnect(*u, (short)Util::toInt(c.getParameters()[1]), token, secure);
}

void AdcHub::handle(AdcCommand::RCM, AdcCommand& c) throw() {
	if(!ClientManager::getInstance()->isActive())
		return;
	OnlineUser* u = findUser(c.getFrom());
	if(!u || u->getUser() == ClientManager::getInstance()->getMe())
		return;
	if(c.getParameters().empty() || (c.getParameters()[0] != CLIENT_PROTOCOL && c.getParameters()[0] != SECURE_CLIENT_PROTOCOL))
		return;
	string token;
	c.getParam("TO", 1, token);
    connect(*u, token, c.getParameters()[0] == SECURE_CLIENT_PROTOCOL);
}

void AdcHub::handle(AdcCommand::CMD, AdcCommand& c) throw() {
	if(c.getParameters().size() < 1)
		return;
	const string& name = c.getParam(0);
	bool rem = c.hasFlag("RM", 1);
	if(rem) {
		int cmd = FavoriteManager::getInstance()->findUserCommand(name);
		if(cmd != -1)
			FavoriteManager::getInstance()->removeUserCommand(cmd);
	}
	bool sep = c.hasFlag("SP", 1);
	string sctx;
	if(!c.getParam("CT", 1, sctx))
		return;
	int ctx = Util::toInt(sctx);
	if(ctx <= 0)
		return;
	if(sep) {
		FavoriteManager::getInstance()->addUserCommand(UserCommand::TYPE_SEPARATOR, ctx, UserCommand::FLAG_NOSAVE, name, "", getHubUrl());
		return;
	}
	bool once = c.hasFlag("CO", 1);
	string txt;
	if(!c.getParam("TT", 1, txt))
		return;
	FavoriteManager::getInstance()->addUserCommand(once ? UserCommand::TYPE_RAW_ONCE : UserCommand::TYPE_RAW, ctx, UserCommand::FLAG_NOSAVE, name, txt, getHubUrl());
}

void AdcHub::sendUDP(const AdcCommand& cmd) {
	try {
		Socket s;
		s.create(Socket::TYPE_UDP);

		Lock l(cs);
		SIDMap::const_iterator i = users.find(cmd.getTo());
		if(i == users.end()) {
			dcdebug("AdcHub::sendUDP: invalid user\n");
			return;
		}
		OnlineUser& u = *i->second;
		string tmp = cmd.toString(u.getUser()->getCID());
		if(u.getIdentity().isUdpActive()) {
			try {
				s.writeTo(u.getIdentity().getIp(), (short)Util::toInt(u.getIdentity().getUdpPort()), tmp);
			} catch(const SocketException& e) {
				dcdebug("AdcHub::sendUDP: write failed: %s\n", e.getError().c_str());
			}
		}
	} catch(SocketException&) {
		dcdebug("Can't create UDP socket\n");
	}
}

void AdcHub::handle(AdcCommand::STA, AdcCommand& c) throw() {
	if(c.getParameters().size() < 2)
		return;

	OnlineUser* u = findUser(c.getFrom());
	if(!u)
		return;

	// @todo Check for invalid protocol and unset SSL if necessary
	fire(ClientListener::Message(), this, *u, c.getParam(1));
}

void AdcHub::handle(AdcCommand::SCH, AdcCommand& c) throw() {
	SIDMap::const_iterator i = users.find(c.getFrom());
	if(i == users.end()) {
		dcdebug("Invalid user in AdcHub::onSCH\n");
		return;
	}

	fire(ClientListener::AdcSearch(), this, c, i->second->getUser()->getCID());
}

void AdcHub::connect(const OnlineUser& user) {
	u_int32_t r = Util::rand();
	connect(user, Util::toString(r), BOOLSETTING(USE_SSL) && user.getUser()->isSet(User::SSL));
}

void AdcHub::connect(const OnlineUser& user, string const& token, bool secure) {
	if(state != STATE_NORMAL)
		return;

	const string& proto = secure ? SECURE_CLIENT_PROTOCOL : CLIENT_PROTOCOL;
	short port = secure ? ConnectionManager::getInstance()->getSecurePort() : ConnectionManager::getInstance()->getPort();

	if(ClientManager::getInstance()->isActive()) {
		send(AdcCommand(AdcCommand::CMD_CTM, user.getSID()).addParam(proto).addParam(Util::toString(port)).addParam(token));
	} else {
		send(AdcCommand(AdcCommand::CMD_RCM, user.getSID()).addParam(proto));
	}
}

void AdcHub::disconnect(bool graceless) {
	Client::disconnect(graceless);
	state = STATE_PROTOCOL;
	clearUsers();
}

void AdcHub::hubMessage(const string& aMessage) {
	if(state != STATE_NORMAL)
		return;
	send(AdcCommand(AdcCommand::CMD_MSG, AdcCommand::TYPE_BROADCAST).addParam(aMessage)); 
}

void AdcHub::privateMessage(const OnlineUser& user, const string& aMessage) { 
	if(state != STATE_NORMAL)
		return;
	send(AdcCommand(AdcCommand::CMD_MSG, user.getSID()).addParam(aMessage).addParam("PM", getMySID())); 
}

void AdcHub::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) { 
	if(state != STATE_NORMAL)
		return;

	AdcCommand c(AdcCommand::CMD_SCH, AdcCommand::TYPE_BROADCAST);

	if(aFileType == SearchManager::TYPE_TTH) {
		c.addParam("TR", aString);
	} else {
		if(aSizeMode == SearchManager::SIZE_ATLEAST) {
			c.addParam("GE", Util::toString(aSize));
		} else if(aSizeMode == SearchManager::SIZE_ATMOST) {
			c.addParam("LE", Util::toString(aSize));
		}
		StringTokenizer<string> st(aString, ' ');
		for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
			c.addParam("AN", *i);
		}
		if(aFileType == SearchManager::TYPE_DIRECTORY) {
			c.addParam("TY", "2");
		}
	}

	if(!aToken.empty())
		c.addParam("TO", aToken);

	if(ClientManager::getInstance()->isActive()) {
		send(c);
	} else {
		c.setType(AdcCommand::TYPE_FEATURE);
		c.setFeatures("+TCP4");
		send(c);
	}
}

void AdcHub::password(const string& pwd) { 
	if(state != STATE_VERIFY)
		return;
	if(!salt.empty()) {
		size_t saltBytes = salt.size() * 5 / 8;
		AutoArray<u_int8_t> buf(saltBytes);
		Encoder::fromBase32(salt.c_str(), buf, saltBytes);
		TigerHash th;
		string cid = getMyIdentity().getUser()->getCID().toBase32();
		th.update(cid.c_str(), cid.length());
		th.update(pwd.data(), pwd.length());
		th.update(buf, saltBytes);
		send(AdcCommand(AdcCommand::CMD_PAS, AdcCommand::TYPE_HUB).addParam(Encoder::toBase32(th.finalize(), TigerHash::HASH_SIZE)));
		salt.clear();
	}
}

void AdcHub::info(bool /*alwaysSend*/) {
	if(state != STATE_IDENTIFY && state != STATE_NORMAL)
		return;

	reloadSettings(false);

	AdcCommand c(AdcCommand::CMD_INF, AdcCommand::TYPE_BROADCAST);
	string tmp;

	StringMapIter i;
#define ADDPARAM(var, content) \
	tmp = content; \
	if((i = lastInfoMap.find(var)) != lastInfoMap.end()) { \
		if(i->second != tmp) { \
			if(tmp.empty()) \
				lastInfoMap.erase(i); \
			else \
				i->second = tmp; \
			c.addParam(var, tmp); \
		} \
	} else if(!tmp.empty()) { \
		c.addParam(var, tmp); \
		lastInfoMap[var] = tmp; \
	}

	updateCounts(false); \

	ADDPARAM("ID", ClientManager::getInstance()->getMyCID().toBase32());
	ADDPARAM("PD", ClientManager::getInstance()->getMyPID().toBase32());
	ADDPARAM("NI", getMyIdentity().getNick());
	ADDPARAM("DE", getMyIdentity().getDescription());
	ADDPARAM("SL", Util::toString(SETTING(SLOTS)));
	ADDPARAM("SS", ShareManager::getInstance()->getShareSizeString());
	ADDPARAM("SF", Util::toString(ShareManager::getInstance()->getSharedFiles()));
	ADDPARAM("EM", SETTING(EMAIL));
	ADDPARAM("HN", Util::toString(counts.normal));
	ADDPARAM("HR", Util::toString(counts.registered));
	ADDPARAM("HO", Util::toString(counts.op));
	ADDPARAM("VE", "++ " VERSIONSTRING);

	string su;
	if(SSLSocketFactory::getInstance()->hasCerts()) {
		su += ADCS_FEATURE + ",";
	} 
	
	if(ClientManager::getInstance()->isActive()) {
		if(BOOLSETTING(NO_IP_OVERRIDE) && !SETTING(EXTERNAL_IP).empty()) {
			ADDPARAM("I4", Socket::resolve(SETTING(EXTERNAL_IP)));
		} else {
			ADDPARAM("I4", "0.0.0.0");
		}
		ADDPARAM("U4", Util::toString(SearchManager::getInstance()->getPort()));
		su += TCP4_FEATURE + ",";
		su += UDP4_FEATURE + ",";
	} else {
		ADDPARAM("I4", "");
		ADDPARAM("U4", "");
	}

	if(!su.empty()) {
		su.erase(su.size() - 1);
	}
	ADDPARAM("SU", su);

#undef ADDPARAM

	if(c.getParameters().size() > 0) {
		send(c);
	}
}

int64_t AdcHub::getAvailable() const {
	Lock l(cs);
	int64_t x = 0;
	for(SIDMap::const_iterator i = users.begin(); i != users.end(); ++i) {
		x+=i->second->getIdentity().getBytesShared();
	}
	return x;
}

string AdcHub::checkNick(const string& aNick) {
	string tmp = aNick;
	string::size_type i = 0;
	while( (i = tmp.find_first_of(" ", i)) != string::npos) {
		tmp[i++]='_';
	}
	return tmp;
}

void AdcHub::on(Connected) throw() { 
	dcassert(state == STATE_PROTOCOL);
	lastInfoMap.clear();
	reconnect = true;
	send(AdcCommand(AdcCommand::CMD_SUP, AdcCommand::TYPE_HUB).addParam("ADBAS0"));
	
	fire(ClientListener::Connected(), this);
}

void AdcHub::on(Line, const string& aLine) throw() { 
	if(BOOLSETTING(ADC_DEBUG)) {
		fire(ClientListener::StatusMessage(), this, "<ADC>" + aLine + "</ADC>");
	}
	dispatch(aLine); 
}

void AdcHub::on(Failed, const string& aLine) throw() { 
	clearUsers();
	state = STATE_PROTOCOL;
	fire(ClientListener::Failed(), this, aLine);
}

void AdcHub::on(TimerManagerListener::Second, u_int32_t aTick) throw() {
	if(socket && (getLastActivity() + getReconnDelay() * 1000) < aTick) {
		// Nothing's happened for ~120 seconds, check if we're connected, if not, try to connect...
		if(!isConnected()) {
			// Try to reconnect...
			if(reconnect && !getAddress().empty())
				Client::connect();
		}
	}
}

void AdcHub::send(const AdcCommand& cmd) {
	dcassert(socket);
	if(!socket)
		return;
	if(cmd.getType() == AdcCommand::TYPE_UDP)
		sendUDP(cmd);
	send(cmd.toString(sid));
}
