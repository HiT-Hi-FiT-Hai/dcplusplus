/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "UserConnection.h"

#include "ServerSocket.h"

void UserConnection::connect(const string& aServer, short aPort /* = 412 */) {
	if(socket.isConnected())
		disconnect();

	socket.connect(aServer, aPort);
}

void UserConnection::accept(const ServerSocket& aServer) {
	if(socket.isConnected())
		disconnect();

	socket.accept(aServer);
}
void UserConnection::onLine(const string& aLine) {
	if(aLine.length() == 0) {
		// Do nothing
	} else if(aLine.find("$MyNick") != string::npos) {
		nick = aLine.substr(8);
		fireMyNick(nick);
	} else if(aLine.find("$Direction") != string::npos) {
		string tmp = aLine.substr(11);
		string dir = tmp.substr(0, tmp.find(" "));
		tmp = tmp.substr(tmp.find(" ")+1);
		fireDirection(dir, tmp);
	} else if(aLine.find("$Error") != string::npos) {
		fireError(aLine.substr(6));
	} else if(aLine.find("$FileLength") != string::npos) {
		fireFileLength(aLine.substr(12));
	} else if(aLine.find("$Get") != string::npos) {
		string tmp = aLine.substr(5);
		string file = tmp.substr(0, tmp.find('$'));

		fireGet(file, _atoi64(tmp.substr(tmp.find('$')+1).c_str()));
	} else if(aLine.find("$Key") != string::npos) {
		fireKey(aLine.substr(5));
	} else if(aLine.find("$Lock") != string::npos) {
		string tmp = aLine.substr(6);
		string lock = tmp.substr(0, tmp.find(" Pk="));
		tmp = tmp.substr(tmp.find(" Pk=") + 5);
		fireLock(lock, tmp);
	} else if(aLine.find("$MyNick") != string::npos) {
		fireMyNick(aLine.substr(8));
	} else if(aLine.find("$Send")) {
		fireSend();
	} else {
		dcdebug("Unknown UserConnection command: %s\n", aLine.c_str());
	}
}

void UserConnection::waitForConnection(short aPort /* = 412 */) {
	
}
/**
 * @file UserConnection.cpp
 * $Id: UserConnection.cpp,v 1.1 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: UserConnection.cpp,v $
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */