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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "BitInputStream.h"
#include "BitOutputStream.h"

#include "CryptoManager.h"

#include "../bzip2/bzlib.h"

CryptoManager* Singleton<CryptoManager>::instance;

void CryptoManager::decodeBZ2(const u_int8_t* is, int sz, string& os) {
	bz_stream bs;

	memset(&bs, 0, sizeof(bs));

	if(BZ2_bzDecompressInit(&bs, 0, 0) != BZ_OK)
		return;

	// We assume that the files aren't compressed more than 4:1...
	int bufsize = 4*sz;
	char* buf = new char[bufsize];
	
	bs.avail_in = sz;
	bs.avail_out = bufsize;
	bs.next_in = (char*)(const_cast<u_int8_t*>(is));
	bs.next_out = buf;

	int err;

	while((err = BZ2_bzDecompress(&bs)) == BZ_OK) {
		os += string(buf, bufsize-bs.avail_out);
		bs.avail_out = bufsize;
		bs.next_out = buf;
	}

	if(err == BZ_STREAM_END)
		os += string(buf, bufsize-bs.avail_out);

	BZ2_bzDecompressEnd(&bs);
	
	delete buf;
}

void CryptoManager::encodeBZ2(const string& is, string& os) {
	bz_stream bs;
	
	memset(&bs, 0, sizeof(bs));

	if(BZ2_bzCompressInit(&bs, 9, 0, 30) != BZ_OK) {
		return;
	}

	// This size guarantees that the compressed data will fit (according to the bzip docs)
	int bufsize = (int)((double)is.size() * 1.01) + 600;
	
	char* buf = new char[bufsize];

	bs.next_in = const_cast<char*>(is.data());
	bs.avail_in = is.size();

	bs.next_out = buf;
	bs.avail_out = bufsize;

	int err = BZ2_bzCompress ( &bs, BZ_FINISH );
	dcassert(err != BZ_FINISH);
	if(err == BZ_STREAM_END) {
		os = string(buf, bufsize-bs.avail_out);
	}

	BZ2_bzCompressEnd(&bs);

	delete buf;
}

string CryptoManager::keySubst(const u_int8_t* aKey, int len, int n) {
	u_int8_t* temp = new u_int8_t[len + n * 10];
	
	int j=0;
	
	for(int i = 0; i<len; i++) {
		if(isExtra(aKey[i])) {
			temp[j++] = '/'; temp[j++] = '%'; temp[j++] = 'D';
			temp[j++] = 'C'; temp[j++] = 'N';
			switch(aKey[i]) {
			case 0: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '0'; break;
			case 5: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '5'; break;
			case 36: temp[j++] = '0'; temp[j++] = '3'; temp[j++] = '6'; break;
			case 96: temp[j++] = '0'; temp[j++] = '9'; temp[j++] = '6'; break;
			case 124: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '4'; break;
			case 126: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '6'; break;
			}
			temp[j++] = '%'; temp[j++] = '/';
		} else {
			temp[j++] = aKey[i];
		}
	}
	string tmp((char*)temp, j);
	delete[] temp;
	return tmp;
}

string CryptoManager::makeKey(const string& lock) {
	if(lock.size() < 3)
		return Util::emptyString;

	u_int8_t* temp = new u_int8_t[lock.length()];
	u_int8_t v1;
	int extra=0;
	
	v1 = (u_int8_t)(lock[0]^5);
	v1 = (u_int8_t)(((v1 >> 4) | (v1 << 4)) & 0xff);
	temp[0] = v1;
	
	string::size_type i;

	for(i = 1; i<lock.length(); i++) {
		v1 = (u_int8_t)(lock[i]^lock[i-1]);
		v1 = (u_int8_t)(((v1 >> 4) | (v1 << 4))&0xff);
		temp[i] = v1;
		if(isExtra(temp[i]))
			extra++;
	}
	
	temp[0] = (u_int8_t)(temp[0] ^ temp[lock.length()-1]);
	
	if(isExtra(temp[0])) {
		extra++;
	}
	
	string tmp = keySubst(temp, lock.length(), extra);
	delete[] temp;
	return tmp;
}

void CryptoManager::decodeHuffman(const u_int8_t* is, string& os) {
//	BitInputStream bis;
	int pos = 0;

	if(is[pos] != 'H' || is[pos+1] != 'E' || !((is[pos+2] == '3') || (is[pos+2] == '0'))) {
		return;
	}
	pos+=5;

	int size;
	size = *(int*)&is[pos];

	pos+=4;

	dcdebug("Size: %d\n", size);
	
	short treeSize;
	treeSize = *(short*)&is[pos];

	pos+=2;

	Leaf** leaves = new Leaf*[treeSize];

	int i;
	for(i=0; i<treeSize; i++) {
		int chr =  is[pos++];
		int bits = is[pos++];
		leaves[i] = new Leaf(chr, bits);
	}

	BitInputStream bis(is, pos);

	DecNode* root = new DecNode();

	for(i=0; i<treeSize; i++) {
		DecNode* node = root;
		for(int j=0; j<leaves[i]->len; j++) {
			if(bis.get()) {
				if(node->right == NULL)
					node->right = new DecNode();

				node = node->right;
			} else {
				if(node->left == NULL)
					node->left = new DecNode();

				node = node->left;
			}
		}
		node->chr = leaves[i]->chr;
	}
	
	bis.skipToByte();
	
	// We know the size, so no need to use strange STL stuff...
	char* buf = new char[size+1];
	pos = 0;
	for(i=0; i<size; i++) {
		DecNode* node = root;
		while(node->chr == -1) {
			if(bis.get()) {
				node = node->right;
			} else {
				node = node->left;
			}

			if(node == NULL) {
				dcdebug("Bad node found!!!\n");
				return;
			}
		}
		buf[pos++] = (u_int8_t)node->chr;
	}
	buf[pos] = 0;
	os.assign(buf, size);
	delete[] buf;

	for(i=0; i<treeSize; i++) {
		delete leaves[i];
	}
	
	delete[] leaves;
	delete root;
}

