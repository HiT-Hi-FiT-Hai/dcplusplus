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

#if !defined(AFX_FILE_H__CB551CD7_189C_4175_922E_8B00B4C8D6F1__INCLUDED_)
#define AFX_FILE_H__CB551CD7_189C_4175_922E_8B00B4C8D6F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Exception.h"

STANDARD_EXCEPTION(FileException);

class File  
{
public:
	enum {
		READ = GENERIC_READ,
		WRITE = GENERIC_WRITE
	};
	
	enum {
		OPEN = 0x01,
		CREATE = 0x02,
		TRUNCATE = 0x04
	};

	File(const string& aFileName, int access = WRITE, int mode = OPEN) throw(FileException) {
		int m = 0;
		if(mode & OPEN) {
			if(mode & CREATE) {
				m = (mode & TRUNCATE) ? CREATE_ALWAYS : OPEN_ALWAYS;
			} else {
				m = (mode & TRUNCATE) ? TRUNCATE_EXISTING : OPEN_EXISTING;
			}
		} else {
			if(mode & CREATE) {
				m = (mode & TRUNCATE) ? CREATE_ALWAYS : CREATE_NEW;
			} else {
				throw FileException("Bad creation mode");
			}
		}

		h = ::CreateFile(aFileName.c_str(), access, FILE_SHARE_READ, NULL, m, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		
		if(h == INVALID_HANDLE_VALUE) {
			throw FileException(Util::translateError(GetLastError()));
		}
	}

	virtual ~File() {
		CloseHandle(h);
	}

	LONGLONG getSize() {
		DWORD x;
		DWORD l = ::GetFileSize(h, &x);

		return (LONGLONG)l | ((LONGLONG)x)<<32;
	}

	LONGLONG getPos() {
		LONG x = 0;
		DWORD l = ::SetFilePointer(h, 0, &x, FILE_CURRENT);

		return (LONGLONG)l | ((LONGLONG)x)<<32;
	}

	void setPos(LONGLONG pos) {
		LONG x = (LONG) (pos>>32);
		::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_BEGIN);
	}

	void movePos(LONGLONG pos) {
		LONG x = (LONG) (pos>>32);
		::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_CURRENT);
	}

	DWORD read(void* buf, DWORD len) throw(FileException) {
		DWORD x;
		if(!::ReadFile(h, buf, len, &x, NULL)) {
			throw(FileException(Util::translateError(GetLastError())));
		}
		return x;
	}

	string read(DWORD len) throw(FileException) {
		char* buf = new char[len];
		DWORD x;
		try {
			x = read(buf, len);
		} catch(...) {
			delete buf;
			throw;
		}
		string tmp(buf, x);
		delete buf;
		return tmp;
	}

	string read() {
		setPos(0);
		return read(getSize());
	}

	DWORD write(const void* buf, DWORD len) throw(FileException) {
		DWORD x;
		if(!::WriteFile(h, buf, len, &x, NULL)) {
			throw(FileException(Util::translateError(GetLastError())));
		}
		return x;
	}

	DWORD write(const string& aString) {
		return write((void*)aString.data(), aString.size());
	}

private:
	HANDLE h;

};

#endif // !defined(AFX_FILE_H__CB551CD7_189C_4175_922E_8B00B4C8D6F1__INCLUDED_)

/**
 * @file File.h
 * $Id: File.h,v 1.1 2002/01/19 13:09:10 arnetheduck Exp $
 * @if LOG
 * $Log: File.h,v $
 * Revision 1.1  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * @endif
 */

