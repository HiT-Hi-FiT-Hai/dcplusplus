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

#ifndef _HASH_MANAGER
#define _HASH_MANAGER

#pragma once

#include "Singleton.h"
#include "MerkleTree.h"
#include "TigerHash.h"

class HashManager : public Singleton<HashManager> {
public:
	HashManager(void);
	virtual ~HashManager(void);

private:
	typedef MerkleTree<TigerHash> TigerTree;
	typedef TigerTree::HashValue TTH;


};

#endif // _HASH_MANAGER

/**
 * @file
 * $Id: HashManager.h,v 1.2 2004/01/25 16:13:29 arnetheduck Exp $
 */
