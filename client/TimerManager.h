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

#if !defined(AFX_TIMERMANAGER_H__2172C2AD_D4FD_4B46_A1B2_7959D7359CCD__INCLUDED_)
#define AFX_TIMERMANAGER_H__2172C2AD_D4FD_4B46_A1B2_7959D7359CCD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"

class TimerManagerListener {
public:
	typedef TimerManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	virtual void onTimerSecond(DWORD aTick) { };
	
};

class TimerManager : public Speaker<TimerManagerListener>
{
public:
	static DWORD getTick() { return GetTickCount(); };
	
	static TimerManager* getInstance() {
		dcassert(instance);
		return instance;
	}
	
	static void newInstance() {
		if(instance)
			delete instance;
		
		instance = new TimerManager();
	}
	
	static void deleteInstance() {
		if(instance)
			delete instance;
		instance = NULL;
	}
	
private:
	TimerManager() : stopEvent(NULL), readerThread(NULL) { 
		startTicker();		
	};
	virtual ~TimerManager() {
		stopTicker();
	};
	
	HANDLE stopEvent;
	HANDLE readerThread;

	void startTicker() {
		DWORD threadId;
		stopTicker();
		
		stopEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
		readerThread=CreateThread(NULL, 0, &ticker, this, 0, &threadId);
	}
	
	void stopTicker() {
		if(readerThread != NULL) {
			SetEvent(stopEvent);
			
			if(WaitForSingleObject(readerThread, 1000) == WAIT_TIMEOUT) {
				MessageBox(NULL, _T("TimerManager: Unable to stop reader thread!!!"), _T("Internal error"), MB_OK | MB_ICONERROR);
			}
			
			readerThread = NULL;
			CloseHandle(stopEvent);
			stopEvent = NULL;
		}
	}
	
	static DWORD WINAPI ticker(void* p);

	static TimerManager* instance;

	void fireSecond(DWORD aTick) {
		listenerCS.enter();
		TimerManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(TimerManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onTimerSecond(aTick);
		}
	}
	
};

#endif // !defined(AFX_TIMERMANAGER_H__2172C2AD_D4FD_4B46_A1B2_7959D7359CCD__INCLUDED_)

/**
 * @file TimerManager.h
 * $Id: TimerManager.h,v 1.3 2001/12/04 21:50:34 arnetheduck Exp $
 * @if LOG
 * $Log: TimerManager.h,v $
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

