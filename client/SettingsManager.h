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

#if !defined(SETTINGSMANAGER_H)
#define SETTINGSMANAGER_H

#include "stdafx.h"
#include "Util.h"

class SimpleXML;

class SettingsManager : public Singleton<SettingsManager>
{
public:

	static const string connectionSpeeds[];

	enum StrSetting { STR_FIRST,
		CONNECTION = STR_FIRST, DESCRIPTION, DOWNLOAD_DIRECTORY, EMAIL, NICK, SERVER,
		CLIENTVERSION, TEXT_FONT, STR_LAST };

	enum IntSetting { INT_FIRST = STR_LAST + 1,
		CONNECTION_TYPE = INT_FIRST, PORT, SLOTS, ROLLBACK, AUTO_FOLLOW, CLEAR_SEARCH, FULL_ROW_SELECT, REMOVE_NOT_AVAILABLE, 
		BACKGROUND_COLOR, TEXT_COLOR, SHARE_HIDDEN, REMOVE_FINISHED, FILTER_KICKMSGS, MINIMIZE_TRAY,
		OPEN_PUBLIC, OPEN_QUEUE, AUTO_SEARCH, TIME_STAMPS, CONFIRM_EXIT, IGNORE_OFFLINE, POPUP_OFFLINE,
		REMOVE_DUPES,
		INT_LAST, SETTINGS_LAST = INT_LAST };

	enum {	SPEED_288K, SPEED_336K, SPEED_576K, SPEED_ISDN, SPEED_SATELLITE, SPEED_CABLE,
			SPEED_DSL, SPEED_T1, SPEED_T3, SPEED_LAST };

	enum {	CONNECTION_ACTIVE, CONNECTION_PASSIVE };

	const string& get(StrSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? strSettings[key - STR_FIRST] : strDefaults[key - STR_FIRST];
	}

	int get(IntSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? intSettings[key - INT_FIRST] : intDefaults[key - INT_FIRST];
	}

	bool getBool(IntSetting key, bool useDefault = true) const {
		return (get(key, useDefault) != 0);
	}

	void set(StrSetting key, string const& value) {
		strSettings[key - STR_FIRST] = value;
		isSet[key] = !value.empty();
	}

	void set(IntSetting key, int value) {
		if(key == SLOTS) {
			if(value <= 0)
				value = 1;
		}
		intSettings[key - INT_FIRST] = value;
		isSet[key] = true;
	}

	void set(IntSetting key, const string& value) {
		if(value.empty()) {
			intSettings[key - INT_FIRST] = 0;
			isSet[key] = false;
		} else {
			intSettings[key - INT_FIRST] = Util::toInt(value);
			isSet[key] = true;
		}
	}
	
	void set(IntSetting key, bool value) { set(key, (int)value); }

	void setDefault(StrSetting key, string const& value) {
		strDefaults[key - STR_FIRST] = value;
	}

	void setDefault(IntSetting key, int value) {
		intDefaults[key - INT_FIRST] = value;
	}

	void load() {
		load(Util::getAppPath() + "DCPlusPlus.xml");
	}
	void oldLoad(SimpleXML* xml);
	void save() const {
		save(Util::getAppPath() + "DCPlusPlus.xml");
	}

	void load(const string& aFileName);
	void save(const string& aFileName) const;

private:
	friend class Singleton<SettingsManager>;
	SettingsManager();

	static const string settingTags[SETTINGS_LAST+1];

	string strSettings[STR_LAST - STR_FIRST];
	int    intSettings[INT_LAST - INT_FIRST];
	string strDefaults[STR_LAST - STR_FIRST];
	int    intDefaults[INT_LAST - INT_FIRST];
	bool isSet[SETTINGS_LAST];
};


// Shorthand accessor macros
#define SETTING(k) (SettingsManager::getInstance()->get(SettingsManager::k, true))
#define BOOLSETTING(k) (SettingsManager::getInstance()->getBool(SettingsManager::k, true))

#endif // SETTINGSMANAGER_H

/**
 * @file SettingsManager.cpp
 * $Id: SettingsManager.h,v 1.18 2002/03/10 22:41:08 arnetheduck Exp $
 * @if LOG
 * $Log: SettingsManager.h,v $
 * Revision 1.18  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.17  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.16  2002/02/28 00:10:47  arnetheduck
 * Some fixes to the new user model
 *
 * Revision 1.15  2002/02/25 15:39:29  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.14  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.13  2002/02/06 12:29:06  arnetheduck
 * New Buffered socket handling with asynchronous sending (asynchronous everything really...)
 *
 * Revision 1.12  2002/02/03 01:06:56  arnetheduck
 * More bugfixes and some minor changes
 *
 * Revision 1.11  2002/02/02 17:21:27  arnetheduck
 * Fixed search bugs and some other things...
 *
 * Revision 1.10  2002/02/01 02:00:44  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.9  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.8  2002/01/26 16:34:01  arnetheduck
 * Colors dialog added, as well as some other options
 *
 * Revision 1.7  2002/01/26 14:59:23  arnetheduck
 * Fixed disconnect crash
 *
 * Revision 1.6  2002/01/26 12:52:51  arnetheduck
 * More minor fixes
 *
 * @endif
 */

