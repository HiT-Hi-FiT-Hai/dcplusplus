/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "TimerManager.h"

TimerManager* TimerManager::instance;

DWORD WINAPI TimerManager::ticker(void* p) {
	TimerManager* t = (TimerManager*)p;
	DWORD nextTick;
	int nextMin = 0;
	int x;

	nextTick = GetTickCount() + 1000;
	while(WaitForSingleObject(t->stopEvent, (x = (nextTick - GetTickCount())) > 0 ? x : 0) == WAIT_TIMEOUT) {
		DWORD z = GetTickCount();
		nextTick = z + 1000;
		t->fireSecond(z);
		if(nextMin++ >= 60) {
			t->fireMinute(z);
			nextMin = 0;
		}
	}

	return 0;
}
/**
 * @file TimerManager.cpp
 * $Id: TimerManager.cpp,v 1.4 2001/12/16 19:47:48 arnetheduck Exp $
 * @if LOG
 * $Log: TimerManager.cpp,v $
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

