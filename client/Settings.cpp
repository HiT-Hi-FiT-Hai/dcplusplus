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

string Settings::nick;
string Settings::email;
string Settings::description;
string Settings::connection;

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
	}
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

	string xmltext = xml.toXML();

	HANDLE h = CreateFile(aFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	
	DWORD fs;
	WriteFile(h, xmltext.c_str(), xmltext.length(), &fs, NULL);
	
	CloseHandle(h);
	
}
/**
 * @file Settings.cpp
 * $Id: Settings.cpp,v 1.2 2001/11/22 19:47:42 arnetheduck Exp $
 * @if LOG
 * $Log: Settings.cpp,v $
 * Revision 1.2  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */

