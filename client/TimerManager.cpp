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

#include "TimerManager.h"

TimerManager* Singleton<TimerManager>::instance = NULL;

#ifndef WIN32
timeval TimerManager::tv;
#endif

int TimerManager::run() {
	int nextMin = 0;

	u_int32_t x = getTick();
	u_int32_t nextTick = x + 1000;

	while(!s.wait(nextTick > x ? nextTick - x : 0)) {
		u_int32_t z = getTick();
		nextTick = z + 1000;
		fire(TimerManagerListener::SECOND, z);
		if(nextMin++ >= 60) {
			fire(TimerManagerListener::MINUTE, z);
			nextMin = 0;
		}
		x = getTick();
	}

	return 0;
}

/**
 * @file TimerManager.cpp
 * $Id: TimerManager.cpp,v 1.11 2002/12/28 01:31:49 arnetheduck Exp $
 */

