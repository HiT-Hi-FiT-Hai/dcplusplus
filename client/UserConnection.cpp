/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

#include "UserConnection.h"
#include "StringTokenizer.h"

const string UserConnection::UPLOAD = "Upload";
const string UserConnection::DOWNLOAD = "Download";

void UserConnection::onLine(const string& aLine) throw () {

	if(aLine.length() == 0)
		return;

	string cmd;
	string param;

	string::size_type x;
	
	if( (x = aLine.find(' ')) == string::npos) {
		cmd = aLine;
	} else {
		cmd = aLine.substr(0, x);
		param = aLine.substr(x+1);
	}
	
	if(aLine.length() == 0) {
		// Do nothing
	} else if(cmd == "$MyNick") {
		if(!param.empty())
			fire(UserConnectionListener::MY_NICK, this, param);
	} else if(cmd == "$Direction") {
		x = param.find(" ");
		if(x != string::npos) {
			fire(UserConnectionListener::DIRECTION, this, param.substr(0, x), param.substr(x+1));
		}
	} else if(cmd == "$Error") {
		fire(UserConnectionListener::FAILED, this, param);
	} else if(cmd == "$FileLength") {
		if(!param.empty())
		fire(UserConnectionListener::FILE_LENGTH, this, param);
	} else if(cmd == "$GetListLen") {
		fire(UserConnectionListener::GET_LIST_LENGTH, this);
	} else if(cmd == "$Get") {
		x = param.find('$');
		if(x != string::npos) {
			fire(UserConnectionListener::GET, this, param.substr(0, x), Util::toInt64(param.substr(x+1)) - (int64_t)1);
		}
	} else if(cmd == "$Key") {
		if(!param.empty())
			fire(UserConnectionListener::KEY, this, param);
	} else if(cmd == "$Lock") {
		if(!param.empty()) {
			x = param.find(" Pk=");
			if(x != string::npos) {
				fire(UserConnectionListener::C_LOCK, this, param.substr(0, x), param.substr(x + 4));
			} else {
				// Workaround for faulty linux clients...
				x = param.find(' ');
				if(x != string::npos) {
					setFlag(FLAG_INVALIDKEY);
					fire(UserConnectionListener::C_LOCK, this, param.substr(0, x), Util::emptyString);
				} else {
					fire(UserConnectionListener::C_LOCK, this, param, Util::emptyString);
				}
			}
		}
	} else if(cmd == "$Send") {
		fire(UserConnectionListener::SEND, this);
	} else if(cmd == "$MaxedOut") {
		fire(UserConnectionListener::MAXED_OUT, this);
	} else if(cmd == "$Supports") {
		if(!param.empty()) {
			StringTokenizer t(param, ' ');
			fire(UserConnectionListener::SUPPORTS, this, t.getTokens());
		}
	} else {
		dcdebug("Unknown UserConnection command: %.50s\n", aLine.c_str());
	}
}

// BufferedSocketListener
void UserConnection::onAction(BufferedSocketListener::Types type) {
	lastActivity = GET_TICK();
	switch(type) {
	case BufferedSocketListener::CONNECTED:
		fire(UserConnectionListener::CONNECTED, this);
		break;
	case BufferedSocketListener::TRANSMIT_DONE:
		fire(UserConnectionListener::TRANSMIT_DONE, this); break;
	}
}
void UserConnection::onAction(BufferedSocketListener::Types type, u_int32_t bytes) {
	lastActivity = GET_TICK();
	switch(type) {
	case BufferedSocketListener::BYTES_SENT:
		fire(UserConnectionListener::BYTES_SENT, this, bytes); break;
	default:
		dcassert(0);
	}
}
void UserConnection::onAction(BufferedSocketListener::Types type, const string& aLine) {
	lastActivity = GET_TICK();
	switch(type) {
	case BufferedSocketListener::LINE:
		onLine(aLine); break;
	case BufferedSocketListener::FAILED:
		setState(STATE_UNCONNECTED);
		fire(UserConnectionListener::FAILED, this, aLine); break;
	default:
		dcassert(0);
	}
}
void UserConnection::onAction(BufferedSocketListener::Types type, int mode) {
	lastActivity = GET_TICK();
	switch(type) {
	case BufferedSocketListener::MODE_CHANGE:
		fire(UserConnectionListener::MODE_CHANGE, this, mode); break;
	default:
		dcassert(0);
	}
}
void UserConnection::onAction(BufferedSocketListener::Types type, const u_int8_t* buf, int len) {
	lastActivity = GET_TICK();
	switch(type) {
	case BufferedSocketListener::DATA:
		fire(UserConnectionListener::DATA, this, buf, len); break;
	default:
		dcassert(0);
	}
}

/**
 * @file UserConnection.cpp
 * $Id: UserConnection.cpp,v 1.22 2002/05/26 20:28:11 arnetheduck Exp $
 */