/* 
* Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

/*
* Automatic Directory Listing Search
* Henrik Engström, henrikengstrom@home.se
*/

#include "stdinc.h"
#include "DCPlusPlus.h"
#include "File.h"
#include "SimpleXML.h"
#include "ADLSearch.h"

#define ADLS_STORE_FILENAME "ADLSearch.xml"

ADLSearchManager* Singleton<ADLSearchManager>::instance = NULL;

///////////////////////////////////////////////////////////////////////////////
//
//	Load old searches from disk
//
///////////////////////////////////////////////////////////////////////////////
void ADLSearchManager::Load()
{
	// Clear current
	collection.clear();

	// Load file as a string
	try 
	{
		SimpleXML xml;
		xml.fromXML(File(Util::getAppPath() + ADLS_STORE_FILENAME, File::READ, File::OPEN).read());
	
		if(xml.findChild("ADLSearch")) 
		{
			xml.stepIn();

			// Predicted several groups of searches to be differentiated
			// in multiple categories. Not implemented yet.
			if(xml.findChild("SearchGroup")) 
			{
				xml.stepIn();

				// Loop until no more searches found
				while(xml.findChild("Search"))	{
					xml.stepIn();

					// Found another search, load it
					ADLSearch search;

					if(xml.findChild("SearchString"))
					{
						search.searchString = xml.getChildData();
					}
					if(xml.findChild("SourceType"))
					{
						search.sourceType = search.StringToSourceType(xml.getChildData());
					}
					if(xml.findChild("DestDirectory"))
					{
						search.destDir = xml.getChildData();
					}
					if(xml.findChild("IsActive"))
					{
						search.isActive = (Util::toInt(xml.getChildData()) != 0);
					}
					if(xml.findChild("MaxSize"))
					{
						search.maxFileSize = Util::toInt64(xml.getChildData());
					}
					if(xml.findChild("MinSize"))
					{
						search.minFileSize = Util::toInt64(xml.getChildData());
					}

					// Add search to collection
					if(search.searchString.size() > 0)
					{
						search.Prepare();
						collection.push_back(search);
					}

					// Go to next search
					xml.stepOut();
				}
			}
		}
	} catch(SimpleXMLException) {
		return;
	} catch(FileException) {
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	Save current searches to disk
//
///////////////////////////////////////////////////////////////////////////////
void ADLSearchManager::Save()
{
	// Prepare xml string for saving
	try 
	{
		SimpleXML xml;

		xml.addTag("ADLSearch");
		xml.stepIn();

			// Predicted several groups of searches to be differentiated
			// in multiple categories. Not implemented yet.
			xml.addTag("SearchGroup");
			xml.stepIn();

				// Save all	searches
				for(SearchCollection::iterator i = collection.begin(); i != collection.end(); ++i)
				{
					ADLSearch& search = *i;
					if(search.searchString.size() == 0)
					{
						continue;
					}
					string type = "type";
					xml.addTag("Search");
					xml.stepIn();

					xml.addTag("SearchString", search.searchString);
					xml.addChildAttrib(type, string("string"));

					xml.addTag("SourceType", search.SourceTypeToString(search.sourceType));
					xml.addChildAttrib(type, string("string"));

					xml.addTag("DestDirectory", search.destDir);
					xml.addChildAttrib(type, string("string"));

					xml.addTag("IsActive", search.isActive);
					xml.addChildAttrib(type, string("int"));

					xml.addTag("MaxSize", search.maxFileSize);
					xml.addChildAttrib(type, string("int64"));

					xml.addTag("MinSize", search.minFileSize);
					xml.addChildAttrib(type, string("int64"));

					xml.stepOut();
				}

			xml.stepOut();

		xml.stepOut();

		// Save string to file			
		try 
		{
			File fout(Util::getAppPath() + ADLS_STORE_FILENAME, File::WRITE, File::CREATE | File::TRUNCATE);
			fout.write("<?xml version=\"1.0\" encoding=\"windows-1252\"?>\r\n");
			fout.write(xml.toXML());
			fout.close();
		} 
		catch(FileException e) 
		{
			return;
		}
	}
	catch(SimpleXMLException e) 
	{
		return;
	}
}

/**
 * @file ADLSearch.cpp
 * $Id: ADLSearch.cpp,v 1.2 2003/03/26 08:47:09 arnetheduck Exp $
 */

