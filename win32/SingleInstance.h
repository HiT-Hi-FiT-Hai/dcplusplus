/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(DCPLUSPLUS_WIN32_SINGLE_INSTANCE_H)
#define DCPLUSPLUS_WIN32_SINGLE_INSTANCE_H

class SingleInstance
{
	DWORD  LastError;
	HANDLE hMutex;

public:
	static const UINT WMU_WHERE_ARE_YOU;

	SingleInstance(const TCHAR* strMutexName)
	{
		// strMutexName must be unique
		hMutex = ::CreateMutex(NULL, FALSE, strMutexName);
		LastError = ::GetLastError();
	}

	~SingleInstance()
	{
		if(hMutex) {
			CloseHandle(hMutex);
			hMutex = NULL;
		}
	}

	bool isRunning() { return (ERROR_ALREADY_EXISTS == LastError); }
};

#endif // !defined(SINGLE_INSTANCE_H)
