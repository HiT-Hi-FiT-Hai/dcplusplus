/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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
	CriticalSection() throw() {
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&mtx, &attr);	
	};
	~CriticalSection() throw() { pthread_mutex_destroy(&mtx); };
	void enter() throw() { pthread_mutex_lock(&mtx); };
	void leave() throw() { pthread_mutex_unlock(&mtx); };
	pthread_mutex_t& getMutex() { return mtx; };
private:
	pthread_mutex_t mtx;
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
 * $Id: CriticalSection.h,v 1.13 2003/03/13 13:31:16 arnetheduck Exp $
 */
