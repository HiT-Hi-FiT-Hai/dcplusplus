/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#if !defined(AFX_LOGMANAGER_H__73C7E0F5_5C7D_4A2A_827B_53267D0EF4C5__INCLUDED_)
#define AFX_LOGMANAGER_H__73C7E0F5_5C7D_4A2A_827B_53267D0EF4C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "File.h"
#include "CriticalSection.h"
#include "Singleton.h"
#include "TimerManager.h"

class LogManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Message;
	virtual void on(Message, const string&) throw() { };
};

class LogManager : public Singleton<LogManager>, public Speaker<LogManagerListener>
{
public:
	enum LogArea { CHAT, PM, DOWNLOAD, UPLOAD, SYSTEM, STATUS, LAST };
	enum {FILE, FORMAT};

	void log(LogArea area, StringMap& params) throw() {
		string path = SETTING(LOG_DIRECTORY);
		string msg;
	
		path += Util::formatParams(getSetting(area, FILE), params);
		msg = Util::formatParams(getSetting(area, FORMAT), params);

		log(path, msg);
	}

	void message(const string& msg) {
		if(BOOLSETTING(LOG_SYSTEM)) {
			StringMap params;
			params["message"] = msg;
			log(LogManager::SYSTEM, params);
		}
		fire(LogManagerListener::Message(), msg);
	}

	const string& getSetting(int area, int sel) {
		return SettingsManager::getInstance()->get(static_cast<SettingsManager::StrSetting>(logOptions[area][sel]), true);
	}

	void saveSetting(int area, int sel, const string& setting) {
		SettingsManager::getInstance()->set(static_cast<SettingsManager::StrSetting>(logOptions[area][sel]), setting);
	}

private:
	void log(const string& area, const string& msg) throw() {
		Lock l(cs);
		try {
			string aArea = Util::validateFileName(area);
			File::ensureDirectory(aArea);
			File f(aArea, File::WRITE, File::OPEN | File::CREATE);
			f.setEndPos(0);
			f.write(msg + "\r\n");
		} catch (const FileException&) {
			// ...
		}
	}

	friend class Singleton<LogManager>;
	CriticalSection cs;

	int logOptions[LAST][2];

	LogManager() {
		logOptions[UPLOAD][FILE]		= SettingsManager::LOG_FILE_UPLOAD;
		logOptions[UPLOAD][FORMAT]		= SettingsManager::LOG_FORMAT_POST_UPLOAD;
        	logOptions[DOWNLOAD][FILE]		= SettingsManager::LOG_FILE_DOWNLOAD;
		logOptions[DOWNLOAD][FORMAT]		= SettingsManager::LOG_FORMAT_POST_DOWNLOAD;
		logOptions[CHAT][FILE]			= SettingsManager::LOG_FILE_MAIN_CHAT;
		logOptions[CHAT][FORMAT]		= SettingsManager::LOG_FORMAT_MAIN_CHAT;
		logOptions[PM][FILE]			= SettingsManager::LOG_FILE_PRIVATE_CHAT;
		logOptions[PM][FORMAT]			= SettingsManager::LOG_FORMAT_PRIVATE_CHAT;
		logOptions[SYSTEM][FILE]		= SettingsManager::LOG_FILE_SYSTEM;
		logOptions[SYSTEM][FORMAT]		= SettingsManager::LOG_FORMAT_SYSTEM;
		logOptions[STATUS][FILE]		= SettingsManager::LOG_FILE_STATUS;
		logOptions[STATUS][FORMAT]		= SettingsManager::LOG_FORMAT_STATUS;
	};
	virtual ~LogManager() { };

};

#define LOG(area, msg) LogManager::getInstance()->log(area, msg)

#endif // !defined(AFX_LOGMANAGER_H__73C7E0F5_5C7D_4A2A_827B_53267D0EF4C5__INCLUDED_)

/**
 * @file
 * $Id: LogManager.h,v 1.14 2004/12/29 19:52:34 arnetheduck Exp $
 */
