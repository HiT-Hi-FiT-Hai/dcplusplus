/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

void Command::parse(const string& aLine) {
	if(aLine.length() < 4)
		return;
	type = aLine[0];
	memcpy(cmd, &aLine[1], 3);

	string::size_type i = 5;
	string::size_type k = i;

	while(k < aLine.length() && i < aLine.length()) {
		string::size_type j = aLine.find(' ', k);
		if(j == string::npos)
			j = aLine.length();
		if((j > i) && (aLine[j-1] != '\\')) {
			if(type == TYPE_DIRECT && to.isZero()) {
				to = CID(aLine.substr(i, j-i));
			} else {
				parameters.push_back(aLine.substr(i, j-i));
				string::size_type l = 0;
				string& s = parameters.back();
				while( (l = s.find('\\', l)) != string::npos) {
					s.erase(l++, 1);
				}
			}
			k = i = j + 1;
		} else {
			k = j + 1;
		}
	}
}

AdcHub::AdcHub(const string& aHubURL) : Client(aHubURL, '\n') {
}

void AdcHub::handle(Command& c, Command::INF) {
	if(c.getParameters().empty())
		return;
	User::Ptr u = ClientManager::getInstance()->getUser(CID(c.getParameters()[0]), this, true);

	int op = 0;
	int reg = 0;
	int norm = 0;
	string ve;
	int sl = 0;

	for(StringIterC i = (c.getParameters().begin()+1); i != c.getParameters().end(); ++i) {
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
			ve=i->substr(2);
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

	fire(ClientListener::UserUpdated(), this, u);
}

void AdcHub::handle(Command& c, Command::SUP) {
	if(find(c.getParameters().begin(), c.getParameters().end(), "+BASE") == c.getParameters().end()) {
		disconnect();
		return;
	}
	info();
}

void AdcHub::handle(Command& c, Command::MSG) {
	if(c.getParameters().size() < 2)
		return;
	User::Ptr p = ClientManager::getInstance()->getUser(CID(c.getParameters()[0]), false);
	if(!p)
		return;
	string msg = '<' + p->getNick() + "> " + c.getParameters()[1];
	fire(ClientListener::Message(), this, msg);
}

void AdcHub::handle(Command& c, Command::GPA) {
	if(c.getParameters().size() < 1)
		return;
	salt = c.getParameters()[0];

	fire(ClientListener::GetPassword(), this);
}

void AdcHub::handle(Command& c, Command::QUI) {
	User::Ptr p = ClientManager::getInstance()->getUser(CID(c.getParameters()[0]), false);
	if(!p)
		return;
	ClientManager::getInstance()->putUserOffline(p);
	fire(ClientListener::UserRemoved(), this, p);
}

void AdcHub::connect(const User* user) {
	string tmp = (SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) ? "DCTM " : "DRCM ";
	tmp += user->getCID().toBase32();
	tmp += " 0 NMDC/1.0\n";
	send(tmp);
}

void AdcHub::hubMessage(const string& aMessage) {
	string strtmp;
	send("BMSG " + getMe()->getCID().toBase32() + " " + Command::escape(Util::toUtf8(aMessage, strtmp)) + "\n"); 
}

void AdcHub::privateMessage(const User* user, const string& aMessage) { 
	string strtmp;
	send("DMSG " + user->getCID().toBase32() + " " + getMe()->getCID().toBase32() + " " + Command::escape(Util::toUtf8(aMessage, strtmp)) + " PM\n"); 
}

void AdcHub::kick(const User* user, const string& aMessage) { 
	string strtmp;
	send("HDSC " + user->getCID().toBase32() + " KK KK " + getMe()->getCID().toBase32() + " " + Command::escape(Util::toUtf8(aMessage, strtmp)) + "\n"); 
}
void AdcHub::ban(const User* user, const string& aMessage, time_t aSeconds) { 
	string strtmp;
	send("HDSC " + user->getCID().toBase32() + " BA BA " + getMe()->getCID().toBase32() + " " + Util::toString(aSeconds) + " " + Command::escape(Util::toUtf8(aMessage, strtmp)) + "\n"); 
}

void AdcHub::redirect(const User* user, const string& aHub, const string& aMessage) { 
	string strtmp;
	send("HDSC " + user->getCID().toBase32() + " RD RD " + getMe()->getCID().toBase32() + " " + aHub + " " + Command::escape(Util::toUtf8(aMessage, strtmp)) + "\n"); 
}
void AdcHub::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString) { 
	string strtmp;
	strtmp += "BSCH " + getMe()->getCID().toBase32();
	if(aSizeMode == SearchManager::SIZE_ATLEAST) {
		strtmp += ">=" + Util::toString(aSize);
	} else if(aSizeMode == SearchManager::SIZE_ATMOST) {
		strtmp += "<=" + Util::toString(aSize);
	}
	StringTokenizer st(aString, ' ');
	string tmp;
	for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
		strtmp += "++" + Command::escape(Util::toUtf8(*i, tmp));
	}
	strtmp += "\n";
	send(strtmp);
}
void AdcHub::password(const string& pwd) { 
	if(!salt.empty()) {
		static const int SALT_SIZE = 192/8;
		u_int8_t buf[SALT_SIZE];
		Encoder::fromBase32(salt.c_str(), buf, SALT_SIZE);
		string tmp;
		const string& x = Util::toUtf8(pwd, tmp);
		TigerHash th;
		th.update(x.data(), x.length());
		th.update(buf, SALT_SIZE);
		send("HPWD " + Encoder::toBase32(th.finalize(), TigerHash::HASH_SIZE) + "\n");
		salt.clear();
	}
}

void AdcHub::info() {
	if(!getMe())
		return;

	string tmp;

	string minf = "BINF " + getMe()->getCID().toBase32();
	minf += " NI" + Command::escape(Util::toUtf8(getNick(), tmp)); 
	minf += " DE" + Command::escape(Util::toUtf8(getDescription(), tmp));
	minf += " SL" + Util::toString(SETTING(SLOTS));
	minf += " SS" + ShareManager::getInstance()->getShareSizeString();
	minf += " HN" + Util::toString(counts.normal);
	minf += " HR" + Util::toString(counts.registered);
	minf += " HO" + Util::toString(counts.op);
	minf += " VE++\\ " VERSIONSTRING;
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		minf += " I40.0.0.0";
		minf += " U4" + Util::toString(SETTING(IN_PORT));
	}

	minf += "\n";
	if(minf != lastInfo) {
		send(minf);
		lastInfo = minf;
	}

}

void AdcHub::on(Connected) throw() { 
	setMe(ClientManager::getInstance()->getUser(CID(SETTING(CLIENT_ID)), this, false));
	lastInfo.clear();
	send("HSUP +BASE\n");
	
	fire(ClientListener::Connected(), this);
}

void AdcHub::on(Failed, const string& aLine) throw() { 
	if(getMe())
		ClientManager::getInstance()->putUserOffline(getMe());
	setMe(NULL);
	fire(ClientListener::Failed(), this, aLine);
	
}
/**
 * @file
 * $Id: AdcHub.cpp,v 1.4 2004/04/18 12:51:13 arnetheduck Exp $
 */
