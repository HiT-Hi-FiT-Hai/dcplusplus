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

#include "AdcHub.h"
#include "ClientManager.h"
#include "ShareManager.h"
#include "StringTokenizer.h"
#include "AdcCommand.h"
#include "ConnectionManager.h"
#include "version.h"
#include "Util.h"
const string AdcHub::CLIENT_PROTOCOL("ADC/0.9");

AdcHub::AdcHub(const string& aHubURL) : Client(aHubURL, '\n'), state(STATE_PROTOCOL) {
}

AdcHub::~AdcHub() {
	Lock l(cs);
	clearUsers();
}

void AdcHub::handle(AdcCommand::INF, AdcCommand& c) throw() {
	if(c.getFrom().isZero() || c.getParameters().empty())
		return;

	User::Ptr u = ClientManager::getInstance()->getUser(c.getFrom(), this, true);

	int op = 0;
	int reg = 0;
	int norm = 0;
	string ve;
	int sl = 0;

	for(StringIterC i = c.getParameters().begin(); i != c.getParameters().end(); ++i) {
		if(i->length() < 2)
			continue;

		if(i->compare(0, 2, "NI") == 0) {
			Lock l(cs);
			if(!u->getNick().empty()) {
				nickMap.erase(u->getNick());
			}
			u->setNick(i->substr(2));
			nickMap.insert(make_pair(u->getNick(), u));
		} else if(i->compare(0, 2, "HU") == 0) {
			hub = u;
		} else if(i->compare(0, 2, "DE") == 0) {
			u->setDescription(i->substr(2));
		} else if(i->compare(0, 2, "I4") == 0) {
			u->setIp(i->substr(2));
		} else if(i->compare(0, 2, "SS") == 0) {
			u->setBytesShared(i->substr(2));
		} else if(i->compare(0, 2, "VE") == 0) {
			ve = i->substr(2);
		} else if(i->compare(0, 2, "EM") == 0) {
			u->setEmail(i->substr(2));
		} else if(i->compare(0, 2, "OP") == 0) {
			if(i->length() == 2) {
				u->unsetFlag(User::OP);
			} else {
				u->setFlag(User::OP);
			}
		} else if(i->compare(0, 2, "HO") == 0) {
			op = Util::toInt(i->substr(2));
		} else if(i->compare(0, 2, "HR") == 0) {
			reg = Util::toInt(i->substr(2));
		} else if(i->compare(0, 2, "HN") == 0) {
			norm = Util::toInt(i->substr(2));
		} else if(i->compare(0, 2, "SL") == 0) {
			sl = Util::toInt(i->substr(2));
			u->setSlots(sl);
		} else if(i->compare(0, 2, "BO") == 0) {
			if(i->length() == 2) {
				u->unsetFlag(User::BOT);
			} else {
				u->setFlag(User::BOT);
			}
		} else if(i->compare(0, 2, "HI") == 0) {
			if(i->length() == 2) {
				u->unsetFlag(User::HIDDEN);
			} else {
				u->setFlag(User::HIDDEN);
			}
		} else if(i->compare(0, 2, "HU") == 0) {
			if(i->length() == 2) {
				u->unsetFlag(User::HUB);
			} else {
				u->setFlag(User::HUB);
			}
		} else if(i->compare(0, 2, "U4") == 0) {
			u->setUDPPort((short)Util::toInt(i->substr(2)));
		} 
	}

	if(!ve.empty()) {
		if(ve.find(' ') != string::npos) {
			ve.insert(ve.find(' ') + 1, "V:");
		}
		u->setTag("<" + ve + ",M:" + string(u->getIp().empty() ? "P" : "A") + ",H:" + Util::toString(norm) + "/" + 
			Util::toString(reg) + "/" + Util::toString(op) + ",S:" + 
			Util::toString(sl) + ">" );
	}

	if(u == getMe())
		state = STATE_NORMAL;

	fire(ClientListener::UserUpdated(), this, u);
}

void AdcHub::handle(AdcCommand::SUP, AdcCommand& c) throw() {
	if(find(c.getParameters().begin(), c.getParameters().end(), "+BASE") == c.getParameters().end()) {
		disconnect();
		return;
	}
	state = STATE_IDENTIFY;
	info(true);
}

