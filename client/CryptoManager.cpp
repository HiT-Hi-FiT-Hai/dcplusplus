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

#include "CryptoManager.h"

string CryptoManager::lock = "EXTENDEDPROTOCOLABCABCABCABCABCABCABCABC";
string CryptoManager::pk = "DCPLUSPLUS0.01ABCABCABC";

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

/**
 * @file CryptoManager.cpp
 * $Id: CryptoManager.cpp,v 1.1 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: CryptoManager.cpp,v $
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */
