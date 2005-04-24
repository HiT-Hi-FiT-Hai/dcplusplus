/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(SETTINGS_MANAGER_H)
#define SETTINGS_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "Speaker.h"
#include "Singleton.h"

class SimpleXML;

class SettingsManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Load;
	typedef X<1> Save;

	virtual void on(Load, SimpleXML*) throw() { }
	virtual void on(Save, SimpleXML*) throw() { }
};

class SettingsManager : public Singleton<SettingsManager>, public Speaker<SettingsManagerListener>
{
public:

	static StringList connectionSpeeds;

	enum StrSetting { STR_FIRST,
		NICK = STR_FIRST, UPLOAD_SPEED, DESCRIPTION, DOWNLOAD_DIRECTORY, EMAIL, EXTERNAL_IP,
		TEXT_FONT, MAINFRAME_ORDER, MAINFRAME_WIDTHS, HUBFRAME_ORDER, HUBFRAME_WIDTHS, 
		LANGUAGE_FILE, SEARCHFRAME_ORDER, SEARCHFRAME_WIDTHS, FAVORITESFRAME_ORDER, FAVORITESFRAME_WIDTHS, 
		HUBLIST_SERVERS, QUEUEFRAME_ORDER, QUEUEFRAME_WIDTHS, PUBLICHUBSFRAME_ORDER, PUBLICHUBSFRAME_WIDTHS, 
		USERSFRAME_ORDER, USERSFRAME_WIDTHS, HTTP_PROXY, LOG_DIRECTORY, NOTEPAD_TEXT, LOG_FORMAT_POST_DOWNLOAD, 
		LOG_FORMAT_POST_UPLOAD, LOG_FORMAT_MAIN_CHAT, LOG_FORMAT_PRIVATE_CHAT, FINISHED_ORDER, FINISHED_WIDTHS, 
		TEMP_DOWNLOAD_DIRECTORY, BIND_ADDRESS, SOCKS_SERVER, SOCKS_USER, SOCKS_PASSWORD, CONFIG_VERSION,
		DEFAULT_AWAY_MESSAGE, TIME_STAMPS_FORMAT, ADLSEARCHFRAME_ORDER, ADLSEARCHFRAME_WIDTHS, 
		FINISHED_UL_WIDTHS, FINISHED_UL_ORDER, CLIENT_ID, SPYFRAME_WIDTHS, SPYFRAME_ORDER, LOG_FILE_MAIN_CHAT, 
		LOG_FILE_PRIVATE_CHAT, LOG_FILE_STATUS, LOG_FILE_UPLOAD, LOG_FILE_DOWNLOAD, LOG_FILE_SYSTEM,
		LOG_FORMAT_SYSTEM, LOG_FORMAT_STATUS, DIRECTORLISTINGFRAME_ORDER, DIRECTORLISTINGFRAME_WIDTHS, 
		STR_LAST };

	enum IntSetting { INT_FIRST = STR_LAST + 1,
		INCOMING_CONNECTIONS = INT_FIRST, TCP_PORT, SLOTS, ROLLBACK, AUTO_FOLLOW, CLEAR_SEARCH, 
		BACKGROUND_COLOR, TEXT_COLOR, USE_OEM_MONOFONT, SHARE_HIDDEN, FILTER_MESSAGES, MINIMIZE_TRAY,
		AUTO_SEARCH, TIME_STAMPS, CONFIRM_EXIT, IGNORE_OFFLINE, POPUP_OFFLINE,
		LIST_DUPES, BUFFER_SIZE, DOWNLOAD_SLOTS, MAX_DOWNLOAD_SPEED, LOG_MAIN_CHAT, LOG_PRIVATE_CHAT,
		LOG_DOWNLOADS, LOG_UPLOADS, STATUS_IN_CHAT, SHOW_JOINS, PRIVATE_MESSAGE_BEEP, PRIVATE_MESSAGE_BEEP_OPEN,
		USE_SYSTEM_ICONS, POPUP_PMS, MIN_UPLOAD_SPEED, GET_USER_INFO, URL_HANDLER, MAIN_WINDOW_STATE,
		MAIN_WINDOW_SIZE_X, MAIN_WINDOW_SIZE_Y, MAIN_WINDOW_POS_X, MAIN_WINDOW_POS_Y, AUTO_AWAY,
		SMALL_SEND_BUFFER, SOCKS_PORT, SOCKS_RESOLVE, KEEP_LISTS, AUTO_KICK, QUEUEFRAME_SHOW_TREE,
		COMPRESS_TRANSFERS, SHOW_PROGRESS_BARS, SFV_CHECK, MAX_TAB_ROWS, AUTO_UPDATE_LIST,
		MAX_COMPRESSION, FINISHED_DIRTY, QUEUE_DIRTY, TAB_DIRTY, ANTI_FRAG, MDI_MAXIMIZED, NO_AWAYMSG_TO_BOTS,
		SKIP_ZERO_BYTE, ADLS_BREAK_ON_FIRST, TAB_COMPLETION, 
		HUB_USER_COMMANDS, AUTO_SEARCH_AUTO_MATCH, UPLOAD_BAR_COLOR, DOWNLOAD_BAR_COLOR, LOG_SYSTEM,
		LOG_FILELIST_TRANSFERS, SEND_UNKNOWN_COMMANDS, MAX_HASH_SPEED, OPEN_USER_CMD_HELP,
		GET_USER_COUNTRY, FAV_SHOW_JOINS, LOG_STATUS_MESSAGES, SHOW_STATUSBAR,
		SHOW_TOOLBAR, SHOW_TRANSFERVIEW, POPUNDER_PM, POPUNDER_FILELIST, MAGNET_ASK, MAGNET_ACTION, MAGNET_REGISTER,
		ADD_FINISHED_INSTANTLY, DONT_DL_ALREADY_SHARED, SETTINGS_USE_CTRL_FOR_LINE_HISTORY, CONFIRM_HUB_REMOVAL, 
		SETTINGS_OPEN_NEW_WINDOW, UDP_PORT, SEARCH_ONLY_TTH, SHOW_LAST_LINES_LOG, CONFIRM_ITEM_REMOVAL,
		ADVANCED_RESUME, ADC_DEBUG, TOGGLE_ACTIVE_WINDOW, SEARCH_HISTORY, SET_MINISLOT_SIZE,
		PRIO_HIGHEST_SIZE, PRIO_HIGH_SIZE, PRIO_NORMAL_SIZE, PRIO_LOW_SIZE, PRIO_LOWEST, 
		OPEN_PUBLIC, OPEN_FAVORITE_HUBS, OPEN_FAVORITE_USERS, OPEN_QUEUE, OPEN_FINISHED_DOWNLOADS, 
		OPEN_FINISHED_UPLOADS, OPEN_SEARCH_SPY, OPEN_NETWORK_STATISTICS, OPEN_NOTEPAD, OUTGOING_CONNECTIONS, 
		NO_IP_OVERRIDE,
		INT_LAST };

	enum Int64Setting { INT64_FIRST = INT_LAST + 1,
		TOTAL_UPLOAD = INT64_FIRST, TOTAL_DOWNLOAD, INT64_LAST, SETTINGS_LAST = INT64_LAST };

	enum {	INCOMING_DIRECT, INCOMING_FIREWALL_UPNP, INCOMING_FIREWALL_NAT,
		INCOMING_FIREWALL_PASSIVE };
	enum {	OUTGOING_DIRECT, OUTGOING_SOCKS5 };

	enum {	MAGNET_AUTO_SEARCH, MAGNET_AUTO_DOWNLOAD };

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
	virtual ~SettingsManager() throw() { }

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

#endif // !defined(SETTINGS_MANAGER_H)

/**
 * @file
 * $Id: SettingsManager.h,v 1.97 2005/04/24 08:13:11 arnetheduck Exp $
 */
