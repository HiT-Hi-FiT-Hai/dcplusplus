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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "BitInputStream.h"
#include "BitOutputStream.h"

#include "CryptoManager.h"

CryptoManager* CryptoManager::instance;

string CryptoManager::keySubst(string aKey, int n) {
	BYTE* temp = new BYTE[aKey.length() + n * 10];
	
	int j=0;
	
	for(int i = 0; i<aKey.length(); i++) {
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
	delete temp;
	return tmp;
}

string CryptoManager::makeKey(const string& lock) {
	BYTE* temp = new BYTE[lock.length()];
	BYTE v1;
	int extra=0;
	
	v1 = lock[0]^5;
	v1 = (v1 >> 4) | (v1 << 4);
	temp[0] = v1;
	
	for(int i = 1; i<lock.length(); i++) {
		v1 = lock[i]^lock[i-1];
		v1 = (v1 >> 4) | (v1 << 4);
		temp[i] = v1;
		if(isExtra(temp[i]))
			extra++;
	}
	
	temp[0] = (BYTE)(temp[0] ^ temp[lock.length()-1]);
	
	if(isExtra(temp[0])) {
		extra++;
	}
	
	string tmp((char*)temp, i);
	delete temp;
	return keySubst(tmp, extra);
}

void CryptoManager::decodeHuffman(const BYTE* is, string& os) {
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
	
	for(int i=0; i<treeSize; i++) {
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
		buf[pos++] = (BYTE)node->chr;
	}
	buf[pos] = 0;
	os.assign(buf, size);
	delete buf;

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
int CryptoManager::countChars(const string& aString, int* c, BYTE& csum) {
	int chars = 0;
	const BYTE* a = (const BYTE*)aString.data();
	string::size_type len = aString.length();
	for(int i=0; i<len; i++) {

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
void CryptoManager::recurseLookup(vector<BYTE>* table, Node* node, vector<BYTE>& bytes) {
	if(node->chr != -1) {
		table[node->chr] = bytes;
		return;
	}

	vector<BYTE> left = bytes;
	vector<BYTE> right = bytes;
	
	left.push_back(0);
	right.push_back(1);

	recurseLookup(table, node->left, left);
	recurseLookup(table, node->right, right);
}

/**
 * Builds a table over the characters available (for fast lookup).
 * Stores each character as a set of bytes with values {0, 1}.
 */
void CryptoManager::buildLookup(vector<BYTE>* table, Node* aRoot) {
	vector<BYTE> left;
	vector<BYTE> right;

	left.push_back(0);
	right.push_back(1);

	recurseLookup(table, aRoot->left, left);
	recurseLookup(table, aRoot->right, right);
}

/**
 * Encodes a set of data with DC's version of huffman encoding..
 * @todo Use real streams maybe? or something else than string (operator[] contains a compare, slow...)
 */

void CryptoManager::encodeHuffman(const string& is, string& os) {

	if(is.length() == 0) {
		os.append("HE3\x0d");
		
		// Nada...
		os.append(7, 0);
		return;
	}
	// First, we count all characters
	BYTE csum = 0;
	int count[256];
	memset(count, 0, sizeof(count));
	int chars = countChars(is, count, csum);

	// Next, we create a set of nodes and add it to a list, removing all characters that never occur.
	
	list<Node*> nodes;

	for(int i=0; i<256; i++) {
		if(count[i] > 0) {
			nodes.push_back(new Node(i, count[i]));
		}
	}

	nodes.sort(greater<Node*>());
	dcdebug("\n");
	for(list<Node*>::iterator it = nodes.begin(); it != nodes.end(); ++it) dcdebug("%.02x:%d, ", (*it)->chr, (*it)->weight);
	walkTree(nodes);
	dcassert(nodes.size() == 1);

	Node* root = nodes.front();
	vector<BYTE> lookup[256];
	
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
			os.append(1, (BYTE)i);
			os.append(1, (BYTE)lookup[i].size());
		}
	}
	
	BitOutputStream bos(os);
	// The tree itself, ie the bits of each character
	for(i=0; i<256; i++) {
		if(count[i] > 0) {
			bos.put(lookup[i]);
		}
	}
	
	dcdebug("\nBytes: %d", os.size());
	bos.skipToByte();

	for(i=0; i<is.size(); i++) {
		dcassert(lookup[(BYTE)is[i]].size() != 0);
		bos.put(lookup[(BYTE)is[i]]);
	}
	bos.skipToByte();
}

/**
 * @file CryptoManager.cpp
 * $Id: CryptoManager.cpp,v 1.15 2002/01/20 22:54:46 arnetheduck Exp $
 * @if LOG
 * $Log: CryptoManager.cpp,v $
 * Revision 1.15  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.14  2002/01/10 12:33:14  arnetheduck
 * Various fixes
 *
 * Revision 1.13  2002/01/09 19:01:35  arnetheduck
 * Made some small changed to the key generation and search frame...
 *
 * Revision 1.12  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.11  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.10  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.9  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.8  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.7  2001/12/07 20:03:05  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.6  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
 * Revision 1.5  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
 *
 * Revision 1.4  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.3  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */
