// CritcalSection.h: interface for the CritcalSection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CRITCALSECTION_H__1226AAB5_254F_4CBD_B384_5E8D3A23C346__INCLUDED_)
#define AFX_CRITCALSECTION_H__1226AAB5_254F_4CBD_B384_5E8D3A23C346__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CriticalSection  
{
	CRITICAL_SECTION cs;
	int i;
public:
	void enter() {
//		dcdebug("enter %d\n", ++i);
		EnterCriticalSection(&cs);
	}
	void leave() {
//		dcdebug("leave %d\n", --i);
		LeaveCriticalSection(&cs);
	}
	CriticalSection() : i(0) {
		InitializeCriticalSection(&cs);
	}
	~CriticalSection() {
		DeleteCriticalSection(&cs);
	}

};

#endif // !defined(AFX_CRITCALSECTION_H__1226AAB5_254F_4CBD_B384_5E8D3A23C346__INCLUDED_)
