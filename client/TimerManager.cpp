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
	t->nextTick = GetTickCount() + 1000;
	while(WaitForSingleObject(t->stopEvent, t->nextTick - GetTickCount()) == WAIT_TIMEOUT) {
		t->nextTick = GetTickCount() + 1000;
		t->fireSecond();
	}

	return 0;
}
/**
 * @file TimerManager.cpp
 * $Id: TimerManager.cpp,v 1.1 2001/12/02 11:18:10 arnetheduck Exp $
 * @if LOG
 * $Log: TimerManager.cpp,v $
 * Revision 1.1  2001/12/02 11:18:10  arnetheduck
 * Added transfer totals and speed...
 *
 * @endif
 */

