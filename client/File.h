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
		close();
	}

	virtual void close() {
		if(h != INVALID_HANDLE_VALUE) {
			CloseHandle(h);
			h = INVALID_HANDLE_VALUE;
		}
	}
	virtual LONGLONG getSize() {
		DWORD x;
		DWORD l = ::GetFileSize(h, &x);

		return (LONGLONG)l | ((LONGLONG)x)<<32;
	}

	virtual LONGLONG getPos() {
		LONG x = 0;
		DWORD l = ::SetFilePointer(h, 0, &x, FILE_CURRENT);

		return (LONGLONG)l | ((LONGLONG)x)<<32;
	}

	virtual void setPos(LONGLONG pos) {
		LONG x = (LONG) (pos>>32);
		::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_BEGIN);
	}

	virtual void movePos(LONGLONG pos) {
		LONG x = (LONG) (pos>>32);
		::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_CURRENT);
	}

	virtual DWORD read(void* buf, DWORD len) throw(FileException) {
		DWORD x;
		if(!::ReadFile(h, buf, len, &x, NULL)) {
			throw(FileException(Util::translateError(GetLastError())));
		}
		return x;
	}

	virtual string read(DWORD len) throw(FileException) {
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

	virtual string read() throw(FileException) {
		setPos(0);
		return read((DWORD)getSize());
	}

	virtual DWORD write(const void* buf, DWORD len) throw(FileException) {
		DWORD x;
		if(!::WriteFile(h, buf, len, &x, NULL)) {
			throw(FileException(Util::translateError(GetLastError())));
		}
		if(x < len) {
			throw(FileException("Disk full?"));
		}
		return x;
	}

	virtual DWORD write(const string& aString) throw(FileException) {
		return write((void*)aString.data(), aString.size());
	}
	
	static void deleteFile(const string& aFileName) {
		::DeleteFile(aFileName.c_str());
	}
	static void renameFile(const string& source, const string& target) {
		::MoveFile(source.c_str(), target.c_str());
	}
	
	static LONGLONG getSize(const string& aFileName) {
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		
		hFind = FindFirstFile(aFileName.c_str(), &fd);
		
		if (hFind == INVALID_HANDLE_VALUE) {
			return -1;
		} else {
			FindClose(hFind);
			return ((ULONGLONG)fd.nFileSizeHigh << 32 | (ULONGLONG)fd.nFileSizeLow);
		}
	}
private:
	HANDLE h;

};

class BufferedFile : public File {
public:
	BufferedFile(const string& aFileName, int access = WRITE, int mode = OPEN, DWORD bufSize = SETTING(BUFFER_SIZE)) throw(FileException) : File(aFileName, access, mode), pos(0), size(bufSize*1024) {
		buf = new BYTE[size];
	}
	
	virtual ~BufferedFile() {
		flush();
		delete buf;
	}

	virtual void flush() throw(FileException) {
		if(pos > 0) {
			File::write(buf, pos);
		}
		pos = 0;
	}

	virtual DWORD write(const void* aBuf, DWORD len) throw(FileException) {
		while(len) {
			if(pos == 0 && len > size) {
				File::write(aBuf, len);
				len = 0;
			} else {
				int i = min(size-pos, len);
				memcpy(buf+pos, aBuf, i);
				pos += i;
				dcassert(pos <= size);
				len -= i;

				if(pos == size)
					flush();
				
			}
		}
		return len;
	}

	virtual DWORD write(const string& aStr) throw(FileException) {
		return write(aStr.c_str(), aStr.size());
	}

	virtual void close() { flush(); File::close(); };
	virtual LONGLONG getSize() { flush(); return File::getSize(); };
	virtual LONGLONG getPos() { flush(); return File::getPos(); };
	virtual void setPos(LONGLONG pos) { flush(); File::setPos(pos); };	
	virtual void movePos(LONGLONG pos) { flush(); File::movePos(pos); };
	virtual DWORD read(void* aBuf, DWORD len) throw(FileException) { flush(); return File::read(aBuf, len); };
	virtual string read(DWORD len) throw(FileException) { flush(); return File::read(len); };	
	virtual string read() throw(FileException) {  flush(); return File::read(); };
	
private:
	BYTE* buf;
	DWORD pos;
	DWORD size;
};

#endif // !defined(AFX_FILE_H__CB551CD7_189C_4175_922E_8B00B4C8D6F1__INCLUDED_)

/**
 * @file File.h
 * $Id: File.h,v 1.6 2002/03/11 22:58:54 arnetheduck Exp $
 * @if LOG
 * $Log: File.h,v $
 * Revision 1.6  2002/03/11 22:58:54  arnetheduck
 * A step towards internationalization
 *
 * Revision 1.5  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.4  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.3  2002/02/01 02:00:29  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.2  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.1  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * @endif
 */

