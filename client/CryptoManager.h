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

#if !defined(AFX_CRYPTO_H__28F66860_0AD5_44AD_989C_BA4326C42F46__INCLUDED_)
#define AFX_CRYPTO_H__28F66860_0AD5_44AD_989C_BA4326C42F46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Node {
public:
	// What's this? The only way (I've found out) to avoid a Internal Compiler Error! If this class is moved into
	// CryptoManager along with the greater specialization, it generates a ICE on the greater class. The typedefs
	// had to be removed in order to avoid template instatiation.
//	typedef Node* Ptr;
//	typedef list<Ptr> List;
//	typedef List::iterator Iter;
	int weight;
	
	int chr;
	Node* left;
	Node* right;
	
	Node(int aChr, int aWeight) : chr(aChr), weight(aWeight), left(NULL), right(NULL) { };
	Node(Node* aLeft, Node* aRight) : left(aLeft), right(aRight), weight(aLeft->weight + aRight->weight), chr(-1) { };
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

struct std::greater<Node*> { 
	bool operator() (const Node*& a, const Node*& b) const { 
		return *a < *b; 
	}; 
};


class CryptoManager  
{
public:
	string makeKey(const string& aLock);
	string getLock() { return lock; };
	string getPk() { return pk; };

	void decodeHuffman(const BYTE* is, string& os);
	void encodeHuffman(const string& is, string& os);
	
	static CryptoManager* getInstance() {
		dcassert(instance);
		return instance;
	}
	static void newInstance() {
		if(instance)
			delete instance;
		
		instance = new CryptoManager();
	}
	static void deleteInstance() {
		delete instance;
		instance = NULL;
	}

private:
	static CryptoManager* instance;

	class Leaf {
	public:
		int len;
		int chr;
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
	
	CryptoManager() : lock("EXTENDEDPROTOCOLABCABCABCABCABCABCABCABC"), pk("DCPLUSPLUS" VERSIONSTRING "ABCABCABC") { };

	string lock;
	string pk;
	
	int countChars(const string& aString, int* c, BYTE& csum);
	void walkTree(list<Node*>& aTree);
	void recurseLookup(vector<BYTE>* b, Node* node, vector<BYTE>& bytes);
	void buildLookup(vector<BYTE>* b, Node* root);
	
	string keySubst(string aKey, int n);
	boolean isExtra(BYTE b) {
		return (b == 0 || b==5 || b==124 || b==96 || b==126 || b==36);
	}
	
	
};

#endif // !defined(AFX_CRYPTO_H__28F66860_0AD5_44AD_989C_BA4326C42F46__INCLUDED_)

/**
 * @file CryptoManager.h
 * $Id: CryptoManager.h,v 1.8 2002/01/07 20:17:59 arnetheduck Exp $
 * @if LOG
 * $Log: CryptoManager.h,v $
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
