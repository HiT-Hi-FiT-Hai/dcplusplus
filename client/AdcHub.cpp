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

AdcHub::AdcHub(const string& aHubURL) : Client(aHubURL, '\n'), adapter(this) {
}

template<> void AdcHub::handle(Command& c, Command::INF) {
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

	Speaker<AdcHubListener>::fire(AdcHubListener::COMMAND, this, c);
}

template<> void AdcHub::handle(Command& c, Command::QUI) {
	User::Ptr p = ClientManager::getInstance()->getUser(CID(c.getParameters()[0]));
	ClientManager::getInstance()->putUserOffline(p);
	Speaker<AdcHubListener>::fire(AdcHubListener::COMMAND, this, c);
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

void AdcHub::onAction(BufferedSocketListener::Types type) throw() {
	switch(type) {
			case BufferedSocketListener::CONNECTING: 
				Speaker<AdcHubListener>::fire(AdcHubListener::CONNECTING, this); break;
			case BufferedSocketListener::CONNECTED:
				setMe(ClientManager::getInstance()->getAdcMe());
				send("HSUP +BASE\n");
				Speaker<AdcHubListener>::fire(AdcHubListener::CONNECTED, this); break;
			default:
				break;
	}
}

void AdcHub::onAction(BufferedSocketListener::Types type, const string& aLine) throw() {
	switch(type) {
			case BufferedSocketListener::LINE: dispatch(aLine); break;
			case BufferedSocketListener::FAILED: 
				ClientManager::getInstance()->putUserOffline(getMe());
				setMe(NULL);
				Speaker<AdcHubListener>::fire(AdcHubListener::FAILED, this, aLine); break;
			default: break;
	}
}

void AdcHub::ClientAdapter::onAction(Types, AdcHub*, const Command& cmd) throw() { 
	switch(cmd.getCommand()) {
		case Command::CMD_MSG: c->fire(ClientListener::MESSAGE, c, "<" + cmd.getParameters()[0] + "> " + cmd.getParameters()[1]); break;
		case Command::CMD_INF: c->fire(ClientListener::USER_UPDATED, c, ClientManager::getInstance()->getUser(CID(cmd.getParameters()[0]))); break;
		case Command::CMD_QUI: c->fire(ClientListener::USER_REMOVED, c, ClientManager::getInstance()->getUser(CID(cmd.getParameters()[0]))); break;
		default: break;
	}
	string tmp = "Command: ";
	tmp += cmd.getType();
	u_int32_t x = cmd.getCommand();
	tmp += (char*)&x;
	tmp += " ";
	for(StringIterC i = cmd.getParameters().begin(); i != cmd.getParameters().end(); ++i) {
		tmp += *i + " ";
	}
	c->fire(ClientListener::MESSAGE, c, tmp + "\r\n");

};

/**
 * @file
 * $Id: AdcHub.cpp,v 1.1 2004/04/04 12:11:51 arnetheduck Exp $
 */
