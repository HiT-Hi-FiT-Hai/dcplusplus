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

typedef vector<string> StringList;
typedef StringList::iterator StringIter;

#include "resource.h"
#include "Settings.h"
#include "Version.h"

#ifdef _DEBUG
#define dcdebug ATLTRACE
#define dcassert(exp) ATLASSERT(exp)
#else //_DEBUG
#define dcdebug ATLTRACE
#define dcassert(exp) __assume(exp)
#endif //_DEBUG

/**
 * @file DCPlusPlus.h
 * $Id: DCPlusPlus.h,v 1.1 2001/11/21 17:33:20 arnetheduck Exp $
 * @if LOG
 * $Log: DCPlusPlus.h,v $
 * Revision 1.1  2001/11/21 17:33:20  arnetheduck
 * Initial revision
 *
 * @endif
 */

