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

#include "stdafx.h"
#include "Resource.h"

// Only need this for older ATL:s
#if _ATL_VER < 0x0700
#include <atlimpl.cpp>
#endif

// Basic sanity check
#if (_WTL_VER < 0x700) || (defined(HAS_STLPORT) && (_STLPORT_VERSION < 0x455))
#error Read compile.txt and do as it says before asking stupid questions at the forums
#endif

/**
 * @file
 * $Id: stdafx.cpp,v 1.8 2003/11/04 20:18:16 arnetheduck Exp $
 */

