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

#if !defined(AFX_POINTER_H__FCC38D23_858F_43AC_BF23_73FD708FDC82__INCLUDED_)
#define AFX_POINTER_H__FCC38D23_858F_43AC_BF23_73FD708FDC82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class PointerBase
{
public:
	void inc() {
		dcassert(ref>=0);
#ifdef WIN32
		InterlockedIncrement(&ref);
#endif
	}

	void dec() {
		dcassert(ref>0);
		
#ifdef WIN32
		if ( (InterlockedDecrement(&ref)) == 0 ) {
			//dcdebug("Smart Object at 0x%08x deleted\n", this);
			delete this;
		}
#endif
	}
	bool unique() {
		return (ref == 1);
	}
	
protected:
	PointerBase() : ref(0) { };
	
	virtual ~PointerBase() {
		dcassert(!ref);
	}

private:
	long ref;
};

/**
 * Note; don't forget to make the destructor virtual if deriving from this class
 */
template <class T>
class Pointer
{
public:
	Pointer ( PointerBase *aBase = 0) : base(aBase) {	
		if ( base ) {
			base->inc();
		}
	}
	
	Pointer( const Pointer &rhs ) : base(rhs.base) {	   
		if ( base ) {
			base->inc();
		}
	}
	
	Pointer &operator =( const Pointer &rhs ) {
		if ( rhs.base ) {
			rhs.base->inc();
		}
		
		if ( base ) {
			base->dec();
		}
		
		base = rhs.base;
		return *this;
	}

	Pointer &operator =( T* rhs ) {
		if (rhs) {
			rhs->inc();
			if ( base ) {
				base->dec();
			}
			base = rhs;
		}
		
		
		return *this;
	}
	
	~Pointer() { 
		if ( base ) {
			base->dec();
		}
	}
	
	T*			operator->()		  { return		asT();	}
	T&			operator* ()		  { return	   *asT();	}
	const T*	operator->()  const   { return		asT();	}
	const T&	operator* ()  const   { return	   *asT();	}
	
	operator		  bool()  const   { return base != NULL; }
	
	operator T*() { return (T*)base; };
	operator const T*() const { return (T*)base; };
	
	bool operator==(T* rhs) const { return (T*)base == rhs; };
	bool operator==(const Pointer& rhs) const { return base == rhs.base; };
	bool operator<(T* rhs) const { return (T*)base < rhs; };
	bool operator<(const Pointer& rhs) const { return base < rhs.base; };
	bool operator>(T* rhs) const { return (T*)base > rhs; };
	bool operator>(const Pointer& rhs) const { return base > rhs.base; };
	

	static void swap ( Pointer &lhs, Pointer &rhs ) {
		PointerBase *temp = lhs.base;
		lhs.base = rhs.base;
		rhs.base = temp;
	}
	
	void release() {
		if ( base ) {
			base->dec();
			base = 0;
		}
	}
private:
	PointerBase* base;
	
	T* asT () {	
		dcassert(base);
		return (T*)base;	
	}

	const T* asT()	const {	
		dcassert(base);
		return (T*)base;	
	}
};

template <class T>
bool operator==(T* lhs, const Pointer<T>& rhs) { return rhs == lhs; };
template <class T>
bool operator<(T* lhs, const Pointer<T>& rhs) { return rhs > lhs; };
template <class T>
bool operator>(T* lhs, const Pointer<T>& rhs) { return rhs < lhs; };


#endif // !defined(AFX_POINTER_H__FCC38D23_858F_43AC_BF23_73FD708FDC82__INCLUDED_)

/**
 * @file Pointer.h
 * $Id: Pointer.h,v 1.8 2002/04/09 18:43:28 arnetheduck Exp $
 * @if LOG
 * $Log: Pointer.h,v $
 * Revision 1.8  2002/04/09 18:43:28  arnetheduck
 * Major code reorganization, to ease maintenance and future port...
 *
 * Revision 1.7  2002/02/28 00:10:47  arnetheduck
 * Some fixes to the new user model
 *
 * Revision 1.6  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.5  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.4  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.2  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.1  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * @endif
 */

