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
#include "DCPlusPlus.h"

#include "CryptoManager.h"

#include "BitInputStream.h"
#include "BitOutputStream.h"
#include "File.h"

#ifdef _WIN32
#include "../bzip2/bzlib.h"
#else
#include <bzlib.h>
#endif

CryptoManager* Singleton<CryptoManager>::instance;

const int8_t CryptoManager::base32Table[] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,26,27,28,29,30,31,-1,-1,-1,-1,-1,-1,-1,-1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

const char CryptoManager::base32Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

ZCompressor::ZCompressor(File& file, int64_t aMaxBytes /* = -1 */, int aStrength /* = Z_DEFAULT_COMPRESSION */) throw(CryptoException) : 
	inbuf(NULL), f(file), maxBytes(aMaxBytes), level(aStrength) {
	
	memset(&zs, 0, sizeof(zs));
	
	if(deflateInit(&zs, level) != Z_OK) {
		throw CryptoException(STRING(COMPRESSION_ERROR));
	}

	inbuf = new u_int8_t[INBUF_SIZE];
}

u_int32_t ZCompressor::compress(void* buf, u_int32_t bufLen, u_int32_t& bytesRead) throw(FileException, CryptoException) {
	dcassert(buf);

	if(bufLen == 0) {
		return 0;
	}

	zs.avail_out = bufLen;
	zs.next_out = (u_int8_t*) buf;
	bytesRead = 0;

	// Check if we're compressing at all...if not; set level to 0 to just compute
	// the adler32...we want at least 5% compression, a completely arbitrary value.
	// The 64kb probe zone is also taken out of the air...
	if( (maxBytes != 0) && (level != 0) && (zs.total_out > 64*1024) && (zs.total_out > ((u_int32_t)((float)zs.total_in*0.95))) ) {
		dcdebug("Disabling compression for 0x%p (%ld/%ld = %.02f)\n", this, zs.total_out, zs.total_in, ((float)zs.total_out / (float)zs.total_in));
		setStrength(0);
	}

	while(zs.avail_out > 0) {
		// Check if we need to read more
		if(zs.avail_in == 0 && maxBytes != 0) {
			// Read more data
			u_int32_t bytes = (maxBytes == -1) ? INBUF_SIZE : (u_int32_t) min((int64_t) INBUF_SIZE, maxBytes);
			zs.avail_in = f.read(inbuf, bytes);
			zs.next_in = (u_int8_t*)inbuf;
			bytesRead += zs.avail_in;

			if(maxBytes != -1) {
				if(zs.avail_in == 0)
					throw CryptoException(STRING(COMPRESSION_ERROR));
				maxBytes -= zs.avail_in;
			} else {
				if(zs.avail_in == 0)
					maxBytes = 0;
			}
		}

		int err = ::deflate(&zs, ((maxBytes == 0) ? Z_FINISH : Z_NO_FLUSH));
			
		if(err == Z_STREAM_END)
			return bufLen - zs.avail_out;
		if(err != Z_OK) {
			throw CryptoException(STRING(COMPRESSION_ERROR));
		}
	}

	return bufLen;
}

void ZCompressor::setStrength(int str) throw(CryptoException) {
	if(level != str) {
		u_int32_t x = zs.avail_in;
		zs.avail_in = 0;
		int err = ::deflateParams(&zs, str, Z_DEFAULT_STRATEGY);
		zs.avail_in = x;
		dcassert(err != Z_BUF_ERROR);

		if(err != Z_OK) {
			throw CryptoException(STRING(COMPRESSION_ERROR));
		}
		level = str;
	}
}

ZDecompressor::ZDecompressor() throw(CryptoException) : outbuf(NULL) {
	memset(&zs, 0, sizeof(zs));

	if(inflateInit(&zs) != Z_OK)
		throw(CryptoException(STRING(DECOMPRESSION_ERROR)));

	outbuf = new u_int8_t[OUTBUF_SIZE];
}

u_int32_t ZDecompressor::decompress(const void* inbuf, int& inbytes) throw(CryptoException) {
	zs.avail_in = inbytes;
	zs.avail_out = OUTBUF_SIZE;
	zs.next_in = (u_int8_t*)const_cast<void*>(inbuf);
	zs.next_out = (u_int8_t*)outbuf;

	int err = inflate(&zs, Z_NO_FLUSH);

	if(err == Z_OK || err == Z_STREAM_END) {
		inbytes = zs.avail_in;
		return OUTBUF_SIZE - zs.avail_out;
	} else {
		dcdebug("ZDecompressor::decompress Error %d while decompressing\n", err);
		throw CryptoException(STRING(DECOMPRESSION_ERROR));
	}
}

