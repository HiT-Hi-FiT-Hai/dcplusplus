/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#include "Util.h"
#include "Speaker.h"
#include "Singleton.h"

class SimpleXML;

class SettingsManagerListener {
public:
	typedef SettingsManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum Types {
		LOAD,
		SAVE
	};
	
	virtual void onAction(Types, SimpleXML*) throw() = 0;
};

class SettingsManager : public Singleton<SettingsManager>, public Speaker<SettingsManagerListener>
{
public:

	static const string connectionSpeeds[];

	enum StrSetting { STR_FIRST,
		CONNECTION = STR_FIRST, DESCRIPTION, DOWNLOAD_DIRECTORY, EMAIL, NICK, SERVER,
		CLIENTVERSION, TEXT_FONT, MAINFRAME_ORDER, MAINFRAME_WIDTHS, HUBFRAME_ORDER, HUBFRAME_WIDTHS, 
		LANGUAGE_FILE, SEARCHFRAME_ORDER, SEARCHFRAME_WIDTHS, FAVORITESFRAME_ORDER, FAVORITESFRAME_WIDTHS, 
		HUBLIST_SERVERS, QUEUEFRAME_ORDER, QUEUEFRAME_WIDTHS, PUBLICHUBSFRAME_ORDER, PUBLICHUBSFRAME_WIDTHS, 
		USERSFRAME_ORDER, USERSFRAME_WIDTHS, HTTP_PROXY, LOG_DIRECTORY, NOTEPAD_TEXT, LOG_FORMAT_POST_DOWNLOAD, 
		LOG_FORMAT_POST_UPLOAD, LOG_FORMAT_MAIN_CHAT, LOG_FORMAT_PRIVATE_CHAT, FINISHED_ORDER, FINISHED_WIDTHS, 
		TEMP_DOWNLOAD_DIRECTORY, SOCKS_SERVER, SOCKS_USER, SOCKS_PASSWORD, CONFIG_VERSION,
		DEFAULT_AWAY_MESSAGE, ADLSEARCHFRAME_ORDER, ADLSEARCHFRAME_WIDTHS, 
		FINISHED_UL_WIDTHS, FINISHED_UL_ORDER,
		STR_LAST };

	enum IntSetting { INT_FIRST = STR_LAST + 1,
		CONNECTION_TYPE = INT_FIRST, IN_PORT, SLOTS, ROLLBACK, AUTO_FOLLOW, CLEAR_SEARCH, FULL_ROW_SELECT, 
		BACKGROUND_COLOR, TEXT_COLOR, USE_OEM_MONOFONT, SHARE_HIDDEN, FILTER_MESSAGES, MINIMIZE_TRAY,
		OPEN_PUBLIC, OPEN_QUEUE, AUTO_SEARCH, AUTO_SEARCH_AUTO_STRING, TIME_STAMPS, CONFIRM_EXIT, IGNORE_OFFLINE, POPUP_OFFLINE,
		REMOVE_DUPES, BUFFER_SIZE, DOWNLOAD_SLOTS, MAX_DOWNLOAD_SPEED, LOG_MAIN_CHAT, LOG_PRIVATE_CHAT,
		LOG_DOWNLOADS, LOG_UPLOADS, STATUS_IN_CHAT, SHOW_JOINS, PRIVATE_MESSAGE_BEEP, PRIVATE_MESSAGE_BEEP_OPEN,
		USE_SYSTEM_ICONS, POPUP_PMS, MIN_UPLOAD_SPEED, GET_USER_INFO, URL_HANDLER, MAIN_WINDOW_STATE,
		MAIN_WINDOW_SIZE_X, MAIN_WINDOW_SIZE_Y, MAIN_WINDOW_POS_X, MAIN_WINDOW_POS_Y, AUTO_AWAY,
		SMALL_SEND_BUFFER, SOCKS_PORT, SOCKS_RESOLVE, KEEP_LISTS, AUTO_KICK, QUEUEFRAME_SHOW_TREE,
		COMPRESS_TRANSFERS, SHOW_PROGRESS_BARS, SFV_CHECK, MAX_TAB_ROWS, AUTO_UPDATE_LIST,
		MAX_COMPRESSION, FINISHED_DIRTY, ANTI_FRAG, MDI_MAXIMIZED, NO_AWAYMSG_TO_BOTS,
		SKIP_ZERO_BYTE, ADLS_BREAK_ON_FIRST, TAB_COMPLETION, OPEN_FAVORITE_HUBS, OPEN_FINISHED_DOWNLOADS,
		HUB_USER_COMMANDS, AUTO_SEARCH_AUTO_MATCH, UPLOAD_BAR_COLOR, DOWNLOAD_BAR_COLOR, 
		LOG_FILELIST_TRANSFERS, INT_LAST };

	enum Int64Setting { INT64_FIRST = INT_LAST + 1,
		TOTAL_UPLOAD = INT64_FIRST, TOTAL_DOWNLOAD, INT64_LAST, SETTINGS_LAST = INT64_LAST };

	enum {	SPEED_288K, SPEED_336K, SPEED_576K, SPEED_ISDN, SPEED_SATELLITE, SPEED_CABLE,
			SPEED_DSL, SPEED_T1, SPEED_T3, SPEED_LAST };

	enum {	CONNECTION_ACTIVE, CONNECTION_PASSIVE, CONNECTION_SOCKS5 };

	const string& get(StrSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? strSettings[key - STR_FIRST] : strDefaults[key - STR_FIRST];
	}

	int get(IntSetting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? intSettings[key - INT_FIRST] : intDefaults[key - INT_FIRST];
	}
	int64_t get(Int64Setting key, bool useDefault = true) const {
		return (isSet[key] || !useDefault) ? int64Settings[key - INT64_FIRST] : int64Defaults[key - INT64_FIRST];
	}

	bool getBool(IntSetting key, bool useDefault = true) const {
		return (get(key, useDefault) != 0);
	}

	void set(StrSetting key, string const& value) {
		if(((key == DESCRIPTION) || (key == NICK)) && (value.size() > 35)) {
			strSettings[key - STR_FIRST] = value.substr(0, 35);
		} else {
			strSettings[key - STR_FIRST] = value;
		}
		isSet[key] = !value.empty();
	}

	void set(IntSetting key, int value) {
		if((key == SLOTS) && (value <= 0)) {
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

	void set(Int64Setting key, int64_t value) {
		int64Settings[key - INT64_FIRST] = value;
		isSet[key] = true;
	}

	void set(Int64Setting key, const string& value) {
		if(value.empty()) {
			int64Settings[key - INT64_FIRST] = 0;
			isSet[key] = false;
		} else {
			int64Settings[key - INT64_FIRST] = Util::toInt64(value);
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
	void setDefault(Int64Setting key, int64_t value) {
		int64Defaults[key - INT64_FIRST] = value;
	}

	bool isDefault(int aSet) { return !isSet[aSet]; };

	void load() {
		load(Util::getAppPath() + "DCPlusPlus.xml");
	}
	void save() {
		save(Util::getAppPath() + "DCPlusPlus.xml");
	}

	void load(const string& aFileName);
	void save(const string& aFileName);

private:
	friend class Singleton<SettingsManager>;
	SettingsManager();

	static const string settingTags[SETTINGS_LAST+1];

	string strSettings[STR_LAST - STR_FIRST];
	int    intSettings[INT_LAST - INT_FIRST];
	int64_t int64Settings[INT64_LAST - INT64_FIRST];
	string strDefaults[STR_LAST - STR_FIRST];
	int    intDefaults[INT_LAST - INT_FIRST];
	int64_t int64Defaults[INT64_LAST - INT64_FIRST];
	bool isSet[SETTINGS_LAST];
};

// Shorthand accessor macros
#define SETTING(k) (SettingsManager::getInstance()->get(SettingsManager::k, true))
#define BOOLSETTING(k) (SettingsManager::getInstance()->getBool(SettingsManager::k, true))

#endif // SETTINGSMANAGER_H

/**
 * @file
 * $Id: SettingsManager.h,v 1.52 2004/01/07 14:14:52 arnetheduck Exp $
 */

