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
	void log(const string& area, const string& msg) throw() {
		Lock l(cs);
		try {
			string aArea = Util::validateFileName(SETTING(LOG_DIRECTORY) + area);
			File::ensureDirectory(aArea);
			File f(aArea, File::WRITE, File::OPEN | File::CREATE);
			f.setEndPos(0);
			f.write(msg + "\r\n");
		} catch (const FileException&) {
			// ...
		}
	}

	void logDateTime(const string& area, const string& msg) throw() {
		log(area, Util::formatTime("%Y-%m-%d %H:%M: ", TimerManager::getInstance()->getTime()) + msg);
	}

	void message(const string& msg) {
		if(BOOLSETTING(LOG_SYSTEM)) {
			log("system.log", Util::formatTime("%Y-%m-%d %H:%M:%S: ", TimerManager::getInstance()->getTime()) + msg);
		}
		fire(LogManagerListener::Message(), msg);
	}

private:
	friend class Singleton<LogManager>;
	CriticalSection cs;
	
	LogManager() { };
	virtual ~LogManager() { };
	
};

#define LOG(area, msg) LogManager::getInstance()->log(area, msg)
#define LOGDT(area, msg) LogManager::getInstance()->logDateTime(area, msg)

#endif // !defined(AFX_LOGMANAGER_H__73C7E0F5_5C7D_4A2A_827B_53267D0EF4C5__INCLUDED_)

/**
 * @file
 * $Id: LogManager.h,v 1.13 2004/12/05 15:51:05 arnetheduck Exp $
 */
