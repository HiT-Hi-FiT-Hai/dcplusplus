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

#include <math.h>

template<class Hasher>
class MerkleTree {
public:
	struct HashValue {
		enum { SIZE = Hasher::HASH_SIZE };

		HashValue() { };
		HashValue(u_int8_t* aData) { memcpy(data, aData, SIZE); }
		HashValue(const HashValue& rhs) { memcpy(data, rhs.data, SIZE); }
		HashValue& operator=(const HashValue& rhs) { memcpy(data, rhs.data, SIZE); return *this; }
		u_int8_t data[SIZE];
	};

	typedef vector<HashValue> HashList;
	typedef typename HashList::iterator HashIter;

	MerkleTree() : fileSize(-1), blockSize(0) {
	}

	~MerkleTree() {
	}

	void hashFile(const string& fileName, int maxLevels, size_t aBlockSize = 1024) throw(FileException) {

		File f(fileName, File::READ, File::OPEN);
		fileSize = f.getSize();
		blockSize = aBlockSize;
		
		int64_t maxHashes = (int64_t)pow(2, maxLevels - 1);
		while((maxHashes * (int64_t)blockSize) < fileSize)
			blockSize *= 2;

		u_int8_t zero = 0;

		// We only calculate leaves for now
		typedef pair<HashValue, size_t> HashBlock;
		typedef vector<HashBlock> HBList;

		HBList blocks;

		AutoArray<u_int8_t> buf(aBlockSize);
		size_t n;
		int64_t left = fileSize; 
		do {
			n = f.read(buf, aBlockSize);
			left -= n;
			Hasher h;
			h.update(&zero, 1);
			h.update(buf, n);
			if(aBlockSize < blockSize) {
				blocks.push_back(make_pair(HashValue(h.finalize()), aBlockSize));

				while(blocks.size() > 1) {
					HashBlock& a = blocks[blocks.size()-2];
					HashBlock& b = blocks[blocks.size()-1];
					if(a.second == b.second) {
						if(a.second*2 == blockSize) {
							leaves.push_back(combine(a.first, b.first));
							blocks.pop_back();
							blocks.pop_back();
						} else {
							a.second *= 2;
							a.first = combine(a.first, b.first);
							blocks.pop_back();
						}
					} else {
						break;
					}
				}
			} else {
				leaves.push_back(HashValue(h.finalize()));
			}
		} while(left > 0);

		while(blocks.size() > 1) {
			HashBlock& a = blocks[blocks.size()-2];
			HashBlock& b = blocks[blocks.size()-1];
			a.first = combine(a.first, b.first);
			blocks.pop_back();
		}

		dcassert(blocks.size() == 0 || blocks.size() == 1);
		if(!blocks.empty()) {
			leaves.push_back(blocks[0].first);
		}
		calcRoot();
	}

	/**
	 * Loads a set of leaf hashes, calculating the root
	 * @param aFileSize Total data size
	 * @param aBlockSize Block size used when creating the hash
	 * @param data Pointer to (aFileSize + aBlockSize - 1) / aBlockSize) hash values,
	 *             stored consecutively left to right
     */
	void load(int64_t aFileSize, size_t aBlockSize, u_int8_t* data) {
		blockSize = aBlockSize;
		fileSize = aFileSize;
		size_t n = (size_t)((fileSize + blockSize - 1) / blockSize);
		for(int i = 0; i < n; i++)
			leaves.push_back(HashValue(data + i * Hasher::HASH_SIZE));

		calcRoot();
	}

	HashValue& getRoot() { return root; }
	HashList& getLeaves() { return leaves; }

	size_t getBlockSize() { return blockSize; }

	bool verifyRoot(const u_int8_t* aRoot) {
		return memcmp(aRoot, getRoot().data(), Hasher::HASH_SIZE) == 0;
	}

private:	
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
			int64_t l = blockSize;
			while(l * 2 < length)
				l *= 2;
			return combine(getHash(start, l), getHash(start+l, length - l));
		}
	}

	HashValue combine(const HashValue& a, const HashValue& b) {
		u_int8_t one = 1;
		Hasher h;
		h.update(&one, 1);
		h.update(a.data, Hasher::HASH_SIZE);
		h.update(b.data, Hasher::HASH_SIZE);
		return HashValue(h.finalize());
	}
};

#endif // _MERKLE_TREE

/**
 * @file
 * $Id: MerkleTree.h,v 1.2 2004/01/25 15:29:07 arnetheduck Exp $
 */
