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

#include "ShareManager.h"
#include "CryptoManager.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "UploadManager.h"
#include "ClientManager.h"

ShareManager* Singleton<ShareManager>::instance = NULL;

string ShareManager::translateFileName(const string& aFile) throw(ShareException) {
	RLock l(cs);
	if(aFile == "MyList.DcLst") {
		return getListFile();
	} else if(aFile == "MyList.bz2") {
		return getBZListFile();
	} else {
		string::size_type i = aFile.find('\\');
		if(i == string::npos)
			throw ShareException("File Not Available");
		
		string aDir = aFile.substr(0, i);

		RLock l(cs);
		StringMapIter j = dirs.find(aDir);
		if(j == dirs.end()) {
			throw ShareException("File Not Available");
		}
		
		// Make sure they're not trying something funny with the path and the get command...
		if(aFile.find("..\\") != string::npos) {
			throw ShareException("File Not Available");
		}
		
		if(Util::findSubString(aFile, "DCPlusPlus.xml") != string::npos) {
			throw ShareException("File Not Available");
		}
		
		if(!checkFile(j->second, aFile.substr(i + 1))) {
			throw ShareException("File Not Available");
		}
		
		return j->second + aFile.substr(i);
	}
}

bool ShareManager::checkFile(const string& dir, const string& aFile) {

	Directory::MapIter mi = directories.find(dir);
	if(mi == directories.end())
		return false;
	Directory* d = mi->second;
	string aDir;

	string::size_type i;
	string::size_type j = 0;
	while( (i = aFile.find('\\', j)) != string::npos) {
		aDir = aFile.substr(j, i-j);
		j = i + 1;
		mi = d->directories.find(aDir);
		if(mi == d->directories.end())
			return false;
		d = mi->second;

		j = i + 1;
	}
	aDir = aFile.substr(j);
	if(d->files.find(aDir) == d->files.end())
		return false;

	return true;
}

void ShareManager::load(SimpleXML* aXml) {
	WLock l(cs);

	if(aXml->findChild("Share")) {
		aXml->stepIn();
		while(aXml->findChild("Directory")) {
			try {
				addDirectory(aXml->getChildData());
			} catch(...) {
				// ...
			}
		}
		aXml->stepOut();
	}
	dirty = true;
}

void ShareManager::save(SimpleXML* aXml) {
	RLock l(cs);
	
	aXml->addTag("Share");
	aXml->stepIn();
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		aXml->addTag("Directory", i->first);
	}
	aXml->stepOut();
}

void ShareManager::addDirectory(const string& aDirectory) throw(ShareException) {
	if(aDirectory.size() == 0) {
		throw ShareException(STRING(NO_DIRECTORY_SPECIFIED));
	}

	{
		WLock l(cs);
		
		string d;
		if(aDirectory[aDirectory.size() - 1] == '\\') {
			d = aDirectory.substr(0, aDirectory.size()-1);
		} else {
			d = aDirectory;
		}
		
		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			if(d.find(i->first + '\\') != string::npos) {
				throw ShareException(STRING(DIRECTORY_ALREADY_SHARED));
			} else if(i->first.find(d + '\\') != string::npos) {
				throw ShareException(STRING(REMOVE_ALL_SUBDIRECTORIES));
			}
		}

		string dir = Util::toLower(d.substr(d.rfind('\\') + 1));
		
		if(dirs.find(dir) != dirs.end()) {
			// We have a duplicate, rename it internally...
			char c = 'a';
			while(dirs.find(dir + c) != dirs.end()) {
				c++;
			}
			dir += c;
		}
		
		Directory* dp = buildTree(d, NULL);
		dp->setName(dir);
		directories[d] = dp;
		dirs[dir] = d;
		
		dirty = true;
	}
}

void ShareManager::removeDirectory(const string& aDirectory) {
	WLock l(cs);

	Directory::MapIter i = directories.find(aDirectory);
	if(i != directories.end()) {
		delete i->second;
		directories.erase(i);
	}

	for(StringMapIter j = dirs.begin(); j != dirs.end(); ++j) {
		if(Util::stricmp(j->second.c_str(), aDirectory.c_str()) == 0) {
			dirs.erase(j);
			break;
		}
	}
	dirty = true;
}

