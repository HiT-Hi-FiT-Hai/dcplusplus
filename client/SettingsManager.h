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
#include "DCPlusPlus.h"
#include "Util.h"

class SimpleXML;

class SettingsManager : public Singleton<SettingsManager>
{
public:

	static const char*connectionSpeeds[];

	enum StrSetting { STR_FIRST,
		CONNECTION = STR_FIRST, DESCRIPTION, DOWNLOAD_DIRECTORY, EMAIL, NICK, SERVER,
		CLIENTVERSION, STR_LAST };

	enum IntSetting { INT_FIRST = STR_LAST + 1,
		CONNECTION_TYPE = INT_FIRST, PORT, SLOTS, ROLLBACK, AUTO_FOLLOW, CLEAR_SEARCH, FULL_ROW_SELECT,
		INT_LAST, SETTINGS_LAST = INT_LAST };

	enum {	SPEED_288K, SPEED_336K, SPEED_576K, SPEED_ISDN, SPEED_SATELLITE, SPEED_CABLE,
			SPEED_DSL, SPEED_T1, SPEED_T3, SPEED_LAST };

	enum {	CONNECTION_ACTIVE, CONNECTION_PASSIVE };

	string const& get(StrSetting key, bool useDefault = true) const {
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
	static string ReadStringFromFile(string const& aFileName);
	static bool WriteStringToFile(string const& aFileName, string const& text);

	friend class Singleton<SettingsManager>;
	SettingsManager();

	static char const* settingTags[SETTINGS_LAST+1];

	string strSettings[STR_LAST - STR_FIRST];
	int    intSettings[INT_LAST - INT_FIRST];
	string strDefaults[STR_LAST - STR_FIRST];
	int    intDefaults[INT_LAST - INT_FIRST];
	bool isSet[SETTINGS_LAST];
};

// Shorthand accessor class
#ifdef ALTERNATE_SETTING_ACCESSORS
class Settings
{
public:
	__inline static string const& getString(StrSetting key, bool useDefault = true) const {
		return SettingsManager::getInstance()->get(key, useDefault);
	}
	__inline static int getInt(IntSetting key, bool useDefault = true) const {
		return SettingsManager::getInstance()->get(key, useDefault);
	}
	__inline static bool getBool(IntSetting key, bool useDefault = true) const {
		return SettingsManager::getInstance()->getBool(key, useDefault);
	}
	__inline static void set(StrSetting key, string const& value) {
		SettingsManager::getInstance()->set(key, value);
	}
	__inline static void set(IntSetting key, int value) {
		SettingsManager::getInstance()->set(key, value);
	}
	__inline static void set(IntSetting key, bool value) {
		SettingsManager::getInstance()->set(key, value);
	}
	__inline static void setDefault(StrSetting key, string const& value) {
		SettingsManager::getInstance()->setDefault(key, value);
	}
	__inline void setDefault(IntSetting key, int value) {
		SettingsManager::getInstance()->setDefault(key, value);
	}
}

// Shorthand accessor functions
__inline string const& Setting(SettingsManager::StrSetting key, bool useDefault = true) {
	return SettingsManager::getInstance()->get(key, useDefault);
}

__inline int Setting(SettingsManager::IntSetting key, bool useDefault = true) {
	return SettingsManager::getInstance()->get(key, useDefault);
}

#endif

// Shorthand accessor macros
#define SETTING(k) (SettingsManager::getInstance()->get(SettingsManager::k, true))
#define BOOLSETTING(k) (SettingsManager::getInstance()->getBool(SettingsManager::k, true))

#endif // SETTINGSMANAGER_H
