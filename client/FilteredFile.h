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

#ifndef _FILTERED_FILE
#define _FILTERED_FILE

#pragma once

#include "File.h"

/**
 * A filter that provides simple read / write buffering.
 */
struct BufferFilter {
	bool operator()(const void* in, size_t& inlen, void* out, size_t& outlen) {
		inlen = outlen = min(inlen, outlen);
		memcpy(out, in, outlen);
		return false;
	}
};

template<class Filter>
class FilteredWriter {
public:
	FilteredWriter(File* aFile) : f(aFile), pos(0) { }
	FilteredWriter(File* aFile, const Filter& aFilter) : f(aFile), filter(aFilter), pos(0) { };
	~FilteredWriter() { }

	u_int32_t flush() throw(Exception) {
		u_int32_t written = 0;

		for(;;) {
			size_t n = BUF_SIZE - pos;
			size_t zero = 0;
            bool more = filter(NULL, zero, buf + pos, n);

			written += f->File::write(buf, pos + n);
			pos = 0;

			if(!more)
				break;
		}
		return written;
	}

	/**
	 * Write data through filter, don't forget to flush once all data has been written.
	 * @param wbuf Data to be written
	 * @param len Length of data
	 * @return Length of data actually written to disk (all data is always consumed)
	 */
	u_int32_t write(const void* wbuf, u_int32_t len) throw(FileException) {
		u_int8_t* wb = (u_int8_t*)wbuf;
		u_int32_t written = 0;
		while(len > 0) {
			size_t n = BUF_SIZE - pos;
			size_t m = len;

			filter(wb, m, buf + pos, n);
			pos += n;
			wb += m;
			len -= m;

			if(pos == BUF_SIZE) {
				written += f->File::write(buf, pos);
			}
		}
		return written;
	}

private:
	enum { BUF_SIZE = 64*1024 };

	File* f;
	Filter filter;

	u_int8_t buf[BUF_SIZE];
	u_int32_t pos;
};

template<class Filter>
class FilteredReader {
public:
	FilteredReader(File* aFile, int64_t maxBytes = -1) : f(aFile), left(maxBytes), pos(0), valid(0), more(true) { }
	FilteredReader(File* aFile, const Filter& aFilter) : f(aFile), filter(aFilter), pos(0), valid(0), more(true) { };
	~FilteredReader() { }

	/**
	* Read data through filter, keep calling until len returns 0.
	* @param rbuf Data buffer
	* @param len Buffer size on entry, length of data in buffer on exit
	* @return Bytes actually read from disk
	*/
	u_int32_t read(void* rbuf, u_int32_t& len) throw(FileException) {
		u_int8_t* rb = (u_int8_t*)rbuf;

		u_int32_t rpos = 0;
		u_int32_t r = 0;
		while(more && rpos < len) {
			if(valid == 0 && left != 0) {
				if(left == -1)
					valid = f->File::read(buf, BUF_SIZE);
				else {
					valid = f->File::read(buf, (size_t)min(left, (int64_t)BUF_SIZE));
					left -= valid;
				}
				r += valid;
			}
			size_t n = len - rpos;
			size_t m = valid - pos;
			more = filter(buf + pos, m, rb + rpos, n);
			pos += m;
			if(pos == valid) {
				valid = pos = 0;
			}
			rpos += n;
		}
		len = rpos;
		return r;
	}

private:
	enum { BUF_SIZE = 64*1024 };

	File* f;
	Filter filter;
	int64_t left;
	u_int8_t buf[BUF_SIZE];
	u_int32_t pos;
	u_int32_t valid;
	bool more;
};

/**
 * FilteredFileWriter passes data through a filter before writing it out to disc.
 * Note that after write() has been called, all file functions (seek etc) become
 * invalid with unspecified behaviour following. This class uses semantics similar
 * to File, to ease usage when filter details are not needed.
 */
template<class Filter>
class FilteredFileWriter : public File {
public:
	FilteredFileWriter(const string& aFileName, int access, int mode, bool aCalcCRC = false) :
		File(aFileName, access, mode, aCalcCRC), filter(this) 
	{
	}

	~FilteredFileWriter() throw(FileException) { filter.flush(); };
	
	virtual void close() throw(FileException) {
		filter.flush();
		File::close();
	}
	
	virtual u_int32_t write(const void* buf, u_int32_t len) throw(FileException) {
		filter.write(buf, len);
		return len;
	}
	u_int32_t write(const string& aString) throw(FileException) { return write((void*)aString.data(), aString.size()); };

private:
	FilteredWriter<Filter> filter;
};

/**
 * Reader, keep in mind that depending on the filter, only read(void*, u_int32_t)
 * will work as expected.
 */
template<class Filter>
class FilteredFileReader : public File {
public:
	FilteredFileReader(const string& aFileName, int access, int mode, bool aCalcCRC = false) :
		File(aFileName, access, mode, aCalcCRC), filter(this) 
	{

	}

	virtual u_int32_t read(void* buf, u_int32_t len) throw(FileException) {
		filter.read(buf, len);
		return len;
	}
private:
	FilteredReader<Filter> filter;
};

#endif // _FILTERED_FILE