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

#if !defined(AFX_TIMERMANAGER_H__2172C2AD_D4FD_4B46_A1B2_7959D7359CCD__INCLUDED_)
#define AFX_TIMERMANAGER_H__2172C2AD_D4FD_4B46_A1B2_7959D7359CCD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "Thread.h"
#include "Semaphore.h"

#ifndef WIN32
#include <sys/time.h>
#endif

class TimerManagerListener {
public:
	typedef TimerManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	enum Types {
		SECOND,
		MINUTE
	};
	
	virtual void onAction(Types, u_int32_t) { };
};

class TimerManager : public Speaker<TimerManagerListener>, public Singleton<TimerManager>, public Thread
{
public:
	u_int32_t getTick() { 
#ifdef WIN32
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
#ifndef WIN32
		gettimeofday(&tv, NULL);
#endif
	};
	
	virtual ~TimerManager() {
		s.signal();
		join();
	};
	
	virtual int run();
	
#ifndef WIN32
	timeval tv;
#endif
};

#define GET_TICK() TimerManager::getInstance()->getTick()
#endif // !defined(AFX_TIMERMANAGER_H__2172C2AD_D4FD_4B46_A1B2_7959D7359CCD__INCLUDED_)

/**
 * @file TimerManager.h
 * $Id: TimerManager.h,v 1.11 2002/04/09 18:43:28 arnetheduck Exp $
 * @if LOG
 * $Log: TimerManager.h,v $
 * Revision 1.11  2002/04/09 18:43:28  arnetheduck
 * Major code reorganization, to ease maintenance and future port...
 *
 * Revision 1.10  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.9  2002/02/26 23:25:22  arnetheduck
 * Minor updates and fixes
 *
 * Revision 1.8  2002/02/10 12:25:24  arnetheduck
 * New properties for favorites, and some minor performance tuning...
 *
 * Revision 1.7  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.6  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.5  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.4  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.3  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.2  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.1  2001/12/02 11:18:10  arnetheduck
 * Added transfer totals and speed...
 *
 * @endif
 */

