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

#if !defined(AFX_CRYPTO_H__28F66860_0AD5_44AD_989C_BA4326C42F46__INCLUDED_)
#define AFX_CRYPTO_H__28F66860_0AD5_44AD_989C_BA4326C42F46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"

class Node {
public:
	// What's this? The only way (I've found out) to avoid a Internal Compiler Error! If this class is moved into
	// CryptoManager along with the greater specialization, it generates a ICE on the greater class. The typedefs
	// had to be removed in order to avoid template instatiation.
//	typedef Node* Ptr;
//	typedef list<Ptr> List;
//	typedef List::iterator Iter;
	int chr;
	int weight;
	
	Node* left;
	Node* right;
	
	Node(int aChr, int aWeight) : chr(aChr), weight(aWeight), left(NULL), right(NULL) { };
	Node(Node* aLeft, Node* aRight) :  chr(-1), weight(aLeft->weight + aRight->weight), left(aLeft), right(aRight) { };
	~Node() {
		delete left;
		delete right;
	}
	bool operator <(const Node& rhs) const {
		return weight<rhs.weight;
	}
	bool operator >(const Node& rhs) const {
		return weight>rhs.weight;
	}
	bool operator <=(const Node& rhs) const {
		return weight<=rhs.weight;
	}
	bool operator >=(const Node& rhs) const {
		return weight>rhs.weight;
	}
};

class CryptoManager : public Singleton<CryptoManager>
{
public:
	string makeKey(const string& aLock);
	const string& getLock() { return lock; };
	const string& getPk() { return pk; };
	bool isExtended(const string& aLock) { return aLock.find("EXTENDEDPROTOCOL") != string::npos; };

	void decodeHuffman(const u_int8_t* is, string& os);
	void encodeHuffman(const string& is, string& os);
	
private:

	friend class Singleton<CryptoManager>;
	
	CryptoManager() : lock("EXTENDEDPROTOCOLABCABCABCABCABCABCABCABC"), pk("DCPLUSPLUS" VERSIONSTRING "ABCABCABC") { };
	virtual ~CryptoManager() { };

	class Leaf {
	public:
		int chr;
		int len;
		Leaf(int aChr, int aLen) : chr(aChr), len(aLen) { };
		Leaf() : chr(-1), len(-1) { };
	};
	
	class DecNode {
	public:
		int chr;
		DecNode* left;
		DecNode* right;
		DecNode(int aChr) : chr(aChr), left(NULL), right(NULL) { };
		DecNode(DecNode* aLeft, DecNode* aRight) : chr(-1), left(aLeft), right(aRight) { };
		DecNode() : chr(-1), left(NULL), right(NULL) { };
		~DecNode() {
			delete left;
			delete right;
		}
	};
	

	const string lock;
	const string pk;

	int countChars(const string& aString, int* c, u_int8_t& csum);
	void walkTree(list<Node*>& aTree);
	void recurseLookup(vector<u_int8_t>* b, Node* node, vector<u_int8_t>& bytes);
	void buildLookup(vector<u_int8_t>* b, Node* root);
	
	string keySubst(string aKey, int n);
	bool isExtra(u_int8_t b) {
		return (b == 0 || b==5 || b==124 || b==96 || b==126 || b==36);
	}
	
	
};

#endif // !defined(AFX_CRYPTO_H__28F66860_0AD5_44AD_989C_BA4326C42F46__INCLUDED_)

/**
 * @file CryptoManager.h
 * $Id: CryptoManager.h,v 1.14 2002/04/09 18:43:27 arnetheduck Exp $
 * @if LOG
 * $Log: CryptoManager.h,v $
 * Revision 1.14  2002/04/09 18:43:27  arnetheduck
 * Major code reorganization, to ease maintenance and future port...
 *
 * Revision 1.13  2002/04/03 23:20:35  arnetheduck
 * ...
 *
 * Revision 1.12  2002/03/19 00:41:37  arnetheduck
 * 0.162, hub counting and cpu bug
 *
 * Revision 1.11  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.10  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.9  2002/01/11 14:52:56  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.8  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.7  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.6  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.5  2001/12/07 20:03:06  arnetheduck
 * More work done towards application stability
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
