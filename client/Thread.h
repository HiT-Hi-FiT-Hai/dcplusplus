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

#if !defined(AFX_THREAD_H__3006956B_7C69_4DAD_9596_A49E1BD007D5__INCLUDED_)
#define AFX_THREAD_H__3006956B_7C69_4DAD_9596_A49E1BD007D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WIN32
#include <pthread.h>
#include <sched.h>
#include <asm/atomic.h>
#include <sys/resource.h>
#endif

#include "Exception.h"
STANDARD_EXCEPTION(ThreadException);

class Thread  
{
public:
#ifdef WIN32
	enum Priority {
		LOW = THREAD_PRIORITY_LOWEST,
		NORMAL = THREAD_PRIORITY_NORMAL,
		HIGH = THREAD_PRIORITY_HIGHEST
	};

	Thread() throw() : threadHandle(NULL), threadId(0){ };
	virtual ~Thread() { 
		if(threadHandle)
			CloseHandle(threadHandle);
	};
	
	void start() throw(ThreadException) {
		if( (threadHandle = CreateThread(NULL, 0, &starter, this, 0, &threadId)) == NULL) {
			throw ThreadException(STRING(UNABLE_TO_CREATE_THREAD));
		}
	}

	void join() throw(ThreadException) {
		if(threadHandle == NULL) {
			return;
		}

		WaitForSingleObject(threadHandle, INFINITE);
		CloseHandle(threadHandle);
		threadHandle = NULL;
	}

	void setThreadPriority(Priority p) { ::SetThreadPriority(threadHandle, p); };
	
	static void sleep(u_int32_t millis) { ::Sleep(millis); };
	static void yield() { ::Sleep(0); };
	static long safeInc(long* v) { return InterlockedIncrement(v); };
	static long safeDec(long* v) { return InterlockedDecrement(v); };
#else

	enum Priority {
		LOW = 1,
		NORMAL = 0,
		HIGH = -1
	};
	Thread() throw() : t(0) { };
	virtual ~Thread() { 
		if(t != 0) {
			pthread_detach(t);
		}
	};
	void start() throw(ThreadException) { 
		if(pthread_create(&t, NULL, &starter, this) != 0) {
			throw ThreadException(STRING(UNABLE_TO_CREATE_THREAD));
		}
	};
	void join() throw() { 
		void* x;
		pthread_join(t, &x);
		t = 0;
	};

	void setThreadPriority(Priority p) { setpriority(PRIO_PROCESS, 0, p); };
	static void sleep(u_int32_t millis) { ::usleep(millis*1000); };
	static void yield() { ::sched_yield(); };
	static long safeInc(long* v) { 
		atomic_t t = ATOMIC_INIT(*v);
		atomic_inc(&t);
		return (*v=t.counter);
	};
	static long safeDec(long* v) { 
		atomic_t t = ATOMIC_INIT(*v);
		atomic_dec(&t);
		return (*v=t.counter);
	};
#endif

protected:
	virtual int run() = 0;
	
private:

#ifdef WIN32
	HANDLE threadHandle;
	DWORD threadId;
	static DWORD WINAPI starter(void* p) {
		Thread* t = (Thread*)p;
		t->run();
		return 0;
	}
#else
	pthread_t t;
	static void* starter(void* p) {
		Thread* t = (Thread*)p;
		t->run();
		return NULL;
	}
#endif
};

#endif // !defined(AFX_THREAD_H__3006956B_7C69_4DAD_9596_A49E1BD007D5__INCLUDED_)

/**
 * @file Thread.h
 * $Id: Thread.h,v 1.5 2002/06/03 20:45:38 arnetheduck Exp $
 */

