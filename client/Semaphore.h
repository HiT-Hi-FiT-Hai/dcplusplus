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
	Semaphore() {
		h = CreateSemaphore(NULL, 0, MAXLONG, NULL);
	};

	operator HANDLE() {
		return h;
	}

	void signal() {
		ReleaseSemaphore(h, 1, NULL);
	}

	bool wait() { return WaitForSingleObject(h, INFINITE) == WAIT_OBJECT_0; };
	bool wait(u_int32_t millis) { return WaitForSingleObject(h, millis) == WAIT_OBJECT_0; };

	~Semaphore() {
		CloseHandle(h);
	};

private:
	HANDLE h;
#else
public:
	Semaphore() { };
	~Semaphore() { };
	void signal() { };
	bool wait() { };
	bool wait(u_int32_t) { return false; };

#endif
};

#endif // !defined(AFX_SEMAPHORE_H__99141DD0_FECE_4131_BC9B_7BE4CF216874__INCLUDED_)

/**
 * @file Semaphore.h
 * $Id: Semaphore.h,v 1.4 2002/05/03 18:53:02 arnetheduck Exp $
 */
