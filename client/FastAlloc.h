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

#ifndef _FAST_ALLOC
#define _FAST_ALLOC

#ifdef HAS_STLPORT
struct FastAllocBase {
	/** This one should allocate one byte at a time. */
	static allocator<u_int8_t> alloc;
};

/** 
 * Fast new/delete replacements that use the node allocator from STLPort to 
 * alloc/dealloc memory. Only makes sense if STLPort is used...
 */
template<class T>
struct FastAlloc : private FastAllocBase {
	// Custom new & delete that (hopefully) use the node allocator
	static void* operator new(size_t s) {
		if(s != sizeof(T))
			return ::operator new(s);
		return alloc.allocate(s);
	}
	static void operator delete(void* m, size_t s) {
		if (s != sizeof(T)) {
			::operator delete(m);
		} else {
			alloc.deallocate((u_int8_t*)m, s);
		}
	}
};
#else
template<class T>
struct FastAlloc {
	// Empty
};
#endif

#endif // _FAST_ALLOC
/**
 * @file
 * $Id: FastAlloc.h,v 1.1 2004/01/28 20:19:20 arnetheduck Exp $
 */
