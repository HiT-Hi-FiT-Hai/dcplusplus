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

#if !defined(AFX_TIMERMANAGER_H__2172C2AD_D4FD_4B46_A1B2_7959D7359CCD__INCLUDED_)
#define AFX_TIMERMANAGER_H__2172C2AD_D4FD_4B46_A1B2_7959D7359CCD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Thread.h"
#include "Semaphore.h"
#include "Speaker.h"
#include "Singleton.h"

#ifndef _WIN32
#include <sys/time.h>
#endif

class TimerManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Second;
	typedef X<1> Minute;

	// We expect everyone to implement this...
	virtual void on(Second, u_int32_t) throw() { }
	virtual void on(Minute, u_int32_t) throw() { }
};

class TimerManager : public Speaker<TimerManagerListener>, public Singleton<TimerManager>, public Thread
{
public:
	static time_t getTime() {
		return (time_t)time(NULL);
	}
	static u_int32_t getTick() { 
#ifdef _WIN32
		return GetTickCount(); 
#else
		timeval tv2;
		gettimeofday(&tv2, NULL);
		return (u_int32_t)((tv2.tv_sec - tv.tv_sec) * 1000 ) + ( (tv2.tv_usec - tv.tv_usec) / 1000);
#endif
	};
private:

	Semaphore s;

	friend class Singleton<TimerManager>;
	TimerManager() { 
#ifndef _WIN32
		gettimeofday(&tv, NULL);
#endif
	};
	
	virtual ~TimerManager() {
		dcassert(listeners.empty());
		s.signal();
		join();
	};
	
	virtual int run();
	
#ifndef _WIN32
	static timeval tv;
#endif
};

#define GET_TICK() TimerManager::getTick()
#define GET_TIME() TimerManager::getTime()

#endif // !defined(AFX_TIMERMANAGER_H__2172C2AD_D4FD_4B46_A1B2_7959D7359CCD__INCLUDED_)

/**
 * @file
 * $Id: TimerManager.h,v 1.24 2004/09/06 12:32:43 arnetheduck Exp $
 */

