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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "File.h"

template<class Filter, bool managed>
class CalcOutputStream : public OutputStream {
public:
	using OutputStream::write;

	CalcOutputStream(OutputStream* aStream) : s(aStream) { }
	virtual ~CalcOutputStream() { if(managed) delete s; }

	size_t flush() throw(Exception) {
		return s->flush();
	}

	size_t write(const void* buf, size_t len) throw(Exception) {
		filter(buf, len);
		return s->write(buf, len);
	}


	const Filter& getFilter() const { return filter; }
private:
	OutputStream* s;
	Filter filter;
};

template<class Filter, bool managed> 
class CalcInputStream : public InputStream {
public:
	CalcInputStream(InputStream* aStream) : s(aStream) { }
	virtual ~CalcInputStream() { if(managed) delete s; }

	size_t read(void* buf, size_t& len) throw(Exception) {
		size_t x = s->read(buf, len);
		filter(buf, x);
		return x;
	}

	const Filter& getFilter() const { return filter; }
private:
	InputStream* s;
	Filter filter;
};

template<class Filter, bool manage>
class FilteredOutputStream : public OutputStream {
public:
	using OutputStream::write;

	FilteredOutputStream(OutputStream* aFile) : f(aFile), flushed(false) { }
	~FilteredOutputStream() { if(manage) delete f; }

	size_t flush() throw(Exception) {
		if(flushed)
			return 0;

		flushed = true;
		size_t written = 0;

		for(;;) {
			size_t n = BUF_SIZE;
			size_t zero = 0;
            bool more = filter(NULL, zero, buf, n);

			written += f->write(buf, n);

			if(!more)
				break;
		}
		return written + f->flush();
	}

	size_t write(const void* wbuf, size_t len) throw(Exception) {
		if(flushed)
			throw Exception("No filtered writes after flush");

		u_int8_t* wb = (u_int8_t*)wbuf;
		size_t written = 0;
		while(len > 0) {
			size_t n = BUF_SIZE;
			size_t m = len;

			bool more = filter(wb, m, buf, n);
			wb += m;
			len -= m;

			written += f->write(buf, n);

			if(!more) {
				if(len > 0) {
					throw Exception("Garbage data after end of stream");
				}
				return written;
			}
		}
		return written;
	}

private:
	static const size_t BUF_SIZE = 64*1024;

	OutputStream* f;
	Filter filter;

	u_int8_t buf[BUF_SIZE];
	bool flushed;
};

template<class Filter, bool managed>
class FilteredInputStream : public InputStream {
public:
	FilteredInputStream(InputStream* aFile) : f(aFile), pos(0), valid(0), more(true) { }
	virtual ~FilteredInputStream() { if(managed) delete f; }

	/**
	* Read data through filter, keep calling until len returns 0.
	* @param rbuf Data buffer
	* @param len Buffer size on entry, bytes actually read on exit
	* @return Length of data in buffer
	*/
	size_t read(void* rbuf, size_t& len) throw(Exception) {
		u_int8_t* rb = (u_int8_t*)rbuf;

		size_t totalRead = 0;
		size_t totalProduced = 0;

		while(more && totalProduced < len) {
			size_t curRead = BUF_SIZE;
			if(valid == 0) {
				dcassert(pos == 0);
				valid = f->read(buf, curRead);
				totalRead += curRead;
			}

			size_t n = len - totalProduced;
			size_t m = valid - pos;
			more = filter(buf + pos, m, rb, n);
			pos += m;
			if(pos == valid) {
				valid = pos = 0;
			}
			totalProduced += n;
			rb += n;
		}
		len = totalRead;
		return totalProduced;
	}

private:
	static const size_t BUF_SIZE = 64*1024;

	InputStream* f;
	Filter filter;
	u_int8_t buf[BUF_SIZE];
	size_t pos;
	size_t valid;
	bool more;
};

#endif // _FILTERED_FILE
