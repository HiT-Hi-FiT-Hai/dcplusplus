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

class CryptoManager  
{
public:
	string makeKey(const string& aLock);
	string getLock() { return lock; };
	string getPk() { return pk; };

	void decodeHuffman(const string& is, string& os);

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

	CryptoManager() : lock("EXTENDEDPROTOCOLABCABCABCABCABCABCABCABC"), pk("DCPLUSPLUS0.01ABCABCABC") { };

	string lock;
	string pk;
	
	string keySubst(string aKey, int n);
	boolean isExtra(BYTE b) {
		return (b == 0 || b==5 || b==124 || b==96 || b==126 || b==36);
	}
	
	class Leaf {
	public:
		int len;
		int chr;
		Leaf(int aChr, int aLen) : chr(aChr), len(aLen) { };
		Leaf() : chr(-1), len(-1) { };
	};
	
	class Node {
	public:
		int weight;

		int chr;
		Node* left;
		Node* right;
		
		Node(int aChr, int aWeight) : chr(aChr), weight(aWeight), left(NULL), right(NULL) { };
		Node(Node* aLeft, Node* aRight) : left(aLeft), right(aRight), weight(aLeft->weight + aRight->weight) { };
		~Node() {
			delete left;
			delete right;
		}

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
	
	
};

#endif // !defined(AFX_CRYPTO_H__28F66860_0AD5_44AD_989C_BA4326C42F46__INCLUDED_)

/**
 * @file CryptoManager.h
 * $Id: CryptoManager.h,v 1.2 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: CryptoManager.h,v $
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
