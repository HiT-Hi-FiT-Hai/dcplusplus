/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#include "SettingsManager.h"
#include "Exception.h"

#ifndef WIN32
#include <sys/stat.h>
#include <fcntl.h>
#endif

STANDARD_EXCEPTION(FileException);

class CRC32 {
public:
	CRC32() : value(0xffffffffL) { };
	void update(u_int8_t b) { value = (value >> 8) ^ Util::crcTable[(value & 0xff) ^ b]; };
	u_int32_t getValue() const { return ~(value); };
private:
	u_int32_t value;
};


class File  
{
public:
	enum {
		READ = 0x01,
		WRITE = 0x02,
		RW = READ | WRITE
	};
	
	enum {
		OPEN = 0x01,
		CREATE = 0x02,
		TRUNCATE = 0x04
	};

#ifdef WIN32
	File(const string& aFileName, int access, int mode, bool aCalcCRC = false) throw(FileException) : calcCRC(aCalcCRC) {
		dcassert(access == WRITE || access == READ || access == (READ | WRITE));

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
				dcassert(0);
			}
		}
		int a = 0;
		if(access & READ)
			a |= GENERIC_READ;
		if(access & WRITE)
			a |= GENERIC_WRITE;

		h = ::CreateFile(aFileName.c_str(), a, FILE_SHARE_READ, NULL, m, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		
		if(h == INVALID_HANDLE_VALUE) {
			throw FileException(Util::translateError(GetLastError()));
		}

	}

	bool isOpen() { return h != INVALID_HANDLE_VALUE; };

	virtual void close() throw() {
		if(isOpen()) {
			CloseHandle(h);
			h = INVALID_HANDLE_VALUE;
		}
	}
	
	virtual int64_t getSize() throw() {
		DWORD x;
		DWORD l = ::GetFileSize(h, &x);
		
		if( (l == INVALID_FILE_SIZE) && (GetLastError() != NO_ERROR))
			return -1;
		
		return (int64_t)l | ((int64_t)x)<<32;
	}

	virtual int64_t getPos() throw() {
		LONG x = 0;
		DWORD l = ::SetFilePointer(h, 0, &x, FILE_CURRENT);
		
		return (int64_t)l | ((int64_t)x)<<32;
	}		

	virtual void setPos(int64_t pos) throw() {
		LONG x = (LONG) (pos>>32);
		::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_BEGIN);
	}		
	virtual void setEndPos(int64_t pos) throw() {
		LONG x = (LONG) (pos>>32);
		::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_END);
	}		

	virtual void movePos(int64_t pos) throw() {
		LONG x = (LONG) (pos>>32);
		::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_CURRENT);
	}
	
	virtual u_int32_t read(void* buf, u_int32_t len) throw(FileException) {
		DWORD x;
		if(!::ReadFile(h, buf, len, &x, NULL)) {
			throw(FileException(Util::translateError(GetLastError())));
		}
		if(calcCRC) {
			for(DWORD i = 0; i < x; ++i) {
				crc32.update(((u_int8_t*)buf)[i]);
			}
		}
		return x;
	}

	virtual void write(const void* buf, u_int32_t len) throw(FileException) {
		DWORD x;
		if(!::WriteFile(h, buf, len, &x, NULL)) {
			throw FileException(Util::translateError(GetLastError()));
		}
		if(x < len) {
			throw FileException(STRING(DISC_FULL));
		}
		if(calcCRC) {
			for(DWORD i = 0; i < x; ++i) {
				crc32.update(((u_int8_t*)buf)[i]);
			}
		}
	}
	virtual void setEOF() throw(FileException) {
		dcassert(isOpen());
		if(!SetEndOfFile(h)) {
			throw FileException(Util::translateError(GetLastError()));
		}
	}

	static void deleteFile(const string& aFileName) throw() { ::DeleteFile(aFileName.c_str()); };
	static void renameFile(const string& source, const string& target) throw(FileException) { 
		if(!::MoveFile(source.c_str(), target.c_str())) {
			// Can't move, try copy/delete...
			if(!CopyFile(source.c_str(), target.c_str(), FALSE)) {
				throw FileException(Util::translateError(GetLastError()));
			}
			deleteFile(source);
		}
	};

	static int64_t getSize(const string& aFileName) throw() {
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		
		hFind = FindFirstFile(aFileName.c_str(), &fd);
		
		if (hFind == INVALID_HANDLE_VALUE) {
			return -1;
		} else {
			FindClose(hFind);
			return ((int64_t)fd.nFileSizeHigh << 32 | (int64_t)fd.nFileSizeLow);
		}
	}
	
