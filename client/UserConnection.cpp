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

#include "UserConnection.h"

#include "StringTokenizer.h"

const string UserConnection::UPLOAD = "Upload";
const string UserConnection::DOWNLOAD = "Download";

void Transfer::updateRunningAverage() {
	u_int32_t tick = GET_TICK();
	if(tick > lastTick) {
		u_int32_t diff = tick - lastTick;
		if(diff == 0) {
			// No time passed, don't update runningAverage;
		} else if( ((tick - getStart()) < AVG_PERIOD) ) {
			runningAverage = getAverageSpeed();
		} else {
			int64_t bdiff = total - last;
			int64_t avg = bdiff * (int64_t)1000 / diff;
			if(diff > AVG_PERIOD) {
				runningAverage = avg;
			} else {
				// Weighted average...
				runningAverage = ((avg * diff) + (runningAverage*(AVG_PERIOD-diff)))/AVG_PERIOD;
			}
		}
		last = total;
	}
	lastTick = tick;
}


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
		if(Util::stricmp(param.c_str(), "File Not Available") == 0) {
			fire(UserConnectionListener::FILE_NOT_AVAILABLE, this);
		} else {
			fire(UserConnectionListener::FAILED, this, param);
		}
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
	} else if(cmd == "$GetTestZBlock") {
		string::size_type i = param.find(' ');
		if(i == string::npos)
			return;
		int64_t start = Util::toInt64(param.substr(0, i));
		i++;
		string::size_type j = param.find(' ', i);
		if(j == string::npos)
			return;
		int64_t bytes = Util::toInt64(param.substr(i, j-i));
		fire(UserConnectionListener::GET_ZBLOCK, this, param.substr(j+1), start, bytes);
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
	} else if(cmd == "$Sending") {
		fire(UserConnectionListener::SENDING, this);
	} else if(cmd == "$MaxedOut") {
		fire(UserConnectionListener::MAXED_OUT, this);
	} else if(cmd == "$Supports") {
		if(!param.empty()) {
			fire(UserConnectionListener::SUPPORTS, this, StringTokenizer(param, ' ').getTokens());
		}
	} else {
		dcdebug("Unknown UserConnection command: %.50s\n", aLine.c_str());
	}
}

// BufferedSocketListener
void UserConnection::onAction(BufferedSocketListener::Types type) throw() {
	lastActivity = GET_TICK();
	switch(type) {
	case BufferedSocketListener::CONNECTED:
		fire(UserConnectionListener::CONNECTED, this);
		break;
	case BufferedSocketListener::TRANSMIT_DONE:
		fire(UserConnectionListener::TRANSMIT_DONE, this); break;
	}
}
void UserConnection::onAction(BufferedSocketListener::Types type, u_int32_t bytes) throw() {
	lastActivity = GET_TICK();
	switch(type) {
	case BufferedSocketListener::BYTES_SENT:
		fire(UserConnectionListener::BYTES_SENT, this, bytes); break;
	default:
		dcassert(0);
	}
}
void UserConnection::onAction(BufferedSocketListener::Types type, const string& aLine) throw() {
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
void UserConnection::onAction(BufferedSocketListener::Types type, int mode) throw() {
	lastActivity = GET_TICK();
	switch(type) {
	case BufferedSocketListener::MODE_CHANGE:
		fire(UserConnectionListener::MODE_CHANGE, this, mode); break;
	default:
		dcassert(0);
	}
}
void UserConnection::onAction(BufferedSocketListener::Types type, const u_int8_t* buf, int len) throw() {
	lastActivity = GET_TICK();
	switch(type) {
	case BufferedSocketListener::DATA:
		fire(UserConnectionListener::DATA, this, buf, len); break;
	default:
		dcassert(0);
	}
}

/**
 * @file
 * $Id: UserConnection.cpp,v 1.30 2003/11/10 22:42:12 arnetheduck Exp $
 */