ShareManager::Directory* ShareManager::buildTree(const string& aName, Directory* aParent) {
	Directory* dir = new Directory(aName.substr(aName.rfind('\\') + 1), aParent);
#ifdef WIN32
	WIN32_FIND_DATA data;
	HANDLE hFind;
	
	hFind = FindFirstFile((aName + "\\*").c_str(), &data);
	
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			string name = data.cFileName;
			if(name == "." || name == "..")
				continue;
			if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if( !((!BOOLSETTING(SHARE_HIDDEN)) && (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) ) {
					dir->directories[name] = buildTree(aName + '\\' + name, dir);
				}
			} else {

				if( !((!BOOLSETTING(SHARE_HIDDEN)) && (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) ) {

					// Not a directory, assume it's a file...make sure we're not sharing the settings file...
					if(Util::stricmp(name.c_str(), "DCPlusPlus.xml") != 0) {
						dir->files[name] = (int64_t)data.nFileSizeLow | ((int64_t)data.nFileSizeHigh)<<32;
						dir->size+=(int64_t)data.nFileSizeLow | ((int64_t)data.nFileSizeHigh)<<32;
					}
				}
			}
		} while(FindNextFile(hFind, &data));
	}
	
	FindClose(hFind);
#endif
	return dir;
}

StringList ShareManager::getDirectories() {
	RLock l(cs);

	StringList tmp;
	tmp.reserve(directories.size());
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		tmp.push_back(i->first);
	}
	return tmp;
}

void ShareManager::refresh(bool dirs /* = false */, bool aUpdate /* = true */, bool block /* = false */) throw(ShareException) {
	update = aUpdate;
	refreshDirs = dirs;
	if(dirty) {
		join();
		start();
		if(block) {
			join();
		} else {
			setThreadPriority(Thread::LOW);
		}
	}
}

int ShareManager::run() {

	string tmp, tmp2;
	{
		WLock l(cs);
		
		if(refreshDirs) {
			StringList dirs = getDirectories();
			for(StringIter k = dirs.begin(); k != dirs.end(); ++k) {
				removeDirectory(*k);
			}
			for(StringIter l = dirs.begin(); l != dirs.end(); ++l) {
				addDirectory(*l);
			}
			refreshDirs = false;
		}

		Directory::DupeMap dupes;
		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			tmp += i->second->toString(dupes);
		}
		CryptoManager::getInstance()->encodeHuffman(tmp, tmp2);
		try {
			File f(getListFile(), File::WRITE, File::CREATE | File::TRUNCATE);
			f.write(tmp2);
		} catch(FileException e) {
		}
		
		listLen = tmp2.length();
		tmp2.clear();
		CryptoManager::getInstance()->encodeBZ2(tmp, tmp2);
		try {
			File f(getBZListFile(), File::WRITE, File::CREATE | File::TRUNCATE);
			f.write(tmp2);
		} catch(FileException e) {
		}
		
		dirty = false;
	}

	if(update) {
		ClientManager::getInstance()->infoUpdated();
	}
	return 0;
}

string ShareManager::Directory::toString(DupeMap& dupes, int ident /* = 0 */) {
	string tmp(ident, '\t');
	tmp += name + "\r\n";

	for(MapIter i = directories.begin(); i != directories.end(); ++i) {
		tmp += i->second->toString(dupes, ident + 1);
	}
	
	Directory::FileIter j = files.begin();
	while(j != files.end()) {
		pair<DupeIter, DupeIter> p = dupes.equal_range(j->second);
		DupeIter k = p.first;
		for(; k != p.second; ++k) {
			if(k->second == j->first) {
				dcdebug("SM::D::toString Dupe found: %s (%I64d bytes)\n", k->second.c_str(), j->second);
				break;
			}
		}

		if(k != p.second) {
			size-=j->second;
			if(BOOLSETTING(REMOVE_DUPES)) {
				files.erase(j++);
			} else {
				++j;
			}
		} else {
			dupes.insert(make_pair(j->second, j->first));
			tmp += string(ident + 1, '\t') + j->first + "|" + Util::toString(j->second) + "\r\n";
			++j;
		}
	}

	return tmp;
}

#define IS_TYPE(x) (Util::findSubString(aString, x) != string::npos)

static const string types[] = { 
	".mp3", ".mp2", ".mid", ".wav", ".au", ".aiff",				// 0-6
	".zip", ".ace", ".rar",										// 6-9
	".htm", ".doc",												// 9-11
	".exe",														// 11-12
	".eps", ".ai", ".ps", ".img", ".pct", ".pict", ".psp", ".pic", ".png", ".tif", ".rle", ".bmp", ".pcx",	// 12-25
	".mpg", ".mov", ".mpeg", ".asf", ".avi", ".rm", ".pxp"		// 25-32
};

