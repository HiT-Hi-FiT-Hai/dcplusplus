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
#include "SimpleXML.h"
#include "SettingsManager.h"
#include "Util.h"
#include "File.h"

SettingsManager* Singleton<SettingsManager>::instance = 0;

const string SettingsManager::settingTags[] =
{
	// Strings
	"Connection", "Description", "DownloadDirectory", "EMail", "Nick", "Server",
	"ClientVersion", "Font", "MainFrameOrder", "MainFrameWidths", "HubFrameOrder", "HubFrameWidths", 
	"LanguageFile", "SearchFrameOrder", "SearchFrameWidths", "FavoritesFrameOrder", "FavoritesFrameWidths", 
	"HublistServers", "QueueFrameOrder", "QueueFrameWidths", "PublicHubsFrameOrder", "PublicHubsFrameWidths", 
	"UsersFrameOrder", "UsersFrameWidths", "HttpProxy", "LogDirectory", "NotepadText", "LogFormatPostDownload",
	"LogFormatPostUpload", "LogFormatMainChat", "LogFormatPrivateChat", "FinishedOrder", "FinishedWidths",	
	"TempDownloadDirectory",
	"SENTRY", 
	// Ints
	"ConnectionType", "Port", "Slots", "Rollback", "AutoFollow", "ClearSearch", "FullRow",
	"BackgroundColor", "TextColor", "ShareHidden", "FilterKickMessages", "MinimizeToTray",
	"OpenPublic", "OpenQueue", "AutoSearch", "TimeStamps", "ConfirmExit", "IgnoreOffline", "PopupOffline",
	"RemoveDupes", "BufferSize", "DownloadSlots", "MaxDownloadSpeed", "LogMainChat", "LogPrivateChat",
	"LogDownloads", "LogUploads", "StatusInChat", "ShowJoins", "PrivateMessageBeep", "PrivateMessageBeepOpen",
	"UseSystemIcons", "PopupPMs", "MinUploadSpeed", "GetUserInfo", 
	"SENTRY",
	// Int64
	"TotalUpload", "TotalDownload",
	"SENTRY"
};

const string SettingsManager::connectionSpeeds[] = { "28.8Kbps", "33.6Kbps", "56Kbps", "ISDN", 
"Satellite", "Cable", "DSL", "LAN(T1)", "LAN(T3)" };

SettingsManager::SettingsManager()
{
	for(int i=0; i<SETTINGS_LAST; i++)
		isSet[i] = false;

	for(int j=0; j<INT_LAST-INT_FIRST; j++) {
		intDefaults[j] = 0;
		intSettings[j] = 0;
	}
	for(int k=0; k<INT64_LAST-INT64_FIRST; k++) {
		int64Defaults[k] = 0;
		int64Settings[k] = 0;
	}
	
	setDefault(SLOTS, 1);
	setDefault(SERVER, Util::getLocalIp());
	setDefault(PORT, 1412);
	setDefault(ROLLBACK, 4096);
	setDefault(CLIENTVERSION, "1,0091");
	setDefault(AUTO_FOLLOW, true);
	setDefault(CLEAR_SEARCH, true);
	setDefault(FULL_ROW_SELECT, true);
	setDefault(SHARE_HIDDEN, false);
	setDefault(FILTER_KICKMSGS, false);
	setDefault(MINIMIZE_TRAY, false);
	setDefault(OPEN_PUBLIC, true);
	setDefault(OPEN_QUEUE, true);
	setDefault(AUTO_SEARCH, false);
	setDefault(TIME_STAMPS, false);
	setDefault(CONFIRM_EXIT, false);
	setDefault(IGNORE_OFFLINE, false);
	setDefault(POPUP_OFFLINE, false);
	setDefault(REMOVE_DUPES, true);
	setDefault(BUFFER_SIZE, 64);
	setDefault(HUBLIST_SERVERS, "http://dcpp.lichlord.org/PublicHubList.config.bz2;http://dcplusplus.sourceforge.net/PublicHubList.config");
	setDefault(DOWNLOAD_SLOTS, 0);
	setDefault(MAX_DOWNLOAD_SPEED, 0);
	setDefault(LOG_DIRECTORY, Util::getAppPath() + "Logs\\");
	setDefault(LOG_UPLOADS, false);
	setDefault(LOG_DOWNLOADS, false);
	setDefault(LOG_PRIVATE_CHAT, false);
	setDefault(LOG_MAIN_CHAT, false);
	setDefault(STATUS_IN_CHAT, true);
	setDefault(SHOW_JOINS, false);
	setDefault(CONNECTION, connectionSpeeds[0]);
	setDefault(PRIVATE_MESSAGE_BEEP, false);
	setDefault(PRIVATE_MESSAGE_BEEP_OPEN, false);
	setDefault(USE_SYSTEM_ICONS, true);
	setDefault(POPUP_PMS, true);
	setDefault(MIN_UPLOAD_SPEED, 0);
	setDefault(LOG_FORMAT_POST_DOWNLOAD, "%Y-%m-%d %H:%M: %[target]" + STRING(DOWNLOADED_FROM) + "%[user], %[size] (%[chunksize]), %[speed], %[time]");
	setDefault(LOG_FORMAT_POST_UPLOAD, "%Y-%m-%d %H:%M: %[source]" + STRING(UPLOADED_TO) + "%[user], %[size] (%[chunksize]), %[speed], %[time]");
	setDefault(LOG_FORMAT_MAIN_CHAT, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(LOG_FORMAT_PRIVATE_CHAT, "[%Y-%m-%d %H:%M] %[message]");
	setDefault(GET_USER_INFO, true);
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
			dcassert(attr.find("SENTRY") == string::npos);

			if(xml.findChild(attr))
				set(StrSetting(i), xml.getChildData());
			xml.resetCurrentChild();
		}
		for(i=INT_FIRST; i<INT_LAST; i++)
		{
			attr = settingTags[i];
			dcassert(attr.find("SENTRY") == string::npos);

			if(xml.findChild(attr))
				set(IntSetting(i), Util::toInt(xml.getChildData()));
			xml.resetCurrentChild();
		}
		for(i=INT64_FIRST; i<INT64_LAST; i++)
		{
			attr = settingTags[i];
			dcassert(attr.find("SENTRY") == string::npos);

			if(xml.findChild(attr))
				set(Int64Setting(i), Util::toInt64(xml.getChildData()));
			xml.resetCurrentChild();
		}
		
		xml.stepOut();
	}
	
	fire(SettingsManagerListener::LOAD, &xml);
	xml.stepOut();
}

void SettingsManager::save(string const& aFileName) {
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
	curType = "int64";
	for(i=INT64_FIRST; i<INT64_LAST; i++)
	{
		if(isSet[i])
		{
			attr = settingTags[i];
			xml.addTag(attr, get(Int64Setting(i), false));
			xml.addChildAttrib(type, curType);
		}
	}
	xml.stepOut();
	
	fire(SettingsManagerListener::SAVE, &xml);

	try {
		File f(aFileName + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write("<?xml version=\"1.0\" encoding=\"windows-1252\"?>\r\n");
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
 * $Id: SettingsManager.cpp,v 1.44 2002/06/13 18:47:00 arnetheduck Exp $
 */

