/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#ifndef _DCPLUSPLUS_H
#define _DCPLUSPLUS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _DEBUG

#ifdef _WIN32
// Warning C4130: '==' : logical operation on address of string constant
#pragma warning (disable: 4130)
#endif

inline void CDECL debugTrace(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	
#ifdef _WIN32
	char buf[512];
	
	_vsnprintf(buf, sizeof(buf), format, args);
	OutputDebugStringA(buf);
#else // _WIN32
	vprintf(format, args);
#endif // _WIN32
	va_end(args);
}

#define dcdebug debugTrace
#ifdef _WIN32
#define dcassert(exp) \
do { if (!(exp)) { \
	dcdebug("Assertion hit in %s(%d): " #exp "\n", __FILE__, __LINE__); \
	if(1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, #exp)) \
_CrtDbgBreak(); } } while(false)
#define dcasserta(exp) dcassert(0)
#else
#include <assert.h>
#define dcasserta(exp) assert(exp)
#define dcassert(exp) assert(exp)
#endif
#define dcdrun(exp) exp
#else //_DEBUG
#ifdef _WIN32
#define dcasserta(exp) __assume(exp)
#else
#define dcasserta(exp)
#endif // _WIN32
#define dcdebug if (false) printf
#define dcassert(exp) 
#define dcdrun(exp)
#endif //_DEBUG

// Make sure we're using the templates from algorithm...
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

typedef vector<string> StringList;
typedef StringList::iterator StringIter;
typedef StringList::const_iterator StringIterC;

typedef pair<string, string> StringPair;
typedef vector<StringPair> StringPairList;
typedef StringPairList::iterator StringPairIter;

typedef HASH_MAP<string, string> StringMap;
typedef StringMap::iterator StringMapIter;

typedef vector<wstring> WStringList;
typedef WStringList::iterator WStringIter;
typedef WStringList::const_iterator WStringIterC;

typedef pair<wstring, wstring> WStringPair;
typedef vector<WStringPair> WStringPairList;
typedef WStringPairList::iterator WStringPairIter;

typedef HASH_MAP<wstring, wstring> WStringMap;
typedef WStringMap::iterator WStringMapIter;

#ifdef UNICODE

typedef wstring tstring;
typedef WStringList TStringList;
typedef WStringIter TStringIter;

typedef WStringPair TStringPair;
typedef WStringPairIter TStringPairIter;
typedef WStringPairList TStringPairList;

typedef WStringMap TStringMap;
typedef WStringMapIter TStringMapIter;
#else
typedef string tstring;
typedef StringList TStringList;
typedef StringIter TStringIter;

typedef StringPair TStringPair;
typedef StringPairIter TStringPairIter;
typedef StringPairList TStringPairList;

typedef StringMap TStringMap;
typedef StringMapIter TStringMapIter;
#endif

#include "version.h"

extern void startup(void (*f)(void*, const string&), void* p);
extern void shutdown();

template<typename T, bool flag> struct ReferenceSelector {
	typedef T ResultType;
};
template<typename T> struct ReferenceSelector<T,true> {
	typedef const T& ResultType;
};

template<typename T> class IsOfClassType {
private:
	struct Overloads {
		template<typename U> static char check(int U::*);
		template<typename U> static float check(...);
	};
public:
	enum { Result = (sizeof(IsOfClassType<T>::Overloads::check<T>(0)) == 1) };
};

template<typename T> struct TypeTraits {
	typedef typename ReferenceSelector<T,IsOfClassType<T>::Result || 
		(sizeof(T)>sizeof(char*))>::ResultType ParameterType;
};

#define GETSET(type, name, name2) \
private: type name; \
public: TypeTraits<type>::ParameterType get##name2() const { return name; }; \
	void set##name2(TypeTraits<type>::ParameterType a##name2) { name = a##name2; };

#define LIT(x) x, (sizeof(x)-1)

#endif // _DCPLUSPLUS_H

/**
 * @file
 * $Id: DCPlusPlus.h,v 1.42 2004/10/17 12:51:30 arnetheduck Exp $
 */

