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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ShareManager.h"
#include "CryptoManager.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "UploadManager.h"
#include "ClientManager.h"
#include "File.h"

ShareManager* Singleton<ShareManager>::instance = NULL;

ShareManager::ShareManager() : hits(0), listLen(0), dirty(false), refreshDirs(false), 
	update(false), listN(0), lFile(NULL), bFile(NULL), lastUpdate(GET_TICK()) { 
	SettingsManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
	/* Common search words used to make search more efficient, should be more dynamic */
	words.push_back("avi");
	words.push_back("mp3");
	words.push_back("bin");
	words.push_back("zip");
	words.push_back("jpg");
	words.push_back("mpeg");
	words.push_back("mpg");
	words.push_back("rar");
	words.push_back("ace");
	words.push_back("bin");
	words.push_back("iso");
	words.push_back("dev");
	words.push_back("flt");
	words.push_back("ccd");
	words.push_back("txt");
	words.push_back("sub");
	words.push_back("nfo");
	words.push_back("wav");
	words.push_back("exe");
	words.push_back("ccd");

};

ShareManager::~ShareManager() {
	SettingsManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);

	join();

	delete lFile;
	delete bFile;

	for(int i = 0; i <= listN; ++i) {
		File::deleteFile(Util::getAppPath() + "MyList" + Util::toString(i) + ".DcLst");
		File::deleteFile(Util::getAppPath() + "MyList" + Util::toString(i) + ".bz2");
	}

	for(Directory::MapIter j = directories.begin(); j != directories.end(); ++j) {
		delete j->second;
	}
}

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
			} catch(const ShareException&) {
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
	dir->addSearchType(getMask(dir->getName()));

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
					dir->addType(SearchManager::TYPE_DIRECTORY);
					dir->directories[name] = buildTree(aName + '\\' + name, dir);
					dir->addSearchType(dir->directories[name]->getSearchTypes()); 
				}
			} else {

				if( !((!BOOLSETTING(SHARE_HIDDEN)) && (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) ) {

					// Not a directory, assume it's a file...make sure we're not sharing the settings file...
					if( (Util::stricmp(name.c_str(), "DCPlusPlus.xml") != 0) && 
						(Util::stricmp(name.c_str(), "Favorites.xml") != 0) &&
						(name.find('$') == string::npos) ) {

						dir->addSearchType(getMask(name));
						dir->addType(getType(name));
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
			i->second->toString(tmp, dupes);
		}
		CryptoManager::getInstance()->encodeHuffman(tmp, tmp2);
		
		listN++;

		try {
			if(lFile != NULL) {
				delete lFile;
				lFile = NULL;
				// Try to delete it...
				File::deleteFile(getListFile());
			}

			setListFile(Util::getAppPath() + "MyList" + Util::toString(listN) + ".DcLst");
			
			lFile = new File(getListFile(), File::WRITE, File::CREATE | File::TRUNCATE);
			lFile->write(tmp2);
			delete lFile;
			// Null in case of exception...
			lFile = NULL;

			lFile = new File(getListFile(), File::READ, File::OPEN);
		} catch(const FileException&) {
		}

		listLen = tmp2.length();
		tmp2.clear();
		CryptoManager::getInstance()->encodeBZ2(tmp, tmp2);
		try {
			if(bFile != NULL) {
				delete bFile;
				bFile = NULL;
				File::deleteFile(getBZListFile());
			}

			setBZListFile(Util::getAppPath() + "MyList" + Util::toString(listN) + ".bz2");

			bFile = new File(getBZListFile(), File::WRITE, File::CREATE | File::TRUNCATE);
			bFile->write(tmp2);
			delete bFile;
			bFile = NULL;
			
			bFile = new File(getBZListFile(), File::READ, File::OPEN);
		} catch(const FileException&) {
		}
		
		dirty = false;
		lastUpdate = GET_TICK();
	}

	if(update) {
		ClientManager::getInstance()->infoUpdated();
	}
	return 0;
}

#define STRINGLEN(n) n, sizeof(n)-1
void ShareManager::Directory::toString(string& tmp, DupeMap& dupes, int ident /* = 0 */) {
	tmp.append(ident, '\t');
	tmp.append(name);
	tmp.append(STRINGLEN("\r\n"));

	for(MapIter i = directories.begin(); i != directories.end(); ++i) {
		i->second->toString(tmp, dupes, ident + 1);
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
				tmp.append(ident+1, '\t');
				tmp.append(j->first);
				tmp.append(STRINGLEN("|"));
				tmp.append(Util::toString(j->second));
				tmp.append(STRINGLEN("\r\n"));
				++j;
			}
		} else {
			dupes.insert(make_pair(j->second, j->first));
			tmp.append(ident+1, '\t');
			tmp.append(j->first);
			tmp.append(STRINGLEN("|"));
			tmp.append(Util::toString(j->second));
			tmp.append(STRINGLEN("\r\n"));
			++j;
		}
	}
}


// These ones we can look up as ints (4 bytes...)...

static const char* typeAudio[] = { ".mp3", ".mp2", ".mid", ".wav", ".ogg", ".wma" };
static const char* typeCompressed[] = { ".zip", ".ace", ".rar" };
static const char* typeDocument[] = { ".htm", ".doc", ".txt", ".nfo" };
static const char* typeExecutable[] = { ".exe" };
static const char* typePicture[] = { ".jpg", ".gif", ".png", ".eps", ".img", ".pct", ".psp", ".pic", ".tif", ".rle", ".bmp", ".pcx" };
static const char* typeVideo[] = { ".mpg", ".mov", ".asf", ".avi", ".pxp" };

static const string type2Audio[] = { ".au", ".aiff" };
static const string type2Picture[] = { ".ai", ".ps", ".pict" };
static const string type2Video[] = { ".rm", ".divx", ".mpeg" };

#define IS_TYPE(x) ( type == (*((u_int32_t*)x)) )
#define IS_TYPE2(x) (Util::stricmp(aString.c_str() + aString.length() - x.length(), x.c_str()) == 0)

bool checkType(const string& aString, int aType) {
	if(aType == SearchManager::TYPE_ANY)
		return true;

	if(aString.length() < 5)
		return false;
	
	const char* c = aString.c_str() + aString.length() - 3;
	u_int32_t type = '.' | (Util::toLower(c[0]) << 8) | (Util::toLower(c[1]) << 16) | (((u_int32_t)Util::toLower(c[2])) << 24);

	switch(aType) {
	case SearchManager::TYPE_AUDIO:
		{
			for(int i = 0; i < (sizeof(typeAudio) / sizeof(typeAudio[0])); i++) {
				if(IS_TYPE(typeAudio[i])) {
					return true;
				}
			}
			if( IS_TYPE2(type2Audio[0]) || IS_TYPE2(type2Audio[1]) ) {
				return true;
			}
		}
		break;
	case SearchManager::TYPE_COMPRESSED:
		if( IS_TYPE(typeCompressed[0]) || IS_TYPE(typeCompressed[1]) || IS_TYPE(typeCompressed[2]) ) {
			return true;
		}
		break;
	case SearchManager::TYPE_DOCUMENT:
		if( IS_TYPE(typeDocument[0]) || IS_TYPE(typeDocument[1]) || 
			IS_TYPE(typeDocument[2]) || IS_TYPE(typeDocument[3]) ) {
			return true;
		}
		break;
	case SearchManager::TYPE_EXECUTABLE:
		if(IS_TYPE(typeExecutable[0]) ) {
			return true;
		}
		break;
	case SearchManager::TYPE_PICTURE:
		{
			for(int i = 0; i < (sizeof(typePicture) / sizeof(typePicture[0])); i++) {
				if(IS_TYPE(typePicture[i])) {
					return true;
				}
			}
            if( IS_TYPE2(type2Picture[0]) || IS_TYPE2(type2Picture[1]) || IS_TYPE2(type2Picture[2]) ) {
				return true;
			}
		}
		break;
	case SearchManager::TYPE_VIDEO:
		{
			for(int i = 0; i < (sizeof(typeVideo) / sizeof(typeVideo[0])); i++) {
				if(IS_TYPE(typeVideo[i])) {
					return true;
				}
			}
            if( IS_TYPE2(type2Video[0]) || IS_TYPE2(type2Video[1]) || IS_TYPE2(type2Video[2]) ) {
				return true;
			}
		}
		break;
	default:
		dcasserta(0);
		break;
	}
	return false;
}

SearchManager::TypeModes ShareManager::getType(const string& aFileName) {
	if(aFileName[aFileName.length() - 1] == '\\') {
		return SearchManager::TYPE_DIRECTORY;
	}

	if(checkType(aFileName, SearchManager::TYPE_VIDEO))
		return SearchManager::TYPE_VIDEO;
	else if(checkType(aFileName, SearchManager::TYPE_AUDIO))
		return SearchManager::TYPE_AUDIO;
	else if(checkType(aFileName, SearchManager::TYPE_COMPRESSED))
		return SearchManager::TYPE_COMPRESSED;
	else if(checkType(aFileName, SearchManager::TYPE_DOCUMENT))
		return SearchManager::TYPE_DOCUMENT;
	else if(checkType(aFileName, SearchManager::TYPE_EXECUTABLE))
		return SearchManager::TYPE_EXECUTABLE;
	else if(checkType(aFileName, SearchManager::TYPE_PICTURE))
		return SearchManager::TYPE_PICTURE;

	return SearchManager::TYPE_ANY;
}

