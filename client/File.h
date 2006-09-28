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

#if !defined(FILE_H)
#define FILE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SettingsManager.h"

#include "Util.h"
#include "Text.h"
#include "Streams.h"

#ifndef _WIN32
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#endif

#ifdef _WIN32
#include "../zlib/zlib.h"
#else
#include <zlib.h>
#endif

class File : public IOStream {
public:
	enum {
		OPEN = 0x01,
		CREATE = 0x02,
		TRUNCATE = 0x04
	};

#ifdef _WIN32
	enum {
		READ = GENERIC_READ,
		WRITE = GENERIC_WRITE,
		RW = READ | WRITE
	};

	File(const string& aFileName, int access, int mode) throw(FileException);

	u_int32_t getLastModified() {
		FILETIME f = {0};
		::GetFileTime(h, NULL, NULL, &f);
		return convertTime(&f);
	}

	static u_int32_t convertTime(FILETIME* f);
	bool isOpen() { return h != INVALID_HANDLE_VALUE; }

	virtual void close() throw();
	virtual int64_t getSize() throw();
	virtual void setSize(int64_t newSize) throw(FileException);

	virtual int64_t getPos() throw();
	virtual void setPos(int64_t pos) throw();
	virtual void setEndPos(int64_t pos) throw();
	virtual void movePos(int64_t pos) throw();

	virtual size_t read(void* buf, size_t& len) throw(Exception);
	virtual size_t write(const void* buf, size_t len) throw(Exception);
	virtual void setEOF() throw(FileException);

	virtual size_t flush() throw(Exception);

	static void deleteFile(const string& aFileName) throw() { ::DeleteFile(Text::toT(aFileName).c_str()); }
	static void renameFile(const string& source, const string& target) throw(FileException);

	static void copyFile(const string& src, const string& target) throw(FileException);
	static int64_t getSize(const string& aFileName) throw();
	static void ensureDirectory(const string& aFile);
	static bool isAbsolute(const string& path);

#else // _WIN32

	enum {
		READ = 0x01,
		WRITE = 0x02,
		RW = READ | WRITE
	};
	File(const string& aFileName, int access, int mode) throw(FileException);

	u_int32_t getLastModified();

	bool isOpen() { return h != -1; }

	void close() throw();
	virtual int64_t getSize() throw(FileException);
	virtual int64_t getPos() throw(FileException);
	virtual void setPos(int64_t pos) throw(FileException);
	virtual void setEndPos(int64_t pos) throw(FileException);
	virtual void movePos(int64_t pos) throw(FileException);

	virtual size_t read(void* buf, size_t& len) throw(FileException);
	virtual size_t write(const void* buf, size_t len) throw(FileException);

	// some ftruncate implementations can't extend files like SetEndOfFile,
	// not sure if the client code needs this...
	int extendFile(int64_t len);
	virtual void setEOF() throw(FileException);

	virtual void setSize(int64_t newSize) throw(FileException);
	virtual size_t flush() throw(Exception);

	static void deleteFile(const string& aFileName) throw() { ::unlink(aFileName.c_str()); }

	/* ::rename seems to have problems when source and target is on different partitions
       from "man 2 rename"
       EXDEV  oldpath  and  newpath are not on the same mounted filesystem.  (Linux permits a
       filesystem to be mounted at multiple points, but rename(2) does not
       work across different mount points, even if the same filesystem is mounted on both.)
    */
	static void renameFile(const string& source, const string& target) throw();
	static void copyFile(const string& source, const string& target) throw(FileException);

	static int64_t getSize(const string& aFileName);
	static void ensureDirectory(const string& aFile);
	static bool isAbsolute(const string& path);
#endif // _WIN32

	virtual ~File() throw() {
		File::close();
	}

	string read(size_t len) throw(FileException);
	string read() throw(FileException);
	void write(const string& aString) throw(FileException) { write((void*)aString.data(), aString.size()); }
	static StringList findFiles(const string& path, const string& pattern);

protected:
#ifdef _WIN32
	HANDLE h;
#else
	int h;
#endif
private:
	File(const File&);
	File& operator=(const File&);
};

#endif // !defined(FILE_H)
