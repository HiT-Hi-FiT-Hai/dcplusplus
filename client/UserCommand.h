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

#if !defined(AFX_USERCOMMAND_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_)
#define AFX_USERCOMMAND_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"

class UserCommand : public Flags {
public:
	typedef vector<UserCommand> List;
	typedef List::iterator Iter;

	enum {
		TYPE_SEPARATOR,
		TYPE_RAW,
		TYPE_RAW_ONCE,
		TYPE_CLEAR = 255
	};

	enum {
		CONTEXT_HUB = 0x01,
		CONTEXT_CHAT = 0x02,
		CONTEXT_SEARCH = 0x04,
		CONTEXT_MASK = CONTEXT_HUB | CONTEXT_CHAT | CONTEXT_SEARCH,
	};

	enum {
		FLAG_NOSAVE = 0x01
	};

	UserCommand() : id(0), type(0), ctx(0) { };
	UserCommand(int aId, int aType, int aCtx, int aFlags, const string& aName, const string& aCommand, const string& aHub) throw() 
		: Flags(aFlags), id(aId), type(aType), ctx(aCtx), name(aName), command(aCommand), hub(aHub) { };
	
	UserCommand(const UserCommand& rhs) : Flags(rhs), id(rhs.id), type(rhs.type), 
		ctx(rhs.ctx), name(rhs.name), command(rhs.command), hub(rhs.hub) 
	{
		
	}

	UserCommand& operator=(const UserCommand& rhs) {
		id = rhs.id; type = rhs.type; ctx = rhs.ctx;
		name = rhs.name; command = rhs.command; hub = rhs.hub;
		*((Flags*)this) = rhs;
		return *this;
	}
	GETSET(int, id, Id);
	GETSET(int, type, Type);
	GETSET(int, ctx, Ctx);
	GETSET(string, name, Name);
	GETSET(string, command, Command);
	GETSET(string, hub, Hub);
};

#endif

/**
 * @file
 * $Id: UserCommand.h,v 1.7 2004/05/09 22:06:23 arnetheduck Exp $
 */

