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

#include "stdinc.h"

#if defined(HAS_STLPORT) && (_STLPORT_VERSION != 0x462)
#error STLPort not correctly installed, read compile.txt
#endif

#ifndef _STLP_NO_IOSTREAMS
#error You're not using the STLPort from the DC++ homepage, that uses a different configuration than the original one. Remove this line only if you know what you're doing.
#endif

/**
 * @file
 * $Id: stdinc.cpp,v 1.4 2004/04/18 12:51:14 arnetheduck Exp $
 */
