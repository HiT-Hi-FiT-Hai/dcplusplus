/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#if !defined(AFX_RESOURCEMANAGER_H__AA978E1D_82F9_434B_8C3C_1D58B93F7582__INCLUDED_)
#define AFX_RESOURCEMANAGER_H__AA978E1D_82F9_434B_8C3C_1D58B93F7582__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Singleton.h"

class ResourceManager : public Singleton<ResourceManager> {
public:
	
#include "StringDefs.h"

	void loadLanguage(const string& aFile);
	const string& getString(Strings x) const { dcassert(x >= 0 && x < LAST); return strings[x]; };
	const wstring& getStringW(Strings x) const { dcassert(x >= 0 && x < LAST); return wstrings[x]; };

private:
	friend class Singleton<ResourceManager>;
	
	typedef HASH_MAP<string, Strings> NameMap;
	typedef NameMap::iterator NameIter;

	ResourceManager() { 
		createWide();
	};

	virtual ~ResourceManager() { };
	
	static string strings[LAST];
	static wstring wstrings[LAST];
	static string names[LAST];

	void createWide();
};


#define STRING(x) ResourceManager::getInstance()->getString(ResourceManager::x)
#define CSTRING(x) ResourceManager::getInstance()->getString(ResourceManager::x).c_str()
#define WSTRING(x) ResourceManager::getInstance()->getStringW(ResourceManager::x)
#define CWSTRING(x) ResourceManager::getInstance()->getStringW(ResourceManager::x).c_str()

#define STRING_I(x) ResourceManager::getInstance()->getString(x)
#define CSTRING_I(x) ResourceManager::getInstance()->getString(x).c_str()
#define WSTRING_I(x) ResourceManager::getInstance()->getStringW(x)
#define CWSTRING_I(x) ResourceManager::getInstance()->getStringW(x).c_str()

#ifdef UNICODE
#define TSTRING WSTRING
#define CTSTRING CWSTRING
#define CTSTRING_I CWSTRING_I
#else
#define TSTRING STRING
#define CTSTRING CSTRING
#endif


#endif // !defined(AFX_RESOURCEMANAGER_H__AA978E1D_82F9_434B_8C3C_1D58B93F7582__INCLUDED_)

/**
 * @file
 * $Id: ResourceManager.h,v 1.13 2004/09/06 12:32:42 arnetheduck Exp $
 */
