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

#ifndef _MERKLE_TREE
#define _MERKLE_TREE

#pragma once

#include "File.h"

template<class Hasher>
class MerkleTree {
public:
	MerkleTree() : fileSize(-1), blockSize(1024) {
	}

	~MerkleTree() {
	}

	void hashFile(const string& fileName) throw(FileException) {
		File f(fileName, File::READ, File::OPEN);
		fileSize = f.getSize();

		u_int8_t zero = 0;

		// We only calculate leaves for now
		AutoArray<u_int8_t> buf(blockSize);
		size_t n;
		do {
			n = f.read(buf, blockSize);
			Hasher h;
			h.update(&zero, 1);
			h.update(buf, n);
			leaves.push_back(HashValue(h.finalize()));
		} while(n > 0);

		calcRoot();
	}

	u_int8_t* getRoot() { return root.data; }

	bool verifyRoot(const u_int8_t* aRoot) {
		return memcmp(aRoot, getRoot(), Hasher::HASH_SIZE) == 0;
	}

private:
	struct HashValue {
		HashValue() { };
		HashValue(u_int8_t* aData) { memcpy(data, aData, Hasher::HASH_SIZE); }
		HashValue(const HashValue& rhs) { memcpy(data, rhs.data, Hasher::HASH_SIZE); }
		HashValue& operator=(const HashValue& rhs) { memcpy(data, rhs.data, Hasher::HASH_SIZE); return *this; }
		u_int8_t data[Hasher::HASH_SIZE];
	};

	typedef vector<HashValue> HashList;
	typedef typename HashList::iterator HashIter;
	
	HashList leaves;

	HashValue root;
	int64_t fileSize;
	size_t blockSize;
	
	void calcRoot() {
		root = getHash(0, fileSize);
	}

	HashValue getHash(int64_t start, int64_t length) {
		dcassert((start % blockSize) == 0);
		if(length <= blockSize) {
			dcassert((start / blockSize) < leaves.size());
			return leaves[(u_int32_t)(start / blockSize)];
		} else {
			u_int8_t one = 1;
			Hasher h;
			h.update(&one, 1);
			int64_t l2 = blockSize;
			while(l2 * 2 < length)
				l2 *= 2;

			HashValue l = getHash(start, l2);
			HashValue r = getHash(start + l2, length - l2);
			h.update(l.data, Hasher::HASH_SIZE);
			h.update(r.data, Hasher::HASH_SIZE);
			return HashValue(h.finalize());
		}
	}
};

#endif // _MERKLE_TREE

/**
 * @file
 * $Id: MerkleTree.h,v 1.1 2004/01/25 10:37:40 arnetheduck Exp $
 */
