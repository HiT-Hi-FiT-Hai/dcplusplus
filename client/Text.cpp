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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "Text.h"

char Text::asciiLower[128];
wchar_t Text::lower[65536];

void Text::initialize() {
	for(size_t i = 0; i < 65536; ++i) {
#ifdef _WIN32
		lower[i] = (wchar_t)CharLowerW((LPWSTR)i);
#else
#warning FIXME for non-ascii char codes
		lower[i] = (char)tolower(i);
#endif
	}

	for(size_t i = 0; i < 128; ++i) {
		asciiLower[i] = (char)lower[i];
	}

}

int Text::utf8ToWc(const char* str, wchar_t& c) {
	int l = 0;
	if(str[0] & 0x80) {
		if(str[0] & 0x40) {
			if(str[0] & 0x20) {
				if(str[1] == 0 || str[2] == 0 ||
					!((((unsigned char)str[1]) & ~0x3f) == 0x80) ||
					!((((unsigned char)str[2]) & ~0x3f) == 0x80))
				{
					return -1;
				}
				c = ((wchar_t)(unsigned char)str[0] & 0xf) << 12 |
					((wchar_t)(unsigned char)str[1] & 0x3f) << 6 |
					((wchar_t)(unsigned char)str[2] & 0x3f);
				l = 3;
			} else {
				if(str[1] == 0 ||
					!((((unsigned char)str[1]) & ~0x3f) == 0x80)) 
				{
					return -1;
				}
				c = ((wchar_t)(unsigned char)str[0] & 0x1f) << 6 |
					((wchar_t)(unsigned char)str[1] & 0x3f);
				l = 2;
			}
		} else {
			return -1;
		}
	} else {
		c = (unsigned char)str[0];
		l = 1;
	}

	return l;
}

void Text::wcToUtf8(wchar_t c, string& str) {
	if(c >= 0x0800) {
		str += (char)(0x80 | 0x40 | 0x20  | (c >> 12));
		str += (char)(0x80 | ((c >> 6) & 0x3f));
		str += (char)(0x80 | (c & 0x3f));
	} else if(c >= 0x0080) {
		str += (char)(0x80 | 0x40 | (c >> 6));
		str += (char)(0x80 | (c & 0x3f));
	} else {
		str += (char)c;
	}
}

string& Text::acpToUtf8(const string& str, string& tmp) throw() {
	wstring wtmp;
	return wideToUtf8(acpToWide(str, wtmp), tmp);
}

wstring& Text::acpToWide(const string& str, wstring& tmp) throw() {
#ifdef _WIN32
	int n = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), (int)str.length(), NULL, 0);
	if(n == 0) {
		return tmp;
	}

	tmp.resize(n);
	n = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), (int)str.length(), &tmp[0], n);
	if(n == 0) {
		tmp.clear();
		return tmp;
	}
	return tmp;
#else
#warning FIXME
#endif
}

string& Text::wideToUtf8(const wstring& str, string& tgt) throw() {
	string::size_type n = str.length();
	for(string::size_type i = 0; i < n; ++i) {
		wcToUtf8(str[i], tgt);
	}
	return tgt;
}

string& Text::wideToAcp(const wstring& str, string& tmp) throw() {
#ifdef _WIN32
	int n = WideCharToMultiByte(CP_ACP, 0, str.c_str(), (int)str.length(), NULL, 0, NULL, NULL);
	if(n == 0) {
		return tmp;
	}

	tmp.resize(n);
	n = WideCharToMultiByte(CP_ACP, 0, str.c_str(), (int)str.length(), &tmp[0], n, NULL, NULL);
	if(n == 0) {
		tmp.clear();
		return tmp;
	}
	return tmp;
#else
#warning FIXME
#endif
}

string& Text::utf8ToAcp(const string& str, string& tmp) throw() {
	wstring wtmp;
	return wideToAcp(utf8ToWide(str, wtmp), tmp);
}

wstring& Text::utf8ToWide(const string& str, wstring& tgt) throw() {
	tgt.reserve(str.length());
	string::size_type n = str.length();
	for(string::size_type i = 0; i < n; ) {
		wchar_t c = 0;
		int x = utf8ToWc(str.c_str() + i, c);
		if(x == -1) {
			i++;
		} else {
			i+=x;
			tgt += c;
		}
	}
	return tgt;
}

wstring& Text::toLower(const wstring& str, wstring& tmp) throw() {
	tmp.reserve(str.length());
	wstring::const_iterator end = str.end();
	for(wstring::const_iterator i = str.begin(); i != end; ++i) {
		tmp += toLower(*i);
	}
	return tmp;
}

string& Text::toLower(const string& str, string& tmp) throw() {
	tmp.reserve(str.length());
	const char* end = &str[0] + str.length();
	for(const char* p = &str[0]; p < end;) {
		wchar_t c = 0;
		int n = utf8ToWc(p, c);
		if(n == -1) {
			p++;
		} else {
			p += n;
			wcToUtf8(toLower(c), tmp);
		}
	}
	return tmp;
}

/**
 * @file
 * $Id: Text.cpp,v 1.4 2004/09/23 09:06:26 arnetheduck Exp $
 */
