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
	HANDLE h;
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

	void wait() {
		WaitForSingleObject(h, INFINITE);
	}

	~Semaphore() {
		CloseHandle(h);
	};

};

#endif // !defined(AFX_SEMAPHORE_H__99141DD0_FECE_4131_BC9B_7BE4CF216874__INCLUDED_)

/**
 * @file Semaphore.h
 * $Id: Semaphore.h,v 1.1 2002/02/25 19:30:55 arnetheduck Exp $
 * @if LOG
 * $Log: Semaphore.h,v $
 * Revision 1.1  2002/02/25 19:30:55  arnetheduck
 * Semaphore class
 *
 * @endif
 */
