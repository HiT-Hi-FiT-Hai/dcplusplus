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

#ifndef _MERKLE_TREE
#define _MERKLE_TREE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TigerHash.h"
#include "Encoder.h"
#include "HashValue.h"
#include "File.h"

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
	enum { BASE_BLOCK_SIZE = baseBlockSize };

	typedef HashValue<Hasher> MerkleValue;
	typedef vector<MerkleValue> MerkleList;
	typedef typename MerkleList::iterator MerkleIter;

	MerkleTree() : fileSize(0), timeStamp(0), blockSize(baseBlockSize) { }
	MerkleTree(int64_t aBlockSize, u_int32_t aTimeStamp = 0) : fileSize(0), timeStamp(aTimeStamp), blockSize(aBlockSize) {
	}

	/**
	 * Loads a set of leaf hashes, calculating the root
	 * @param data Pointer to (aFileSize + aBlockSize - 1) / aBlockSize) hash values,
	 *             stored consecutively left to right
	 */
	MerkleTree(int64_t aFileSize, u_int32_t aTimeStamp, int64_t aBlockSize, u_int8_t* aData) : 
		fileSize(aFileSize), timeStamp(aTimeStamp), blockSize(aBlockSize) 
	{
		size_t n = calcBlocks(aFileSize, aBlockSize);
		for(size_t i = 0; i < n; i++)
			leaves.push_back(MerkleValue(aData + i * Hasher::HASH_SIZE));

		calcRoot();
	}

	~MerkleTree() {
	}

	static int64_t calcBlockSize(int64_t aFileSize, int maxLevels) {
		int64_t tmp = baseBlockSize;
		int64_t maxHashes = ((int64_t)1) << (maxLevels - 1);
		while((maxHashes * tmp) < aFileSize)
			tmp *= 2;
		return tmp;
	}
	static size_t calcBlocks(int64_t aFileSize, int64_t aBlockSize) {
		return max((size_t)((aFileSize + aBlockSize - 1) / aBlockSize), (size_t)1);
	}

	/**
	 * Update the merkle tree.
	 * @param len Length of data, must be a multiple of baseBlockSize, unless it's
	 *            the last block.
	 */
	void update(const void* data, size_t len) {
		u_int8_t* buf = (u_int8_t*)data;
		u_int8_t zero = 0;
		size_t i = 0;

		// Skip empty data sets if we already added at least one of them...
		if(len == 0 && !(leaves.empty() && blocks.empty()))
			return;
		
		do {
			size_t n = min(baseBlockSize, len-i);
			Hasher h;
			h.update(&zero, 1);
			h.update(buf + i, n);
			if(baseBlockSize < blockSize) {
				blocks.push_back(make_pair(MerkleValue(h.finalize()), baseBlockSize));
				reduceBlocks();
			} else {
				leaves.push_back(MerkleValue(h.finalize()));
			}
			i += n;
		} while(i < len);
		fileSize += len;
	}

	u_int8_t* finalize() {
		while(blocks.size() > 1) {
			MerkleBlock& a = blocks[blocks.size()-2];
			MerkleBlock& b = blocks[blocks.size()-1];
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

	MerkleValue& getRoot() { return root; }
	const MerkleValue& getRoot() const { return root; }
	MerkleList& getLeaves() { return leaves; }
	const MerkleList& getLeaves() const { return leaves; }

	int64_t getBlockSize() const { return blockSize; }
	void setBlockSize(int64_t aSize) { blockSize = aSize; }

	int64_t getFileSize() const { return fileSize; }
	void setFileSize(int64_t aSize) { fileSize = aSize; }

	u_int32_t getTimeStamp() const { return timeStamp; }

	bool verifyRoot(const u_int8_t* aRoot) {
		return memcmp(aRoot, getRoot().data(), Hasher::HASH_SIZE) == 0;
	}

	void calcRoot() {
		root = getHash(0, fileSize);
	}

private:	
	typedef pair<MerkleValue, size_t> MerkleBlock;
	typedef vector<MerkleBlock> MBList;

	MBList blocks;

	MerkleList leaves;

	MerkleValue root;
	/** Total size of hashed data */
	int64_t fileSize;
	/** Last modification date of data */
	u_int32_t timeStamp;
	/** Final block size */
	int64_t blockSize;
	
	MerkleValue getHash(int64_t start, int64_t length) {
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

	MerkleValue combine(const MerkleValue& a, const MerkleValue& b) {
		u_int8_t one = 1;
		Hasher h;
		h.update(&one, 1);
		h.update(a.data, MerkleValue::SIZE);
		h.update(b.data, MerkleValue::SIZE);
		return MerkleValue(h.finalize());
	}

	void reduceBlocks() {
		while(blocks.size() > 1) {
			MerkleBlock& a = blocks[blocks.size()-2];
			MerkleBlock& b = blocks[blocks.size()-1];
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
typedef TigerTree::MerkleValue TTHValue;

template<class T>
class TreeInputStream : public InputStream {
public:
	
	TreeInputStream(const MerkleTree<T>& aTree) : leaves(aTree.getLeaves().size() * Value::SIZE, 0),  pos(0) {
		u_int8_t* p = &leaves[0];
		for(size_t i = 0; i < aTree.getLeaves().size(); ++i) {
			memcpy(p + i * Value::SIZE, &aTree.getLeaves()[i], Value::SIZE);
		}
	}

	virtual ~TreeInputStream() {
	}

	virtual size_t read(void* buf, size_t& len) throw(Exception) {
		len = min(len, leaves.size() - pos);
		memcpy(buf, &leaves[pos], len);
		pos += len;
		return len;
	}
private:
	typedef typename MerkleTree<T>::MerkleValue Value;
	vector<u_int8_t> leaves;
	size_t pos;
};
struct TTFilter {
	TTFilter(int64_t aBlockSize, u_int32_t aTimeStamp = 0) : tt(aBlockSize, aTimeStamp) { };
	void operator()(const void* data, size_t len) { tt.update(data, len); }
	TigerTree& getTree() { tt.finalize(); return tt; }
private:
	TigerTree tt;
};

#endif // _MERKLE_TREE

/**
 * @file
 * $Id: MerkleTree.h,v 1.18 2004/10/26 13:53:58 arnetheduck Exp $
 */