#else // WIN32
	
	File(const string& aFileName, int access, int mode, bool aCalcCRC = false) throw(FileException) : calcCRC(aCalcCRC) {
		dcassert(access == WRITE || access == READ || access == (READ | WRITE));
		
		int m = 0;
		if(access == READ)
			m |= O_RDONLY;
		else if(access == WRITE)
			m |= O_WRONLY;
		else
			m |= O_RDWR;
		
		if(mode & CREATE) {
			m |= O_CREAT;
		}
		if(mode & TRUNCATE) {
			m |= O_TRUNC;
		}
		h = open(aFileName.c_str(), m, S_IRUSR | S_IWUSR);
		if(h == -1)
			throw FileException("Could not open file");
	}	

	bool isOpen() { return h != -1; };

	virtual void close() throw(FileException) {
		if(h != -1) {
			::close(h);
			h = -1;
		}
	}

	virtual int64_t getSize() throw(FileException) {
		struct stat s;
		if(::fstat(h, &s) == -1)
			return -1;
		
		return (int64_t)s.st_size;
	}

	virtual int64_t getPos() throw(FileException) {
		return (int64_t) lseek(h, 0, SEEK_CUR);
	}

	virtual void setPos(int64_t pos) throw(FileException) { lseek(h, (off_t)pos, SEEK_SET); };
	virtual void setEndPos(int64_t pos) throw(FileException) { lseek(h, (off_t)pos, SEEK_END); };
	virtual void movePos(int64_t pos) throw(FileException) { lseek(h, (off_t)pos, SEEK_CUR); };

	virtual u_int32_t read(void* buf, u_int32_t len) throw(FileException) {
		ssize_t x = ::read(h, buf, (size_t)len);
		if(x == -1)
			throw("Read error");
		
		if(calcCRC) {
			for(ssize_t i = 0; i < x; ++i) {
				crc32.update(((u_int8_t*)buf)[i]);
			}
		}

		return (u_int32_t)x;
	}
	
	virtual void write(const void* buf, u_int32_t len) throw(FileException) {
		ssize_t x = ::write(h, buf, len);
		if(x == -1)
			throw FileException("Write error");
		if(x < (ssize_t)len)
			throw FileException("Disk full(?)");

		if(calcCRC) {
			for(ssize_t i = 0; i < x; ++i) {
				crc32.update(((u_int8_t*)buf)[i]);
			}
		}
	}

	/**
	 * @todo fix for unix...
	 */
	virtual void setEOF() throw(FileException) {
	}

	static void deleteFile(const string& aFileName) throw() { ::unlink(aFileName.c_str()); };
	static void renameFile(const string& source, const string& target) throw() { ::rename(source.c_str(), target.c_str()); };

	static int64_t getSize(const string& aFileName) {
		struct stat s;
		if(stat(aFileName.c_str(), &s) == -1)
			return -1;
		
		return s.st_size;
	}
	
#endif // WIN32

	virtual ~File() throw(FileException) {
		File::close();
	}

	string read(u_int32_t len) throw(FileException) {
		string s(len, 0);
		u_int32_t x = read(&s[0], len);
		if(x != len)
			s.resize(x);
		return s;
	}

	string read() throw(FileException) {
		setPos(0);
		int64_t sz = getSize();
		if(sz == -1)
			return Util::emptyString;
		return read((u_int32_t)sz);
	}

	void write(const string& aString) throw(FileException) { write((void*)aString.data(), aString.size()); };

	bool hasCRC32() const { return calcCRC; };
	u_int32_t getCRC32() const { return crc32.getValue(); };

protected:
#ifdef WIN32
	HANDLE h;
#else
	int h;
#endif
	CRC32 crc32;
	bool calcCRC;

};

class BufferedFile : public File {
public:
	BufferedFile(const string& aFileName, int access, int mode, bool aCrc32 = false, int bufSize = SETTING(BUFFER_SIZE)) throw(FileException) : 
		File(aFileName, access, mode, aCrc32), pos(0), size(bufSize*1024) {
		
		buf = new u_int8_t[size];
	}
	
	virtual ~BufferedFile() throw(FileException) {
		flush();
		delete[] buf;
	}

	void flush() throw(FileException) {
		if(pos > 0) {
			try {
				File::write(buf, (u_int32_t)pos);
			} catch(...) {
				pos = 0;
				throw;
			}
			pos = 0;
		}
	}

	virtual void write(const void* aBuf, u_int32_t len) throw(FileException) {
		if( (size == 0) || ((pos == 0) && (len > size)) ) {
			File::write(aBuf, len);
			return;
		}

		u_int32_t pos2 = 0;
		while(pos2 < len) {
			size_t i = min((size_t)(size-pos), (size_t)(len - pos2));
			
			memcpy(buf+pos, ((char*)aBuf)+pos2, i);
			pos += (int)i;
			pos2 += (u_int32_t)i;
			dcassert(pos <= size);
			dcassert(pos2 <= len);

			if(pos == size)
				flush();
		}
	}

	void write(const string& aString) throw(FileException) { write((void*)aString.data(), aString.size()); };
	
	virtual void close() throw(FileException) { if(isOpen()) { flush(); File::close(); } };
	virtual int64_t getSize() throw(FileException) { flush(); return File::getSize(); };
	virtual int64_t getPos() throw(FileException) { flush(); return File::getPos(); };
	virtual void setPos(int64_t aPos) throw(FileException) { flush(); File::setPos(aPos); };
	virtual void setEndPos(int64_t aPos) throw(FileException) { flush(); File::setEndPos(aPos); };
	virtual void movePos(int64_t aPos) throw(FileException) { flush(); File::movePos(aPos); };
	virtual u_int32_t read(void* aBuf, u_int32_t len) throw(FileException) { flush(); return File::read(aBuf, len); };
	virtual void setEOF() throw(FileException) { flush(); File::setEOF(); };

private:
	u_int8_t* buf;
	u_int32_t pos;
	u_int32_t size;
};

class SizedFile : public BufferedFile {
public:
	SizedFile(int64_t aExpectedSize, const string& aFileName, int access, int mode, bool aCrc32 = false, int bufSize = SETTING(BUFFER_SIZE)) throw(FileException) : 
		BufferedFile(aFileName, access, mode, aCrc32, bufSize)
	{
		int64_t tmp = getPos();
		setPos(aExpectedSize);
		setEOF();
		setPos(tmp);
	}

	virtual ~SizedFile() throw(FileException) {
		close();
	}

	virtual void close() throw(FileException) {
		if(isOpen()) {
			BufferedFile::close();
		}
	}
private:
};
#endif // !defined(AFX_FILE_H__CB551CD7_189C_4175_922E_8B00B4C8D6F1__INCLUDED_)

/**
 * @file
 * $Id: File.h,v 1.22 2003/11/06 18:54:39 arnetheduck Exp $
 */