/**
 * The mask is a set of bits that say which words a file matches. Each bit means
 * that a fileName matches the word at position n-1 in the words list where n is
 * the bit number. bit 0 is only set when no words match.
 */
u_int32_t ShareManager::getMask(const string& fileName) {
	u_int32_t mask = 0;
	int n = 1;
	for(StringIter i = words.begin(); i != words.end(); ++i, n++) {
		if(Util::findSubString(fileName, *i) != string::npos) {
			mask |= (1 << n);
		}
	}
	return (mask == 0) ? 1 : mask;
}

u_int32_t ShareManager::getMask(StringList& l) {
	u_int32_t mask = 0;
	int n = 1;

	for(StringIter i = words.begin(); i != words.end(); ++i, n++) {
		for(StringIter j = l.begin(); j != l.end(); ++j) {
			if(Util::findSubString(*j, *i) != string::npos) {
				mask |= (1 << n);
			}
		}
	}
	return (mask == 0) ? 1 : mask;	
}

/**
 * Alright, the main point here is that when searching, a search string is most often found in 
 * the filename, not directory name, so we want to make that case faster. Also, we want to
 * avoid changing StringLists unless we absolutely have to --> this should only be done if a string
 * has been matched in the directory name. This new stringlist should also be used in all descendants,
 * but not the parents...
 */
void ShareManager::Directory::search(SearchResult::List& aResults, StringList& aStrings, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults, u_int32_t mask) {
	// Skip everything if there's nothing to find here (doh! =)
	if(!hasType(aFileType))
		return;

	if(!hasSearchType(mask))
		return;

	StringList* cur = &aStrings;
	StringList newStr;

	// Find any matches in the directory name
	for(StringIter k = aStrings.begin(); k != aStrings.end(); ++k) {
		if(Util::findSubString(name, *k) != string::npos) {
			newStr.push_back(*k);
			u_int32_t xmask = ShareManager::getInstance()->getMask(*k);
			if(xmask != 1) {
				mask &= ~xmask;
			}
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

	if(cur->empty() && ((aFileType == SearchManager::TYPE_ANY) || (aFileType == SearchManager::TYPE_DIRECTORY))) {
		// We satisfied all the search words! Add the directory...
		SearchResult* sr = new SearchResult();
		sr->setType(SearchResult::TYPE_DIRECTORY);
		sr->setFile(getFullName());
		sr->setFreeSlots(UploadManager::getInstance()->getFreeSlots());
		sr->setSlots(SETTING(SLOTS));
		sr->setUser(ClientManager::getInstance()->getUser(aClient->getNick(), aClient, false));
		sr->setHubAddress(aClient->getIp());
		sr->setHubName(aClient->getName());
		aResults.push_back(sr);
		ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
	}

	bool found = true;
	if(aFileType != SearchManager::TYPE_DIRECTORY) {
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
				sr->setType(SearchResult::TYPE_FILE);
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
	}

	for(Directory::MapIter l = directories.begin(); (l != directories.end()) && (aResults.size() < maxResults); ++l) {
		l->second->search(aResults, *cur, aSearchType, aSize, aFileType, aClient, maxResults, mask);
	}
}

SearchResult::List ShareManager::search(const string& aString, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) {
	
	RLock l(cs);
	StringTokenizer t(aString, '$');
	StringList& sl = t.getTokens();
	u_int32_t mask = getMask(sl);
	SearchResult::List results;

	for(Directory::MapIter i = directories.begin(); (i != directories.end()) && (results.size() < maxResults); ++i) {
		i->second->search(results, sl, aSearchType, aSize, aFileType, aClient, maxResults, mask);
	}
	
	return results;
}

// SettingsManagerListener
void ShareManager::onAction(SettingsManagerListener::Types type, SimpleXML* xml) throw() {
	switch(type) {
	case SettingsManagerListener::LOAD: load(xml); break;
	case SettingsManagerListener::SAVE: save(xml); break;
	}
}

void ShareManager::onAction(TimerManagerListener::Types type, u_int32_t tick) throw() {
	if(type == TimerManagerListener::MINUTE && BOOLSETTING(AUTO_UPDATE_LIST)) {
		if(lastUpdate + 60 * 60 * 1000 < tick) {
			try {
				dirty = true;
				refresh(true, true);
				lastUpdate = tick;
			} catch(const ShareException&) {
			}
		}
	}
}

/**
 * @file
 * $Id: ShareManager.cpp,v 1.48 2003/04/15 10:13:54 arnetheduck Exp $
 */

