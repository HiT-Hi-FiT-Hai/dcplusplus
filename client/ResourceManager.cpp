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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ResourceManager.h"
#include "SimpleXML.h"
#include "File.h"

ResourceManager* ResourceManager::instance;

void ResourceManager::loadLanguage(const string& aFile) {
	try {
		File f(aFile, File::READ, File::OPEN);
		SimpleXML xml;
		xml.fromXML(f.read());

		HASH_MAP<string, int> h;
		
		for(int i = 0; i < LAST; ++i) {
			h[names[i]] = i;
		}

		if(xml.findChild("Language")) {
			xml.stepIn();
			if(xml.findChild("Strings")) {
				xml.stepIn();

				while(xml.findChild("String")) {
					HASH_MAP<string, int>::iterator j = h.find(xml.getChildAttrib("Name"));

					if(j != h.end()) {
						strings[j->second] = xml.getChildData();
					}
				}
			}
		}
	} catch(Exception e) {
		// ...
	}
}
/**
 * @file ResourceManager.h
 * $Id: ResourceManager.cpp,v 1.3 2002/04/09 18:43:28 arnetheduck Exp $
 * @if LOG
 * $Log: ResourceManager.cpp,v $
 * Revision 1.3  2002/04/09 18:43:28  arnetheduck
 * Major code reorganization, to ease maintenance and future port...
 *
 * Revision 1.2  2002/03/13 20:35:26  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.1  2002/03/10 22:41:43  arnetheduck
 * First go at the new resource managment...
 *
 * @endif
 */

