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

#ifdef _DEBUG
#define dcdebug ATLTRACE
#define dcassert(exp) ATLASSERT(exp)
#else //_DEBUG
#define dcdebug ATLTRACE
#define dcassert(exp) __assume(exp)
#endif //_DEBUG

typedef vector<string> StringList;
typedef StringList::iterator StringIter;
typedef StringList::const_iterator StringIterC;

typedef map<string, string> StringMap;
typedef StringMap::iterator StringMapIter;

#include "resource.h"
#include "Settings.h"
#include "Version.h"


#define GETSET(type, name, name2) private: type name; public: type get##name2() const { return name; }; void set##name2(type a##name2) { name = a##name2; };
#define GETSETREF(type, name, name2) private: type name; public: const type& get##name2() const { return name; }; void set##name2(const type& a##name2) { name = a##name2; };


/**
 * This message is posted when something's changed about the hub. It's necessary, because using a SendMessage
 * might block the main window thread if it's trying to access a resource that the Speaker owns.
 * wParam Specifies what actually happened
 */
#define WM_SPEAKER (WM_USER + 100)		


/**
 * @file DCPlusPlus.h
 * $Id: DCPlusPlus.h,v 1.8 2002/01/11 14:52:57 arnetheduck Exp $
 * @if LOG
 * $Log: DCPlusPlus.h,v $
 * Revision 1.8  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.7  2002/01/10 12:33:14  arnetheduck
 * Various fixes
 *
 * Revision 1.6  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.5  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.4  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.3  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.2  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */

