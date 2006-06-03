/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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
#include "ResourceManager.h"

#ifdef _WIN32
#include "../bzip2/bzlib.h"
#else
#include <bzlib.h>
#endif

void CryptoManager::decodeBZ2(const u_int8_t* is, size_t sz, string& os) throw (CryptoException) {
	bz_stream bs = { 0 };

	if(BZ2_bzDecompressInit(&bs, 0, 0) != BZ_OK)
		throw(CryptoException(STRING(DECOMPRESSION_ERROR)));

	// We assume that the files aren't compressed more than 2:1...if they are it'll work anyway,
	// but we'll have to do multiple passes...
	size_t bufsize = 2*sz;
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

string CryptoManager::keySubst(const u_int8_t* aKey, size_t len, size_t n) {
	AutoArray<u_int8_t> temp(len + n * 10);
	
	size_t j=0;
	
	for(size_t i = 0; i<len; i++) {
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
	return string((char*)(u_int8_t*)temp, j);
}

string CryptoManager::makeKey(const string& aLock) {
	if(aLock.size() < 3)
		return Util::emptyString;

    AutoArray<u_int8_t> temp(aLock.length());
	u_int8_t v1;
	size_t extra=0;
	
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
	
	return keySubst(temp, aLock.length(), extra);
}

void CryptoManager::decodeHuffman(const u_int8_t* is, string& os, const size_t len) throw(CryptoException) {
//	BitInputStream bis;
	int pos = 0;

	if(len < 11 || is[pos] != 'H' || is[pos+1] != 'E' || !((is[pos+2] == '3') || (is[pos+2] == '0'))) {
		throw CryptoException(STRING(DECOMPRESSION_ERROR));
	}
	pos+=5;

	int size;
	size = *(int*)&is[pos];

	pos+=4;

	dcdebug("Size: %d\n", size);
	
	unsigned short treeSize;
	treeSize = *(unsigned short*)&is[pos];

	pos+=2;

	if(len < (size_t)(11 + treeSize * 2)) 
		throw CryptoException(STRING(DECOMPRESSION_ERROR));
	Leaf** leaves = new Leaf*[treeSize];

	int i;
	for(i=0; i<treeSize; i++) {
		int chr =  is[pos++];
		int bits = is[pos++];
		leaves[i] = new Leaf(chr, bits);
	}

	BitInputStream bis(is, pos, len);

	DecNode* root = new DecNode();

	for(i=0; i<treeSize; i++) {
		DecNode* node = root;
		for(int j=0; j<leaves[i]->len; j++) {
			try {
				if(bis.get()) {
					if(node->right == NULL)
						node->right = new DecNode();

					node = node->right;
				} else {
					if(node->left == NULL)
						node->left = new DecNode();

					node = node->left;
				}
			} catch(const BitStreamException&) {
				throw CryptoException(STRING(DECOMPRESSION_ERROR));
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
			try {
				if(bis.get()) {
					node = node->right;
				} else {
					node = node->left;
				}
			} catch(const BitStreamException&) {
				throw CryptoException(STRING(DECOMPRESSION_ERROR));
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
