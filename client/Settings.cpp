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
#include "SimpleXML.h"

#include "Settings.h"
#include "ShareManager.h"
#include "DownloadManager.h"

string Settings::nick;
string Settings::email;
string Settings::description;
string Settings::connection;
string Settings::server;
string Settings::port;
string Settings::downloadDirectory;

int Settings::connectionType;
int Settings::slots;

char* Settings::connectionSpeeds[] = { "28.8Kbps", "33.6Kbps", "56Kbps", "ISDN", "Satellite", "Cable", "DSL", "Lan(T1)", "Lan(T3)" };

void Settings::load(const string& aFileName) {
	HANDLE h = CreateFile(aFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if(h == INVALID_HANDLE_VALUE) {
		return;
	}

	DWORD fs = GetFileSize(h, NULL);
	char* buf = new char[fs+1];
	buf[fs] = 0;

	ReadFile(h, buf, fs, &fs, NULL);

	CloseHandle(h);

	string xmltext = buf;
	delete buf;
	buf = NULL;

	SimpleXML xml;
	xml.fromXML(xmltext);
	
	xml.stepIn();
	
	if(xml.findChild("User")) {
		nick = xml.getChildAttrib("Nick");
		email = xml.getChildAttrib("EMail");
		description = xml.getChildAttrib("Description");
		connection = xml.getChildAttrib("Connection");
		server = xml.getChildAttrib("Server");
		downloadDirectory = xml.getChildAttrib("DownloadDirectory");
		port = xml.getChildAttrib("Port");
		connectionType = xml.getIntChildAttrib("ConnectionType");
		slots = xml.getIntChildAttrib("Slots");
	}
	
	xml.resetCurrentChild();
	ShareManager::getInstance()->load(&xml);
	xml.resetCurrentChild();
	DownloadManager::getInstance()->load(&xml);
}

void Settings::save(const string& aFileName) {
	SimpleXML xml;

	xml.addTag("DCPlusPlus");
	xml.stepIn();

	xml.addTag("User");
	xml.addChildAttrib("Nick", nick);
	xml.addChildAttrib("EMail", email);
	xml.addChildAttrib("Description", description);
	xml.addChildAttrib("Connection", connection);
	xml.addChildAttrib("Server", server);
	xml.addChildAttrib("DownloadDirectory", downloadDirectory);
	xml.addChildAttrib("Port", port);
	xml.addChildAttrib("ConnectionType", connectionType);
	xml.addChildAttrib("Slots", slots);

	ShareManager::getInstance()->save(&xml);
	DownloadManager::getInstance()->save(&xml);

	string xmltext = xml.toXML();

	HANDLE h = CreateFile(aFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	
	DWORD fs;
	WriteFile(h, xmltext.c_str(), xmltext.length(), &fs, NULL);
	
	CloseHandle(h);
	
}
/**
 * @file Settings.cpp
 * $Id: Settings.cpp,v 1.9 2002/01/05 10:13:40 arnetheduck Exp $
 * @if LOG
 * $Log: Settings.cpp,v $
 * Revision 1.9  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.8  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.7  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.6  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.3  2001/11/22 20:42:18  arnetheduck
 * Fixed Settings dialog (Speed setting actually works now!)
 *
 * Revision 1.2  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */

