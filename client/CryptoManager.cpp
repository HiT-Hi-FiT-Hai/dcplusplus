/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#include "CryptoManager.h"
#include <streambuf>
#include <iterator>

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
	int v1, v2, v3, v4, v5, v6;
	int extra=0;
	
	v1 = lock[0];
	v2 = v1^5;
	v3 = v2 / 0x10;
	v4 = v2 * 0x10;
	v5 = v4 % 0x100;
	v6 = v3 | v5;
	
	temp[0] = (BYTE)v6;
	
	for(int i = 1; i<lock.length(); i++) {
		v1 = lock[i];
		v2 = v1^lock[i-1];
		v3 = v2 / 0x10;
		v4 = v2 * 0x10;
		v5 = v4 % 0x100;
		v6 = v3 | v5;
		temp[i] = (BYTE)v6;
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

void CryptoManager::decodeHuffman(const string& is, string& os) {
//	BitInputStream bis;
	int pos = 0;
	
	if(is[pos++] != 'H' || is[pos++] != 'E' || is[pos++] != '3') {
		return;
	}
	pos++;
	pos++;

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

	os.reserve(size+1);
	for(i=0; i<size; i++) {
		DecNode* node = root;
		while(node->chr == -1) {
			if(bis.get()) {
				node = node->right;
			} else {
				node = node->left;
			}

			if(node == NULL) {
				return;
			}
		}
		os+= (char)node->chr;
	}
	os[i] = 0;

	delete[] leaves;
	delete root;
}

	/**
	 * Counts the occurances of each characters, and adds the total number of
	 * different characters to the end of the array.
	 */
/*	private static int[] countChars(byte[] aData) {
		int count[] = new int[256 + 1];

		for(int i=0; i<aData.length-1; i++) {
			int j = (aData[i]<0) ? 256-aData[i] : aData[i];

			if(count[j] == 0)
				count[count.length-1]++;

			count[j]++;
		}
		return count;
	}
*/
/*	private static void walkTree(LinkedList aTree) {
		while(aTree.size() > 1) {
			// Merge the first two nodes
			Node node = new Node((Node)aTree.removeFirst(), (Node)aTree.removeFirst());

			boolean done = false;
			for(ListIterator i=aTree.listIterator(); i.hasNext(); ) {
				Node n = (Node)i.next();
				if(node.weight <= n.weight) {
					i.add(node);
					done = true;
					break;
				}
			}

			if(!done)
				aTree.addLast(node);
		}
	}
*/
	/**
	 * @todo Make more effective in terms of memory allocations and copies...
	 */
/*	private static void recurseHash(byte[][] table, Node node, byte[] bytes) {
		if(node.chr == -1) {
			table[node.chr] = bytes;
			return;
		}

		byte[] left = new byte[bytes.length+1];
		byte[] right = new byte[bytes.length+1];

		for(int i=0; i<bytes.length; i++) {
			left[i] = right[i] = bytes[i];
		}

		left[bytes.length] = 0;
		right[bytes.length] = 1;

		recurseHash(table, node.left, left);
		recurseHash(table, node.right, right);
	}
*/
	/**
	 * Builds a hash table over the characters available (for fast lookup).
	 * Stores each character as a set of bytes with values {0, 1}.
	 */
/*	private static byte[][] buildLookup(Node aRoot) {
		byte right[] = new byte[0];
		byte left[] = new byte[1];
		left[0] = 0;
		right[0] = 1;
		byte table[][] = new byte[256][];
		recurseHash(table, aRoot.left, left);
		recurseHash(table, aRoot.right, right);

		return table;
	}
*/
	/**
	 * Encodes a set of data with DC's version of huffman encoding..
	 * Uses a byte[] in structure to avoid those strange iostreams where available() doesn't return the whole data set,
	 * as we'll be passing multiple times over the data...
	 */
/*	public static void encode(byte[] aData, OutputStream os) throws IOException {

		// First, we count all characters
		int count[] = countChars(aData);

		// Next, we create a set of nodes and add it to a list, removing all characters that never occur.

		Node[] nodeArray = new Node[count[count.length-1]];

		for(int i=0, j=0; i<count.length-1; i++) {
			if(count[i] > 0) {
				nodeArray[j++] = new Node(i, count[i]);
			}
		}

		// Now sort the data
		Arrays.sort(nodeArray, new Comparator() {
			public int compare(Object o1, Object o2) {
				Node n1 = (Node) o1; Node n2 = (Node) o2;
				if(n1.weight < n2.weight) return -1;
				else if(n1.weight == n2.weight) return 0;
				else return 1;
			}
		});

		LinkedList nodes = new LinkedList(Arrays.asList(nodeArray));

		walkTree(nodes);

		Node root = (Node) nodes.removeFirst();

		// Build a hash table for fast character lookups
		byte chars[][] = buildLookup(root);

		// Alright, start writing...
		os.write('H'); os.write('E'); os.write('3'); os.write(0x0d);

		// This should be a checksum...
		os.write(0);



	}
*/

/**
 * @file CryptoManager.cpp
 * $Id: CryptoManager.cpp,v 1.2 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: CryptoManager.cpp,v $
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
