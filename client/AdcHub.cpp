/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

void Command::parse(const string& aLine, bool nmdc /* = false */) {
	string::size_type i = 5;

	if(nmdc) {
		// "$ADCxxx ..."
		if(aLine.length() < 7)
			return;
		type = Command::TYPE_CLIENT;
		memcpy(cmd, &aLine[4], 3);
		i += 3;
	} else {
		// "yxxx ..."
		if(aLine.length() < 4)
			return;
		type = aLine[0];
		memcpy(cmd, &aLine[1], 3);
	}

	string::size_type len = aLine.length();
	const char* buf = aLine.c_str();
	string cur;
	cur.reserve(128);

	bool toSet = false;
	bool fromSet = false;

	while(i < len) {
		switch(buf[i]) {
		case '\\': i++; cur += buf[i]; break;
		case ' ': 
			// New parameter...
			{
				if(type == TYPE_DIRECT && !toSet) {
					to = CID(cur);
					toSet = true;
				} else if(!fromSet && type != TYPE_CLIENT) {
					from = CID(cur);
					fromSet = true;
				} else {
					parameters.push_back(cur);
				}
				cur.clear();
			}
			break;
		default:
			cur += buf[i];
		}
		i++;
	}
	if(!cur.empty()) {
		if(!fromSet && type != TYPE_CLIENT) {
			from = CID(cur);
			fromSet = true;
		} else 	if(type == TYPE_DIRECT && !toSet) {
			to = CID(cur);
			toSet = true;
		} else {
			parameters.push_back(cur);
		}
		cur.clear();
	}
}

AdcHub::AdcHub(const string& aHubURL) : Client(aHubURL, '\n', true), state(STATE_PROTOCOL) {
}

void AdcHub::handle(Command::INF, Command& c) throw() {
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
			u->setNick(i->substr(2));
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

void AdcHub::handle(Command::SUP, Command& c) throw() {
	if(find(c.getParameters().begin(), c.getParameters().end(), "+BASE") == c.getParameters().end()) {
		disconnect();
		return;
	}
	state = STATE_IDENTIFY;
	info();
}

void AdcHub::handle(Command::MSG, Command& c) throw() {
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

void AdcHub::handle(Command::GPA, Command& c) throw() {
	if(c.getParameters().empty())
		return;
	salt = c.getParameters()[0];
	state = STATE_VERIFY;

	fire(ClientListener::GetPassword(), this);
}

void AdcHub::handle(Command::QUI, Command& c) throw() {
	if(c.getFrom().isZero())
		return;
	User::Ptr p = ClientManager::getInstance()->getUser(c.getFrom(), false);
	if(!p)
		return;
	ClientManager::getInstance()->putUserOffline(p);
	fire(ClientListener::UserRemoved(), this, p);
}

void AdcHub::handle(Command::CTM, Command& c) throw() {
	User::Ptr p = ClientManager::getInstance()->getUser(c.getFrom(), false);
	if(!p || p == getMe())
		return;
	if(c.getParameters().size() != 3 || c.getParameters()[1] != "ADC/1.0")
		return;
	//ConnectionManager::getInstance()->connect(p->getIp(), (short)Util::toInt(c.getParameters()[2]), c.getParameters()[0], getMe()->getCID());
}

void AdcHub::handle(Command::RCM, Command& c) throw() {
	if(SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE)
		return;
	User::Ptr p = ClientManager::getInstance()->getUser(c.getFrom(), false);
	if(!p || p == getMe())
		return;
	if(c.getParameters().size() != 2 || c.getParameters()[1] != "ADC/1.0")
		return;
    connect(&*p, c.getParameters()[0]);
}

void AdcHub::connect(const User* user) {
	u_int32_t r = Util::rand();
	connect(user, Encoder::toBase32((u_int8_t*)&r, sizeof(u_int32_t)));
}

void AdcHub::connect(const User* user, string const& token) {
	if(state != STATE_NORMAL)
		return;
	string tmp;
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		tmp = "DCTM " + user->getCID().toBase32() + " " +
				getMe()->getCID().toBase32() + " " + token + " ADC/1.0 " +
				Util::toString(SETTING(IN_PORT)) + "\n";
	} else {
		tmp = "DRCM " + user->getCID().toBase32() + " " +
				getMe()->getCID().toBase32() + " " + token + " ADC/1.0\n";
	}
	send(tmp);
}

void AdcHub::disconnect() {
	state = STATE_PROTOCOL;
	Client::disconnect();
}

void AdcHub::hubMessage(const string& aMessage) {
	if(state != STATE_NORMAL)
		return;
	string strtmp;
	send("BMSG " + getMe()->getCID().toBase32() + " " + Command::escape(aMessage) + "\n"); 
}

void AdcHub::privateMessage(const User* user, const string& aMessage) { 
	if(state != STATE_NORMAL)
		return;
	string strtmp;
	send("DMSG " + user->getCID().toBase32() + " " + getMe()->getCID().toBase32() + " " + Command::escape(aMessage) + " PM\n"); 
}

void AdcHub::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString) { 
	if(state != STATE_NORMAL)
		return;
	string strtmp;
	strtmp += "BSCH " + getMe()->getCID().toBase32() + " ";
	if(aSizeMode == SearchManager::SIZE_ATLEAST) {
		strtmp += ">=" + Util::toString(aSize) + " ";
	} else if(aSizeMode == SearchManager::SIZE_ATMOST) {
		strtmp += "<=" + Util::toString(aSize) + " ";
	}
	StringTokenizer<string> st(aString, ' ');
	string tmp;
	for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
		strtmp += "++" + Command::escape(*i) + " ";
	}
	strtmp[strtmp.length() - 1] = '\n';
	send(strtmp);
}

