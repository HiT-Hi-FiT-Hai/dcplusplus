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

#if !defined(AFX_FAVORITEUSER_H__64E4A69E_BB58_425D_830C_ADD1760E29A4__INCLUDED_)
#define AFX_FAVORITEUSER_H__64E4A69E_BB58_425D_830C_ADD1760E29A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FastAlloc.h"

class FavoriteUser : public Flags, public FastAlloc<FavoriteUser>
{
public:
	typedef FavoriteUser* Ptr;
	
	enum Flags {
		FLAG_GRANTSLOT = 1 << 0
	};

	GETSET(string, description, Description);
	GETSET(u_int32_t, lastSeen, LastSeen);

	FavoriteUser() : lastSeen(0) {}
	~FavoriteUser() {}
};

#endif // !defined(AFX_FAVORITEUSER_H__64E4A69E_BB58_425D_830C_ADD1760E29A4__INCLUDED_)

/**
 * @file
 * $Id: FavoriteUser.h,v 1.7 2004/09/06 12:32:42 arnetheduck Exp $
 */
