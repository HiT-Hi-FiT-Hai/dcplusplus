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
#include "SettingsManager.h"
#include "ShareManager.h"
#include "DownloadManager.h"
#include "HubManager.h"

SettingsManager* SettingsManager::instance = 0;

char const* SettingsManager::settingTags[] =
{
	// Strings
	"Connection", "Description", "DownloadDirectory", "EMail", "Nick", "Server",
	"SENTRY", 
	// Ints
	"ConnectionType", "Port", "Slots", "Rollback",
	"SENTRY"
};

const char* SettingsManager::connectionSpeeds[] = { "28.8Kbps", "33.6Kbps", "56Kbps", "ISDN", 
"Satellite", "Cable", "DSL", "LAN(T1)", "LAN(T3)" };

SettingsManager::SettingsManager()
{
	for(int i=0; i<SETTINGS_LAST; i++)
		isSet[i] = false;

	for(int j=0; j<INT_LAST-INT_FIRST; j++) {
		intDefaults[j] = 0;
		intSettings[j] = 0;
	}
	
}

void SettingsManager::load(string const& aFileName)
{
	string xmltext = ReadStringFromFile(aFileName);
	if(xmltext.empty()) {
		// Nothing to load...
		return;
	}

	SimpleXML xml;
	xml.fromXML(xmltext);

	// We load the old settings as well for now...
	oldLoad(&xml);
	xml.resetCurrentChild();
	
	xml.stepIn();
	
	if(xml.findChild("Settings"))
	{
		xml.stepIn();

		int i;
		string attr;

		for(i=STR_FIRST; i<STR_LAST; i++)
		{
			attr = settingTags[i];
			dcassert(attr != "SENTRY");

			if(xml.findChild(attr))
				set(StrSetting(i), xml.getChildData());
			xml.resetCurrentChild();
		}
		for(i=INT_FIRST; i<INT_LAST; i++)
		{
			attr = settingTags[i];
			dcassert(attr != "SENTRY");

			if(xml.findChild(attr))
				set(IntSetting(i), atoi(xml.getChildData().c_str()));
			xml.resetCurrentChild();
		}
		
		xml.stepOut();
	}
	
	xml.resetCurrentChild();
	ShareManager::getInstance()->load(&xml);
	xml.resetCurrentChild();
	DownloadManager::getInstance()->load(&xml);
	xml.resetCurrentChild();
	HubManager::getInstance()->load(&xml);
	xml.stepOut();
}

void SettingsManager::oldLoad(SimpleXML* xml)
{
	xml->stepIn();
	
	if(xml->findChild("User"))
	{
		set(NICK, xml->getChildAttrib("Nick"));
		set(EMAIL, xml->getChildAttrib("EMail"));
		set(DESCRIPTION, xml->getChildAttrib("Description"));
		set(CONNECTION, xml->getChildAttrib("Connection"));
		set(SERVER, xml->getChildAttrib("Server"));
		set(DOWNLOAD_DIRECTORY, xml->getChildAttrib("DownloadDirectory"));
		set(PORT, xml->getChildAttrib("Port"));
		set(CONNECTION_TYPE, xml->getChildAttrib("ConnectionType"));
		set(SLOTS, xml->getChildAttrib("Slots"));	
	}
	xml->stepOut();
}

void SettingsManager::save(string const& aFileName) const
{
	SimpleXML xml;
	xml.addTag("DCPlusPlus");
	xml.stepIn();
	xml.addTag("Settings");
	xml.stepIn();

	int i;
	string attr, type("type"), curType("string");
	
	for(i=STR_FIRST; i<STR_LAST; i++)
	{
		if(isSet[i])
		{
			attr = settingTags[i];
			xml.addTag(attr, get(StrSetting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}

	curType = "int";
	for(i=INT_FIRST; i<INT_LAST; i++)
	{
		if(isSet[i])
		{
			attr = settingTags[i];
			xml.addTag(attr, get(IntSetting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}
	xml.stepOut();
	
	ShareManager::getInstance()->save(&xml);
	DownloadManager::getInstance()->save(&xml);
	HubManager::getInstance()->save(&xml);

	string xmltext = xml.toXML();
	WriteStringToFile(aFileName, xmltext);
}

string SettingsManager::ReadStringFromFile(string const& aFileName)
{
	HANDLE h = ::CreateFile(aFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if(h == INVALID_HANDLE_VALUE) {
		dcdebug("Failed to open file \"%s\"", aFileName.c_str());
		return "";
	}

	DWORD fs = ::GetFileSize(h, NULL);
	char* buf = new char[fs+1];
	buf[fs] = 0;

	::ReadFile(h, buf, fs, &fs, NULL);
	::CloseHandle(h);

	string text = buf;
	delete buf;
	return text;
}

bool SettingsManager::WriteStringToFile(string const& aFileName, string const& text)
{
	HANDLE h = ::CreateFile(aFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	
	if(h != INVALID_HANDLE_VALUE)
	{
		DWORD fs;
		BOOL status = ::WriteFile(h, text.c_str(), text.length(), &fs, NULL);
		::CloseHandle(h);
		return status == TRUE;
	}
	else
		return false;
}