bool checkType(const string& aString, int aType) {
	bool found = false;
	switch(aType) {
	case SearchManager::TYPE_ANY: found = true; break;
	case SearchManager::TYPE_AUDIO:
		{
			for(int i = 0; i < 6; i++) {
				if(IS_TYPE(types[i])) {
					found = true;
					break;
				}
			}
		}
		break;
	case SearchManager::TYPE_COMPRESSED:
		if( IS_TYPE(types[6]) || IS_TYPE(types[7]) || IS_TYPE(types[8]) ) {
			found = true;
		}
		break;
	case SearchManager::TYPE_DOCUMENT:
		if( IS_TYPE(types[9]) || IS_TYPE(types[10]) ) {
			found = true;
		}
		break;
	case SearchManager::TYPE_EXECUTABLE:
		if(IS_TYPE(types[11] ) ) {
			found = true;
		}
		break;
	case SearchManager::TYPE_FOLDER:
		break;
	case SearchManager::TYPE_PICTURE:
		{
			for(int i = 12; i < 25; i++) {
				if(IS_TYPE(types[i])) {
					found = true;
					break;
				}
			}
		}
		break;
	case SearchManager::TYPE_VIDEO:
		{
			for(int i = 25; i < 32; i++) {
				if(IS_TYPE(types[i])) {
					found = true;
					break;
				}
			}
		}
		break;
	}
	return found;		
}

/**
 * Alright, the main point here is that when searching, a search string is most often found in 
 * the filename, not directory name, so we want to make that case faster. Also, we want to
 * avoid changing StringLists unless we absolutely have to --> this should only be done if a string
 * has been matched in the directory name. This new stringlist should also be used in all descendants,
 * but not the parents...
 */
void ShareManager::Directory::search(SearchResult::List& aResults, StringList& aStrings, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) {
	StringList* cur = &aStrings;
	StringList newStr;

	// Find any matches in the directory name
	for(StringIter k = aStrings.begin(); k != aStrings.end(); ++k) {
		if(Util::findSubString(name, *k) != string::npos) {
			newStr.push_back(*k);
		}
	}

	if(newStr.size() > 0) {
		// Alright, we want to put the strings not found in newStr, and remove those already found
		// to avoid using another stringlist...
		for(StringIter m = aStrings.begin(); m != aStrings.end(); ++m) {
			StringIter n = find(newStr.begin(), newStr.end(), *m);
			if(n == newStr.end()) {
				newStr.push_back(*m);
			} else {
				newStr.erase(n);
			}
		}
		cur = &newStr;
	}

	bool found = true;
	for(FileIter i = files.begin(); i != files.end(); ++i) {

		if(aSearchType == SearchManager::SIZE_ATLEAST && i->second <= aSize) {
			continue;
		} else if(aSearchType == SearchManager::SIZE_ATMOST && i->second >= aSize) {
			continue;
		}	
		
		found = true;
		for(StringIter j = cur->begin(); (j != cur->end()); ++j) {
			if(Util::findSubString(i->first, *j) == string::npos) {
				found = false;
				break;
			}
		}
		
		if(!found)
			continue;

		// Check file type...
		if(checkType(i->first, aFileType)) {

			SearchResult* sr = new SearchResult();
			sr->setFile(getFullName() + i->first);
			sr->setSize(i->second);
			sr->setFreeSlots(UploadManager::getInstance()->getFreeSlots());
			sr->setSlots(SETTING(SLOTS));
			sr->setUser(ClientManager::getInstance()->getUser(aClient->getNick(), aClient, false));
			sr->setHubAddress(aClient->getIp());
			sr->setHubName(aClient->getName());
			aResults.push_back(sr);
			ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
			if(aResults.size() >= maxResults) {
				break;
			}
		}
	}

	for(Directory::MapIter l = directories.begin(); (l != directories.end()) && (aResults.size() < maxResults); ++l) {
		l->second->search(aResults, *cur, aSearchType, aSize, aFileType, aClient, maxResults);
	}
}

SearchResult::List ShareManager::search(const string& aString, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) {
	
	RLock l(cs);
	StringTokenizer t(aString, '$');
	StringList& sl = t.getTokens();
	SearchResult::List results;

	for(Directory::MapIter i = directories.begin(); (i != directories.end()) && (results.size() < maxResults); ++i) {
		i->second->search(results, sl, aSearchType, aSize, aFileType, aClient, maxResults);
	}
	
	return results;
}

// SettingsManagerListener
void ShareManager::onAction(SettingsManagerListener::Types type, SimpleXML* xml) {
	switch(type) {
	case SettingsManagerListener::LOAD: load(xml); break;
	case SettingsManagerListener::SAVE: save(xml); break;
	}
}

void ShareManager::onAction(TimerManagerListener::Types type, u_int32_t tick) {
	switch(type) {
	case TimerManagerListener::MINUTE:
		if(lastUpdate + 60 * 60 * 1000 < tick) {
			try {
				refresh(true, true);
				lastUpdate = tick;
			} catch(...) {
			}
		}
	}
}

/**
 * @file ShareManager.cpp
 * $Id: ShareManager.cpp,v 1.43 2002/06/03 20:45:38 arnetheduck Exp $
 */

