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

#if !defined(AFX_CRITCALSECTION_H__1226AAB5_254F_4CBD_B384_5E8D3A23C346__INCLUDED_)
#define AFX_CRITCALSECTION_H__1226AAB5_254F_4CBD_B384_5E8D3A23C346__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Thread.h"

class CriticalSection  
{
#ifdef WIN32
public:
	void enter() throw() {
		EnterCriticalSection(&cs);
		dcdrun(counter++);	
	}
	void leave() throw() {
		dcassert(--counter >= 0);
		LeaveCriticalSection(&cs);
	}
	CriticalSection() throw() {
		dcdrun(counter = 0;);
		InitializeCriticalSection(&cs);
	}
	~CriticalSection() throw() {
		dcassert(counter==0);
		DeleteCriticalSection(&cs);
	}
private:
	dcdrun(long counter;);
	CRITICAL_SECTION cs;
#else
public:
	void enter() throw() { };
	void leave() throw() { };
#endif
};

class Lock {
private:
	CriticalSection& cs;
public:
	Lock(CriticalSection& aCs) throw() : cs(aCs)  { cs.enter(); };
	~Lock() throw() { cs.leave(); };
};

class RWLock
{
public:
	RWLock() throw() : readers(0) { }
	~RWLock() throw() { dcassert(readers==0); }

	void enterRead() throw() {
		Lock l(cs);
		readers++;
		dcassert(readers < 100);
	}

	void leaveRead() throw() {
		Thread::safeDec(&readers);
		dcassert(readers >= 0);
	}
	void enterWrite() throw() {
		cs.enter();
		while(readers > 0) {
			cs.leave();
			Thread::yield();
			
			cs.enter();
		}
	}
	void leaveWrite() {
		cs.leave();
	}
private:
	CriticalSection cs;
	long readers;
};

class RLock {
public:
	RLock(RWLock& aCs) throw() : cs(aCs)  { cs.enterRead(); };
	~RLock() throw() { cs.leaveRead(); };
private:
	RWLock& cs;
};

class WLock {
public:
	WLock(RWLock& aCs) throw() : cs(aCs)  { cs.enterWrite(); };
	~WLock() throw() { cs.leaveWrite(); };
private:
	RWLock& cs;
};

#endif // !defined(AFX_CRITCALSECTION_H__1226AAB5_254F_4CBD_B384_5E8D3A23C346__INCLUDED_)

/**
 * @file CriticalSection.h
 * $Id: CriticalSection.h,v 1.10 2002/04/09 18:43:27 arnetheduck Exp $
 * @if LOG
 * $Log: CriticalSection.h,v $
 * Revision 1.10  2002/04/09 18:43:27  arnetheduck
 * Major code reorganization, to ease maintenance and future port...
 *
 * Revision 1.9  2002/03/19 00:41:37  arnetheduck
 * 0.162, hub counting and cpu bug
 *
 * Revision 1.8  2002/03/07 19:07:51  arnetheduck
 * Minor fixes + started code review
 *
 * Revision 1.7  2002/03/04 23:52:30  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.6  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.5  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.4  2002/01/16 20:56:26  arnetheduck
 * Bug fixes, file listing sort and some other small changes
 *
 * Revision 1.3  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * @endif
 */