void AdcHub::password(const string& pwd) { 
	if(state != STATE_VERIFY)
		return;
	if(!salt.empty()) {
		static const int SALT_SIZE = 192/8;
		u_int8_t buf[SALT_SIZE];
		Encoder::fromBase32(salt.c_str(), buf, SALT_SIZE);
		const string& x = pwd;
		TigerHash th;
		th.update(getMe()->getCID().getData(), CID::SIZE);
		th.update(x.data(), x.length());
		th.update(buf, SALT_SIZE);
		send("HPAS " + getMe()->getCID().toBase32() + " " + Encoder::toBase32(th.finalize(), TigerHash::HASH_SIZE) + "\n");
		salt.clear();
	}
}

void AdcHub::info() {
	if(state != STATE_IDENTIFY && state != STATE_NORMAL)
		return;
	if(!getMe())
		return;

	string minf = "BINF " + getMe()->getCID().toBase32();
	unsigned size = minf.size();
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
			minf += var + tmp; \
		} \
	} else if(!tmp.empty()) { \
		minf += var + tmp; \
		lastInfoMap[var] = tmp; \
	}

	ADDPARAM(" NI", Command::escape(getNick()));
	ADDPARAM(" DE", Command::escape(getDescription()));
	ADDPARAM(" SL", Util::toString(SETTING(SLOTS)));
	ADDPARAM(" SS", ShareManager::getInstance()->getShareSizeString());
	ADDPARAM(" HN", Util::toString(counts.normal));
	ADDPARAM(" HR", Util::toString(counts.registered));
	ADDPARAM(" HO", Util::toString(counts.op));
	ADDPARAM(" VE", "++\\ " VERSIONSTRING);
	ADDPARAM(" I4", "0.0.0.0");
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		ADDPARAM(" U4", Util::toString(SETTING(IN_PORT)));
	} else {
		ADDPARAM(" U4", "");
	}

#undef ADDPARAM

	if(minf.size() != size) {
		minf += "\n";
		send(minf);
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

void AdcHub::on(Connected) throw() { 
	dcassert(state == STATE_PROTOCOL);
	setMe(ClientManager::getInstance()->getUser(CID(SETTING(CLIENT_ID)), this, false));
	lastInfoMap.clear();
	send("HSUP +BAS0\n");
	
	fire(ClientListener::Connected(), this);
}

void AdcHub::on(Failed, const string& aLine) throw() { 
	if(getMe())
		ClientManager::getInstance()->putUserOffline(getMe());
	state = STATE_PROTOCOL;
	setMe(NULL);
	state = STATE_PROTOCOL;
	fire(ClientListener::Failed(), this, aLine);
}
/**
 * @file
 * $Id: AdcHub.cpp,v 1.20 2004/10/21 10:27:15 arnetheduck Exp $
 */
