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

#if !defined(AFX_CRITCALSECTION_H__1226AAB5_254F_4CBD_B384_5E8D3A23C346__INCLUDED_)
#define AFX_CRITCALSECTION_H__1226AAB5_254F_4CBD_B384_5E8D3A23C346__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CriticalSection  
{
	CRITICAL_SECTION cs;
public:
	void enter() {
		EnterCriticalSection(&cs);
	}
	void leave() {
		LeaveCriticalSection(&cs);
	}
	CriticalSection() {
		InitializeCriticalSection(&cs);
	}
	~CriticalSection() {
		DeleteCriticalSection(&cs);
	}

};

#endif // !defined(AFX_CRITCALSECTION_H__1226AAB5_254F_4CBD_B384_5E8D3A23C346__INCLUDED_)

/**
 * @file CriticalSection.h
 * $Id: CriticalSection.h,v 1.3 2001/12/04 21:50:34 arnetheduck Exp $
 * @if LOG
 * $Log: CriticalSection.h,v $
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
