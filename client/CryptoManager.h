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
	static string lock;
	static string pk;

	static string keySubst(string aKey, int n);
	static boolean isExtra(BYTE b) {
		return (b == 0 || b==5 || b==124 || b==96 || b==126 || b==36);
	}
	
public:
	static string makeKey(const string& aLock);
	static string getLock() { return lock; };
	static string getPk() { return pk; };
};

#endif // !defined(AFX_CRYPTO_H__28F66860_0AD5_44AD_989C_BA4326C42F46__INCLUDED_)

/**
 * @file CryptoManager.h
 * $Id: CryptoManager.h,v 1.1 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: CryptoManager.h,v $
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */
