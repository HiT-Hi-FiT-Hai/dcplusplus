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
#include "SimpleXML.h"
#include "SettingsManager.h"
#include "ShareManager.h"
#include "QueueManager.h"
#include "HubManager.h"
#include "NotepadFrame.h"
#include "Util.h"

SettingsManager* SettingsManager::instance = 0;

char const* SettingsManager::settingTags[] =
{
	// Strings
	"Connection", "Description", "DownloadDirectory", "EMail", "Nick", "Server",
	"ClientVersion", "Font", "SENTRY", 
	// Ints
	"ConnectionType", "Port", "Slots", "Rollback", "AutoFollow", "ClearSearch", "FullRow", "RemoveNotAvailable",
	"BackgroundColor", "TextColor", "ShareHidden", "RemoveFinished", "FilterKickMessages", "MinimizeToTray",
	"OpenPublic", "OpenQueue", "AutoSearch", "TimeStamps", "ConfirmExit", "IgnoreOffline", "PopupOffline"
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

	setDefault(SERVER, Util::getLocalIp());
	setDefault(PORT, 1412);
	setDefault(ROLLBACK, 1024);
	setDefault(CLIENTVERSION, "1,0091");
	setDefault(AUTO_FOLLOW, true);
	setDefault(CLEAR_SEARCH, true);
	setDefault(FULL_ROW_SELECT, false);
	setDefault(REMOVE_NOT_AVAILABLE, true);
	setDefault(BACKGROUND_COLOR, (int)(GetSysColor(COLOR_WINDOW)));
	setDefault(TEXT_COLOR, (int)(GetSysColor(COLOR_WINDOWTEXT)));
	setDefault(SHARE_HIDDEN, false);
	setDefault(REMOVE_FINISHED, true);
	setDefault(FILTER_KICKMSGS, false);
	setDefault(MINIMIZE_TRAY, false);
	setDefault(OPEN_PUBLIC, true);
	setDefault(OPEN_QUEUE, true);
	setDefault(AUTO_SEARCH, false);
	setDefault(TIME_STAMPS, false);
	setDefault(CONFIRM_EXIT, false);
	setDefault(IGNORE_OFFLINE, false);
	setDefault(POPUP_OFFLINE, false);

	LOGFONT lf;
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
	setDefault(TEXT_FONT, Util::encodeFont(lf));			
}

void SettingsManager::load(string const& aFileName)
{
	string xmltext;
	try {
		File f(aFileName, File::READ, File::OPEN);
		xmltext = f.read();		
	} catch(FileException e) {
		// ...
		return;
	}

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
				set(IntSetting(i), Util::toInt(xml.getChildData()));
			xml.resetCurrentChild();
		}
		
		xml.stepOut();
	}
	
	xml.resetCurrentChild();
	ShareManager::getInstance()->load(&xml);
	xml.resetCurrentChild();
	QueueManager::getInstance()->load(&xml);
	xml.resetCurrentChild();
	HubManager::getInstance()->load(&xml);
	xml.resetCurrentChild();
	NotepadFrame::load(&xml);
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
	QueueManager::getInstance()->save(&xml);
	HubManager::getInstance()->save(&xml);
	NotepadFrame::save(&xml);
	
	try {
		
		File f(aFileName + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write("<?xml version=\"1.0\" encoding=\"windows-1252\"?>");
		f.write(xml.toXML());
		f.close();
		File::deleteFile(aFileName);
		File::renameFile(aFileName + ".tmp", aFileName);
	} catch(FileException e) {
		// ...
	}
}

/**
 * @file SettingsManager.h
 * $Id: SettingsManager.cpp,v 1.20 2002/02/28 00:10:47 arnetheduck Exp $
 * @if LOG
 * $Log: SettingsManager.cpp,v $
 * Revision 1.20  2002/02/28 00:10:47  arnetheduck
 * Some fixes to the new user model
 *
 * Revision 1.19  2002/02/26 23:25:22  arnetheduck
 * Minor updates and fixes
 *
 * Revision 1.18  2002/02/25 15:39:29  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.17  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.16  2002/02/07 22:12:22  arnetheduck
 * Last fixes before 0.152
 *
 * Revision 1.15  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.14  2002/02/03 01:06:56  arnetheduck
 * More bugfixes and some minor changes
 *
 * Revision 1.13  2002/02/01 02:00:44  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.12  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.11  2002/01/26 16:34:01  arnetheduck
 * Colors dialog added, as well as some other options
 *
 * Revision 1.10  2002/01/26 14:59:23  arnetheduck
 * Fixed disconnect crash
 *
 * Revision 1.9  2002/01/26 12:52:51  arnetheduck
 * More minor fixes
 *
 * @endif
 */

