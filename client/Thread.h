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

	Thread() : threadHandle(NULL), threadId(0){ };
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
		LOW,
		NORMAL,
		HIGH
	};
	Thread() { };
	virtual ~Thread() { };
	void start() { };
	void join() { };
	void setThreadPriority(Priority p) { };
	
	static void sleep(u_int32_t millis) { };
	static void yield() { };
	static long safeInc(long* v) { return ++(*v); };
	static long safeDec(long* v) { return --(*v); };
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
#endif
};

#endif // !defined(AFX_THREAD_H__3006956B_7C69_4DAD_9596_A49E1BD007D5__INCLUDED_)

/**
 * @file Thread.h
 * $Id: Thread.h,v 1.3 2002/04/16 16:45:54 arnetheduck Exp $
 */

