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

#if !defined(AFX_RESOURCEMANAGER_H__AA978E1D_82F9_434B_8C3C_1D58B93F7582__INCLUDED_)
#define AFX_RESOURCEMANAGER_H__AA978E1D_82F9_434B_8C3C_1D58B93F7582__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ResourceManager : public Singleton<ResourceManager> {
public:
	
#include "StringDefs.h"

	void loadLanguage(const string& aFile);
	const string& getString(Strings x) const { return strings[x]; };

private:

	friend class Singleton<ResourceManager>;
	ResourceManager() { };
	virtual ~ResourceManager() { };
	
	static string strings[LAST];
	static string names[LAST];
};

#define STRING(x) ResourceManager::getInstance()->getString(ResourceManager::x)
#define CSTRING(x) ResourceManager::getInstance()->getString(ResourceManager::x).c_str()

#endif // !defined(AFX_RESOURCEMANAGER_H__AA978E1D_82F9_434B_8C3C_1D58B93F7582__INCLUDED_)

/**
 * @file ResourceManager.h
 * $Id: ResourceManager.h,v 1.3 2002/03/13 20:35:26 arnetheduck Exp $
 * @if LOG
 * $Log: ResourceManager.h,v $
 * Revision 1.3  2002/03/13 20:35:26  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.2  2002/03/11 22:58:54  arnetheduck
 * A step towards internationalization
 *
 * Revision 1.1  2002/03/10 22:41:43  arnetheduck
 * First go at the new resource managment...
 *
 * @endif
 */