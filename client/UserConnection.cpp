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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "UserConnection.h"

const string UserConnection::UPLOAD = "Upload";
const string UserConnection::DOWNLOAD = "Download";

void UserConnection::onLine(const string& aLine) throw () {

	if(aLine.length() == 0)
		return;

	string cmd;
	string param;
	int x;
	
	if( (x = aLine.find(' ')) == string::npos) {
		cmd = aLine;
	} else {
		cmd = aLine.substr(0, x);
		param = aLine.substr(x+1);
	}
	
	if(aLine.length() == 0) {
		// Do nothing
	} else if(cmd == "$MyNick") {
		string nick = aLine.substr(8);
		fire(UserConnectionListener::MY_NICK, this, nick);
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
			fire(UserConnectionListener::GET, this, param.substr(0, x), Util::toInt64(param.substr(x+1)) - 1I64);
		}
	} else if(cmd == "$Key") {
		fire(UserConnectionListener::KEY, this, param);
	} else if(cmd == "$Lock") {
		x = param.find(" Pk=");
		if(x != string::npos) {
			fire(UserConnectionListener::LOCK, this, param.substr(0, x), param.substr(x + 4));
		} else {
			fire(UserConnectionListener::LOCK, this, param, "");
		}
	} else if(cmd == "$Send") {
		fire(UserConnectionListener::SEND, this);
	} else if(cmd == "$MaxedOut"){
		fire(UserConnectionListener::MAXED_OUT, this);
	} else {
		dcdebug("Unknown UserConnection command: %s\n", aLine.c_str());
	}
}

/**
 * @file UserConnection.cpp
 * $Id: UserConnection.cpp,v 1.14 2002/03/13 20:35:26 arnetheduck Exp $
 * @if LOG
 * $Log: UserConnection.cpp,v $
 * Revision 1.14  2002/03/13 20:35:26  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.13  2002/03/07 19:07:52  arnetheduck
 * Minor fixes + started code review
 *
 * Revision 1.12  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.11  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.10  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.9  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.8  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.7  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.6  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.5  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.4  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.3  2001/11/29 19:10:55  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */