// Semaphore.h: interface for the Semaphore class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEMAPHORE_H__99141DD0_FECE_4131_BC9B_7BE4CF216874__INCLUDED_)
#define AFX_SEMAPHORE_H__99141DD0_FECE_4131_BC9B_7BE4CF216874__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Semaphore  
{
#ifdef WIN32
public:
	Semaphore() throw() {
		h = CreateSemaphore(NULL, 0, MAXLONG, NULL);
	};

	void signal() throw() {
		ReleaseSemaphore(h, 1, NULL);
	}

	bool wait() throw() { return WaitForSingleObject(h, INFINITE) == WAIT_OBJECT_0; };
	bool wait(u_int32_t millis) throw() { return WaitForSingleObject(h, millis) == WAIT_OBJECT_0; };

	~Semaphore() throw() {
		CloseHandle(h);
	};

private:
	HANDLE h;
#else
public:
	Semaphore() throw() : count(0) { pthread_cond_init(&cond, NULL); };
	~Semaphore() throw() { pthread_cond_destroy(&cond); };
	void signal() throw() { 
		Lock l(cs);
		count++;
		pthread_cond_signal(&cond);
	};

	bool wait() throw() { 
		Lock l(cs);
		if(count == 0) {
			pthread_cond_wait(&cond, &cs.getMutex());
		}
		count--;
		return true;
	};
	bool wait(u_int32_t millis) throw() { 
		Lock l(cs);
		if(count == 0) {
			timeval timev;
			timespec t;
			gettimeofday(&timev, NULL);
			millis+=timev.tv_usec/1000;
			t.tv_sec = timev.tv_sec + (millis/1000);
			t.tv_nsec = (millis%1000)*1000*1000;
			int ret = pthread_cond_timedwait(&cond, &cs.getMutex(), &t);
			if(ret != 0) {
				return false;
			}
		}
		count--;
		return true;
	};

private:
	pthread_cond_t cond;
	CriticalSection cs;
	int count;
#endif
};

#endif // !defined(AFX_SEMAPHORE_H__99141DD0_FECE_4131_BC9B_7BE4CF216874__INCLUDED_)

/**
 * @file Semaphore.h
 * $Id: Semaphore.h,v 1.5 2002/06/03 20:45:38 arnetheduck Exp $
 */
