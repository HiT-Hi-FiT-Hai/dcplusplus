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

#include "TigerHash.h"
#include "Encoder.h"

#include <math.h>

/**
 * A class that represents a Merkle Tree hash. Storing
 * only the leaves of the tree, it is rather memory efficient, 
 * but can still take a significant amount of memory during / after 
 * hash generation. 
 * The root hash produced can be used like any
 * other hash to verify the integrity of a whole file, while
 * the leaves provide checking of smaller parts of the file.
 */
template<class Hasher, size_t baseBlockSize = 1024>
class MerkleTree {
public:
	enum { HASH_SIZE = Hasher::HASH_SIZE };
	struct HashValue {
		enum { SIZE = Hasher::HASH_SIZE };

		HashValue() { };
		HashValue(u_int8_t* aData) { memcpy(data, aData, SIZE); }
		HashValue(const string& base32) { Encoder::fromBase32(base32.c_str(), data, SIZE); };
		HashValue(const HashValue& rhs) { memcpy(data, rhs.data, SIZE); }
		HashValue& operator=(const HashValue& rhs) { memcpy(data, rhs.data, SIZE); return *this; }
		bool operator==(const HashValue& rhs) { return memcmp(data, rhs.data, SIZE) == 0; }

		string toBase32() { return Encoder::toBase32(data, SIZE); };

		u_int8_t data[SIZE];
	};

	typedef vector<HashValue> HashList;
	typedef typename HashList::iterator HashIter;

	MerkleTree(int64_t aFileSize, size_t aBlockSize) : fileSize(aFileSize), blockSize(aBlockSize) {
		leaves.reserve((size_t)(aFileSize / blockSize) + 1);
	}

	~MerkleTree() {
	}

	static size_t calcBlockSize(int64_t aFileSize, int maxLevels) {
		size_t tmp = baseBlockSize;
		int64_t maxHashes = (int64_t)pow(2, maxLevels - 1);
		while((maxHashes * tmp) < aFileSize)
			tmp *= 2;
		return tmp;
	}

	/**
	 * Update the merkle tree.
	 * @param len Length of data, must be a multiple of baseBlockSize, unless it's
	 *            the last block.
	 */
	void update(const void* data, u_int32_t len) {
		u_int8_t* buf = (u_int8_t*)data;
		u_int8_t zero = 0;
		size_t i = 0;

		// Skip empty data sets if we already added at least one of them...
		if(len == 0 && !(leaves.empty() && blocks.empty()))
			return;
		
		do {
			size_t n = min(baseBlockSize, (size_t)len-i);
			Hasher h;
			h.update(&zero, 1);
			h.update(buf + i, n);
			if(baseBlockSize < blockSize) {
				blocks.push_back(make_pair(HashValue(h.finalize()), baseBlockSize));
				reduceBlocks();
			} else {
				leaves.push_back(HashValue(h.finalize()));
			}
			i += n;
		} while(i < len);
	}

	u_int8_t* finalize() {
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
		return root.data;
	}

	/**
	 * Loads a set of leaf hashes, calculating the root
	 * @param data Pointer to (aFileSize + aBlockSize - 1) / aBlockSize) hash values,
	 *             stored consecutively left to right
     */
	void load(u_int8_t* data) {
		size_t n = (size_t)((fileSize + blockSize - 1) / blockSize);
		for(int i = 0; i < n; i++)
			leaves.push_back(HashValue(data + i * Hasher::HASH_SIZE));

		calcRoot();
	}

	HashValue& getRoot() { return root; }
	HashList& getLeaves() { return leaves; }

	size_t getBlockSize() { return blockSize; }
	int64_t getFileSize() { return fileSize; }

	bool verifyRoot(const u_int8_t* aRoot) {
		return memcmp(aRoot, getRoot().data(), Hasher::HASH_SIZE) == 0;
	}

private:	
	typedef pair<HashValue, size_t> HashBlock;
	typedef vector<HashBlock> HBList;

	HBList blocks;

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

	void reduceBlocks() {
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
	}
};

typedef MerkleTree<TigerHash> TigerTree;
typedef TigerTree::HashValue TTHValue;

#endif // _MERKLE_TREE

/**
 * @file
 * $Id: MerkleTree.h,v 1.4 2004/01/30 14:12:59 arnetheduck Exp $
 */
