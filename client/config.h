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


// Change these values to use different versions...don't know what happens though...=)
#define WINVER		0x0400
#define _WIN32_IE	0x0500
#define _RICHEDIT_VER	0x0200


// This enables stlport's debug mode
# ifdef _DEBUG
# define _STLP_DEBUG 1
# else
# undef _STLP_DEBUG
# endif

// Remove this line if hashes are not available in your stl
// Hint: the once that comes with mcvc++ doesn't have hashes...
#define HAS_HASH 1