void AdcHub::handle(AdcCommand::MSG, AdcCommand& c) throw() {
	if(c.getFrom().isZero() || c.getParameters().empty())
		return;
	User::Ptr p = ClientManager::getInstance()->getUser(c.getFrom(), false);
	if(!p)
		return;
	if(c.getParameters().size() == 2 && c.getParameters()[1] == "PM") { // add PM<group-cid> as well
		const string& msg = c.getParameters()[0];
		if(c.getFrom() == getMe()->getCID()) {
			p = ClientManager::getInstance()->getUser(c.getTo(), false);
			if(!p)
				return;
		}
		fire(ClientListener::PrivateMessage(), this, p, msg);
	} else {
		string msg = '<' + p->getNick() + "> " + c.getParameters()[0];
		fire(ClientListener::Message(), this, msg);
	}		
}

void AdcHub::handle(AdcCommand::GPA, AdcCommand& c) throw() {
	if(c.getParameters().empty())
		return;
	salt = c.getParameters()[0];
	state = STATE_VERIFY;

	fire(ClientListener::GetPassword(), this);
}

void AdcHub::handle(AdcCommand::QUI, AdcCommand& c) throw() {
	User::Ptr p = ClientManager::getInstance()->getUser(CID(c.getParam(0)), false);
	if(!p)
		return;
	if(!p->getNick().empty()) {
		Lock l(cs);
		nickMap.erase(p->getNick());
	}
	ClientManager::getInstance()->putUserOffline(p);
	fire(ClientListener::UserRemoved(), this, p);
}

void AdcHub::handle(AdcCommand::CTM, AdcCommand& c) throw() {
	User::Ptr p = ClientManager::getInstance()->getUser(c.getFrom(), false);
	if(!p || p == getMe())
		return;
	if(c.getParameters().size() < 3)
		return;

	if(c.getParam(0) != CLIENT_PROTOCOL) {
		// Protocol unhandled...
		AdcCommand cc(AdcCommand::CMD_STA, p->getCID());
		cc.addParam(Util::toString(AdcCommand::ERROR_PROTOCOL_UNSUPPORTED));
		cc.addParam(c.getParam(0));
		cc.addParam(c.getParam(1));
		cc.addParam("Protocol unsupported");
		send(cc);
		return;
	}
	string token;
	c.getParam("TO", 2, token);
	ConnectionManager::getInstance()->adcConnect(p->getIp(), (short)Util::toInt(c.getParameters()[1]), token);
}

void AdcHub::handle(AdcCommand::RCM, AdcCommand& c) throw() {
	if(SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE)
		return;
	User::Ptr p = ClientManager::getInstance()->getUser(c.getFrom(), false);
	if(!p || p == getMe())
		return;
	if(c.getParameters().empty() || c.getParameters()[0] != CLIENT_PROTOCOL)
		return;
	string token;
	c.getParam("TO", 1, token);
    connect(&*p, token);
}

void AdcHub::sendUDP(const AdcCommand& cmd) {
	try {
		Socket s;
		s.create(Socket::TYPE_UDP);

		string tmp = cmd.toString();
		for(User::NickIter i = nickMap.begin(); i != nickMap.end(); ++i) {
			if(i->second->getUDPPort() != 0 && !i->second->getIp().empty()) {
				try {
					s.writeTo(i->second->getIp(), i->second->getUDPPort(), tmp);
				} catch(const SocketException& e) {
					dcdebug("AdcHub::sendUDP: write failed: %s\n", e.getError().c_str());
				}
			}
		}
	} catch(SocketException&) {
		dcdebug("Can't create udp socket\n");
	}
}

void AdcHub::handle(AdcCommand::STA, AdcCommand& c) throw() {
	if(c.getParameters().size() < 2)
		return;

	fire(ClientListener::Message(), this, c.getParam(1));
}

void AdcHub::handle(AdcCommand::SCH, AdcCommand& c) throw() {	
	fire(ClientListener::AdcSearch(), this, c);
}

void AdcHub::connect(const User* user) {
	u_int32_t r = Util::rand();
	connect(user, Util::toString(r));
}

void AdcHub::connect(const User* user, string const& token) {
	if(state != STATE_NORMAL)
		return;

	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		send(AdcCommand(AdcCommand::CMD_CTM, user->getCID()).addParam(CLIENT_PROTOCOL).addParam(Util::toString(SETTING(IN_PORT))).addParam(token));
	} else {
		send(AdcCommand(AdcCommand::CMD_RCM, user->getCID()).addParam(CLIENT_PROTOCOL));
	}
}

void AdcHub::disconnect() {
	state = STATE_PROTOCOL;
	Client::disconnect();
	{
		Lock l(cs);
		clearUsers();
	}
}

