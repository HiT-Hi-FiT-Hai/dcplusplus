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
		fire(UserConnectionListener::MY_NICK, this, param);
	} else if(cmd == "$Direction") {
		x = param.find(" ");
		if(x != string::npos) {
			fire(UserConnectionListener::DIRECTION, this, param.substr(0, x), param.substr(x+1));
		}
	} else if(cmd == "$Error") {
		fire(UserConnectionListener::FAILED, this, param);
	} else if(cmd == "$FileLength") {
		fire(UserConnectionListener::FILE_LENGTH, this, param);
	} else if(cmd == "$GetListLen") {
		fire(UserConnectionListener::GET_LIST_LENGTH, this);
	} else if(cmd == "$Get") {
		x = param.find('$');
		if(x != string::npos) {
			fire(UserConnectionListener::GET, this, param.substr(0, x), Util::toInt64(param.substr(x+1)) - (int64_t)1);
		}
	} else if(cmd == "$Key") {
		fire(UserConnectionListener::KEY, this, param);
	} else if(cmd == "$Lock") {
		x = param.find(" Pk=");
		if(x != string::npos) {
			fire(UserConnectionListener::LOCK, this, param.substr(0, x), param.substr(x + 4));
		} else {
			// Workaround for faulty linux clients...
			x = param.find(' ');
			if(x != string::npos) {
				setFlag(FLAG_INVALIDKEY);
				fire(UserConnectionListener::LOCK, this, param.substr(0, x), Util::emptyString);
			} else {
				fire(UserConnectionListener::LOCK, this, param, Util::emptyString);
			}
		}
	} else if(cmd == "$Send") {
		fire(UserConnectionListener::SEND, this);
	} else if(cmd == "$MaxedOut"){
		fire(UserConnectionListener::MAXED_OUT, this);
	} else {
		dcdebug("Unknown UserConnection command: %.50s\n", aLine.c_str());
	}
}

/**
 * @file UserConnection.cpp
 * $Id: UserConnection.cpp,v 1.18 2002/04/13 12:57:23 arnetheduck Exp $
 */