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
#include "SettingsManager.h"
#include "ShareManager.h"
#include "DownloadManager.h"

SettingsManager* SettingsManager::instance = 0;

char const* SettingsManager::settingTags[] =
{
	// Strings
	"Connection", "Description", "DownloadDirectory", "EMail", "Nick", "Server",
	"SENTRY", 
	// Ints
	"ConnectionType", "Port", "Slots", 
	"SENTRY"
};

SettingsManager::SettingsManager()
{
	for(int i=0; i<SETTINGS_LAST; i++)
		isSet[i] = false;
}

void SettingsManager::setNick(const string& aNick)
{
	string &nick = strSettings[NICK - STR_FIRST];

	nick = aNick; 
	int i;
	while((i=nick.find('$')) != string::npos)
		nick.replace(i, 1, 1, '_');
	while((i=nick.find(' ')) != string::npos)
		nick.replace(i, 1, 1, '_');
	while((i=nick.find('|')) != string::npos)
		nick.replace(i, 1, 1, '_');
}

void SettingsManager::load()
{
	load(Settings::getAppPath() + "DCPlusPlus.xml");
}

void SettingsManager::save() const
{
	save(Settings::getAppPath() + "DCPlusPlus.xml");
}

void SettingsManager::load(string const& aFileName)
{
	string xmltext = ReadStringFromFile(aFileName);
	SimpleXML xml;
	xml.fromXML(xmltext);
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
		}
		for(i=INT_FIRST; i<INT_LAST; i++)
		{
			attr = settingTags[i];
			dcassert(attr != "SENTRY");

			if(xml.findChild(attr))
				set(IntSetting(i), atoi(xml.getChildData().c_str()));
		}
		
		xml.stepOut();
	}
	
	xml.resetCurrentChild();
	ShareManager::getInstance()->load(&xml);
	xml.resetCurrentChild();
	DownloadManager::getInstance()->load(&xml);
}

void SettingsManager::oldLoad()
{
	string aFileName;
	aFileName = Settings::getAppPath() + "DCPlusPlus.xml";

	string xmltext = ReadStringFromFile(aFileName);
	SimpleXML xml;
	xml.fromXML(xmltext);	
	xml.stepIn();
	
	if(xml.findChild("User"))
	{
		int i;
		string attr;

		for(i=STR_FIRST; i<STR_LAST; i++)
		{
			attr = settingTags[i];
			dcassert(attr != "SENTRY");
			set(StrSetting(i), xml.getChildAttrib(attr));
		}
		for(i=INT_FIRST; i<INT_LAST; i++)
		{
			attr = settingTags[i];
			dcassert(attr != "SENTRY");
			set(IntSetting(i), xml.getIntChildAttrib(attr));
		}
	}
	
	xml.resetCurrentChild();
	ShareManager::getInstance()->load(&xml);
	xml.resetCurrentChild();
	DownloadManager::getInstance()->load(&xml);
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