void AdcHub::hubMessage(const string& aMessage) {
	if(state != STATE_NORMAL)
		return;
	string strtmp;
	send(AdcCommand(AdcCommand::CMD_MSG, AdcCommand::TYPE_BROADCAST).addParam(aMessage)); 
}

void AdcHub::privateMessage(const User* user, const string& aMessage) { 
	if(state != STATE_NORMAL)
		return;
	string strtmp;
	send(AdcCommand(AdcCommand::CMD_MSG, user->getCID()).addParam(aMessage).addParam("PM", SETTING(CLIENT_ID))); 
}

void AdcHub::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) { 
	if(state != STATE_NORMAL)
		return;


	AdcCommand c(AdcCommand::CMD_SCH, AdcCommand::TYPE_UDP);

	if(aFileType == SearchManager::TYPE_TTH) {
		c.addParam("TR", aString);
	} else {
		if(aSizeMode == SearchManager::SIZE_ATLEAST) {
			c.addParam(">=", Util::toString(aSize));
		} else if(aSizeMode == SearchManager::SIZE_ATMOST) {
			c.addParam("<=", Util::toString(aSize));
		}
		StringTokenizer<string> st(aString, ' ');
		for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
			c.addParam("++", *i);
		}
	}

	if(!aToken.empty())
		c.addParam("TO", aToken);

	sendUDP(c);

	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		c.setType(AdcCommand::TYPE_PASSIVE);
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
		th.update(SETTING(CLIENT_ID).c_str(), SETTING(CLIENT_ID).length());
		th.update(pwd.data(), pwd.length());
		th.update(buf, saltBytes);
		send(AdcCommand(AdcCommand::CMD_PAS, AdcCommand::TYPE_HUB).addParam(Encoder::toBase32(th.finalize(), TigerHash::HASH_SIZE)));
		salt.clear();
	}
}

void AdcHub::info(bool /*alwaysSend*/) {
	if(state != STATE_IDENTIFY && state != STATE_NORMAL)
		return;
	if(!getMe())
		return;

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

	ADDPARAM("NI", getNick());
	ADDPARAM("DE", getDescription());
	ADDPARAM("SL", Util::toString(SETTING(SLOTS)));
	ADDPARAM("SS", ShareManager::getInstance()->getShareSizeString());
	ADDPARAM("SF", Util::toString(ShareManager::getInstance()->getSharedFiles()));
	ADDPARAM("EM", SETTING(EMAIL));
	ADDPARAM("HN", Util::toString(counts.normal));
	ADDPARAM("HR", Util::toString(counts.registered));
	ADDPARAM("HO", Util::toString(counts.op));
	ADDPARAM("VE", "++ " VERSIONSTRING);
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		ADDPARAM("I4", "0.0.0.0");
		ADDPARAM("U4", Util::toString(SETTING(UDP_PORT)));
	} else {
		ADDPARAM("U4", "");
	}

#undef ADDPARAM

	if(c.getParameters().size() > 0) {
		send(c);
	}
}

string AdcHub::checkNick(const string& aNick) {
	string tmp = aNick;
	string::size_type i = 0;
	while( (i = tmp.find_first_of(" ", i)) != string::npos) {
		tmp[i++]='_';
	}
	return tmp;
}

string AdcHub::getHubURL() {
	return getAddressPort();
}

void AdcHub::clearUsers() {
	for(User::NickIter i = nickMap.begin(); i != nickMap.end(); ++i) {
		ClientManager::getInstance()->putUserOffline(i->second);		
	}
	nickMap.clear();
}

void AdcHub::on(Connected) throw() { 
	dcassert(state == STATE_PROTOCOL);
	setMe(ClientManager::getInstance()->getUser(CID(SETTING(CLIENT_ID)), this, false));
	lastInfoMap.clear();
	send(AdcCommand(AdcCommand::CMD_SUP, AdcCommand::TYPE_HUB).addParam("+BAS0"));
	
	fire(ClientListener::Connected(), this);
}

void AdcHub::on(Line, const string& aLine) throw() { 
	if(BOOLSETTING(ADC_DEBUG)) {
		fire(ClientListener::Message(), this, "<ADC>" + aLine + "</ADC>");
	}
	dispatch(aLine); 
}

void AdcHub::on(Failed, const string& aLine) throw() { 
	clearUsers();
	setMe(NULL);
	state = STATE_PROTOCOL;
	fire(ClientListener::Failed(), this, aLine);
}
/**
 * @file
 * $Id: AdcHub.cpp,v 1.41 2005/03/12 16:45:35 arnetheduck Exp $
 */