void CryptoManager::decodeBZ2(const u_int8_t* is, size_t sz, string& os) throw (CryptoException) {
	bz_stream bs;

	memset(&bs, 0, sizeof(bs));

	if(BZ2_bzDecompressInit(&bs, 0, 0) != BZ_OK)
		throw(CryptoException(STRING(DECOMPRESSION_ERROR)));

	// We assume that the files aren't compressed more than 4:1...if they are it'll work anyway,
	// but we'll have to do multiple passes...
	int bufsize = 4*sz;
	AutoArray<char> buf(bufsize);
	
	bs.avail_in = sz;
	bs.avail_out = bufsize;
	bs.next_in = (char*)(const_cast<u_int8_t*>(is));
	bs.next_out = buf;

	int err;

	os.clear();
	
	while((err = BZ2_bzDecompress(&bs)) == BZ_OK) { 
		if (bs.avail_in == 0 && bs.avail_out > 0) { // error: BZ_UNEXPECTED_EOF 
			BZ2_bzDecompressEnd(&bs); 
			throw CryptoException(STRING(DECOMPRESSION_ERROR)); 
		} 
		os.append(buf, bufsize-bs.avail_out); 
		bs.avail_out = bufsize; 
		bs.next_out = buf; 
	} 

	if(err == BZ_STREAM_END)
		os.append(buf, bufsize-bs.avail_out);
	
	BZ2_bzDecompressEnd(&bs);

	if(err < 0) {
		// This was a real error
		throw CryptoException(STRING(DECOMPRESSION_ERROR));	
	}
}

void CryptoManager::encodeBZ2(const string& is, string& os, int strength /* = 9 */) {
	bz_stream bs;
	
	memset(&bs, 0, sizeof(bs));

	if(BZ2_bzCompressInit(&bs, strength, 0, 30) != BZ_OK) {
		return;
	}

	// This size guarantees that the compressed data will fit (according to the bzip docs)
	int bufsize = (int)((double)is.size() * 1.01) + 600;
	
	AutoArray<char> buf(bufsize);

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

string CryptoManager::makeKey(const string& aLock) {
	if(aLock.size() < 3)
		return Util::emptyString;

    u_int8_t* temp = new u_int8_t[aLock.length()];
	u_int8_t v1;
	int extra=0;
	
	v1 = (u_int8_t)(aLock[0]^5);
	v1 = (u_int8_t)(((v1 >> 4) | (v1 << 4)) & 0xff);
	temp[0] = v1;
	
	string::size_type i;

	for(i = 1; i<aLock.length(); i++) {
		v1 = (u_int8_t)(aLock[i]^aLock[i-1]);
		v1 = (u_int8_t)(((v1 >> 4) | (v1 << 4))&0xff);
		temp[i] = v1;
		if(isExtra(temp[i]))
			extra++;
	}
	
	temp[0] = (u_int8_t)(temp[0] ^ temp[aLock.length()-1]);
	
	if(isExtra(temp[0])) {
		extra++;
	}
	
	string tmp = keySubst(temp, aLock.length(), extra);
	delete[] temp;
	return tmp;
}

void CryptoManager::decodeHuffman(const u_int8_t* is, string& os) throw(CryptoException) {
//	BitInputStream bis;
	int pos = 0;

	if(is[pos] != 'H' || is[pos+1] != 'E' || !((is[pos+2] == '3') || (is[pos+2] == '0'))) {
		throw CryptoException(STRING(DECOMPRESSION_ERROR));
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
	AutoArray<char> buf(size+1);

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
				for(i=0; i<treeSize; i++) {
					delete leaves[i];
				}
				
				delete[] leaves;
				delete root;

				dcdebug("Bad node found!!!\n");
				throw CryptoException(STRING(DECOMPRESSION_ERROR));
			}
		}
		buf[pos++] = (u_int8_t)node->chr;
	}
	buf[pos] = 0;
	os.assign(buf, size);

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
		os.append(7, '\0');
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

string CryptoManager::encodeBase32(const u_int8_t* src, size_t len) {
	// Code snagged from the bitzi bitcollider
	size_t i, index;
	u_int8_t word;
	string dst;
	dst.reserve(((len * 8) / 5) + 1);

	for(i = 0, index = 0; i < len;) {
		/* Is the current word going to span a byte boundary? */
		if (index > 3) {
			word = (src[i] & (0xFF >> index));
			index = (index + 5) % 8;
			word <<= index;
			if ((i + 1) < len)
				word |= src[i + 1] >> (8 - index);

			i++;
		} else {
			word = (src[i] >> (8 - (index + 5))) & 0x1F;
			index = (index + 5) % 8;
			if (index == 0)
				i++;
		}

		dcassert(word < 32);
		dst += base32Alphabet[word];
	}
	return dst;
}

void CryptoManager::decodeBase32(const char* src, u_int8_t* dst, size_t len) {
	size_t i, index, offset;

	memset(dst, 0, len);
	for(i = 0, index = 0, offset = 0; src[i]; i++) {
		// Skip what we don't recognise
		int8_t tmp = base32Table[src[i]];

		if(tmp == -1)
			continue;

		if (index <= 3) {
			index = (index + 5) % 8;
			if (index == 0) {
				dst[offset] |= tmp;
				offset++;
				if(offset == len)
					break;
			} else {
				dst[offset] |= tmp << (8 - index);
			}
		} else {
			index = (index + 5) % 8;
			dst[offset] |= (tmp >> index);
			offset++;
			if(offset == len)
				break;
			dst[offset] |= tmp << (8 - index);
		}
	}
}

/**
 * @file
 * $Id: CryptoManager.cpp,v 1.40 2004/01/25 15:29:07 arnetheduck Exp $
 */