/**
 * Counts the occurances of each characters, and adds the total number of
 * different characters to the end of the array.
 */
int CryptoManager::countChars(const string& aString, int* c, u_int8_t& csum) {
	int chars = 0;
	const u_int8_t* a = (const u_int8_t*)aString.data();
	string::size_type len = aString.length();
	for(string::size_type i=0; i<len; i++) {

		if(c[a[i]] == 0)
			chars++;

		c[a[i]]++;
		csum^=a[i];
	}
	return chars;
}

void CryptoManager::walkTree(list<Node*>& aTree) {
	while(aTree.size() > 1) {
		// Merge the first two nodes
		Node* node = new Node(aTree.front(), *(++aTree.begin()));
		aTree.pop_front();
		aTree.pop_front();

		bool done = false;
		for(list<Node*>::iterator i=aTree.begin(); i != aTree.end(); ++i) {
			if(*node <= *(*i)) {
				aTree.insert(i, node);
				done = true;
				break;
			}
		}

		if(!done)
			aTree.push_back(node);

	}
}

/**
 * @todo Make more effective in terms of memory allocations and copies...
 */
void CryptoManager::recurseLookup(vector<u_int8_t>* table, Node* node, vector<u_int8_t>& u_int8_ts) {
	if(node->chr != -1) {
		table[node->chr] = u_int8_ts;
		return;
	}

	vector<u_int8_t> left = u_int8_ts;
	vector<u_int8_t> right = u_int8_ts;
	
	left.push_back(0);
	right.push_back(1);

	recurseLookup(table, node->left, left);
	recurseLookup(table, node->right, right);
}

/**
 * Builds a table over the characters available (for fast lookup).
 * Stores each character as a set of u_int8_ts with values {0, 1}.
 */
void CryptoManager::buildLookup(vector<u_int8_t>* table, Node* aRoot) {
	vector<u_int8_t> left;
	vector<u_int8_t> right;

	left.push_back(0);
	right.push_back(1);

	recurseLookup(table, aRoot->left, left);
	recurseLookup(table, aRoot->right, right);
}


class greaterNode { 
public:
	bool operator() (Node*& a, Node*& b) const { 
		return *a < *b; 
	}; 
};

/**
 * Encodes a set of data with DC's version of huffman encoding..
 * @todo Use real streams maybe? or something else than string (operator[] contains a compare, slow...)
 */
void CryptoManager::encodeHuffman(const string& is, string& os) {
	
	// We might as well expect this much data as huffman encoding doesn't go very far...
	os.reserve(is.size());
	if(is.length() == 0) {
		os.append("HE3\x0d");
		
		// Nada...
		os.append(7, 0);
		return;
	}
	// First, we count all characters
	u_int8_t csum = 0;
	int count[256];
	memset(count, 0, sizeof(count));
	int chars = countChars(is, count, csum);

	// Next, we create a set of nodes and add it to a list, removing all characters that never occur.
	
	list<Node*> nodes;

	int i;
	for(i=0; i<256; i++) {
		if(count[i] > 0) {
			nodes.push_back(new Node(i, count[i]));
		}
	}

	nodes.sort(greaterNode());
	dcdebug("\n");
#ifdef _DEBUG
	for(list<Node*>::iterator it = nodes.begin(); it != nodes.end(); ++it) dcdebug("%.02x:%d, ", (*it)->chr, (*it)->weight);
#endif
	
	walkTree(nodes);
	dcassert(nodes.size() == 1);

	Node* root = nodes.front();
	vector<u_int8_t> lookup[256];
	
	// Build a lookup table for fast character lookups
	buildLookup(lookup, root);
	delete root;

	// Reserve some memory to avoid all those copies when appending...
	os.reserve(is.size() * 3 / 4);

	os.append("HE3\x0d");
	
	// Checksum
	os.append(1, csum);
	string::size_type sz = is.size();
	os.append((char*)&sz, 4);

	// Character count
	os.append((char*)&chars, 2);

	// The characters and their bitlengths
	for(i=0; i<256; i++) {
		if(count[i] > 0) {
			os.append(1, (u_int8_t)i);
			os.append(1, (u_int8_t)lookup[i].size());
		}
	}
	
	BitOutputStream bos(os);
	// The tree itself, ie the bits of each character
	for(i=0; i<256; i++) {
		if(count[i] > 0) {
			bos.put(lookup[i]);
		}
	}
	
	dcdebug("\nu_int8_ts: %d", os.size());
	bos.skipToByte();

	for(string::size_type j=0; j<is.size(); j++) {
		dcassert(lookup[(u_int8_t)is[j]].size() != 0);
		bos.put(lookup[(u_int8_t)is[j]]);
	}
	bos.skipToByte();
}

/**
 * @file CryptoManager.cpp
 * $Id: CryptoManager.cpp,v 1.26 2002/06/27 23:38:24 arnetheduck Exp $
 */
