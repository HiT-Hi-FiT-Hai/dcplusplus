/* 
 * Copyright (C) 2002
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

#ifndef SINGLEINSTANCE_H
#define SINGLEINSTANCE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WMU_WHERE_ARE_YOU_MSG _T("WMU_WHERE_ARE_YOU-{885D4B75-6606-4add-A8DE-EEEDC04181F1}")
const UINT WMU_WHERE_ARE_YOU = ::RegisterWindowMessage(_T("WMU_WHERE_ARE_YOU_MSG"));

class SingleInstance
{
	DWORD  LastError;
	HANDLE hMutex;

public:
	SingleInstance(char* strMutexName)
	{
		// strMutexName must be unique
		hMutex = CreateMutex(NULL, FALSE, strMutexName);
		LastError = GetLastError();
	}

	~SingleInstance()
	{
		if(hMutex) {
			CloseHandle(hMutex);
			hMutex = NULL;
		}
	}

	BOOL IsAnotherInstanceRunning() { return (ERROR_ALREADY_EXISTS == LastError); }
};

#endif // SINGLEINSTANCE_H
