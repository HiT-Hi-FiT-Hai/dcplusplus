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
#include "ClientManager.h"
#include "LogManager.h"
#include "HashManager.h"

#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "File.h"
#include "FilteredFile.h"
#include "BZUtils.h"

#ifndef _WIN32
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <limits>

ShareManager::ShareManager() : hits(0), listLen(0), bzXmlListLen(0),
	dirty(false), refreshDirs(false), update(false), listN(0), lFile(NULL), 
	xFile(NULL), lastUpdate(GET_TICK()), bloom(1<<20) 
{ 
	SettingsManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
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
	DownloadManager::getInstance()->removeListener(this);

	join();

	delete lFile;
	delete xFile;

	for(int i = 0; i <= listN; ++i) {
		File::deleteFile(Util::getAppPath() + "MyList" + Util::toString(i) + ".DcLst");
		File::deleteFile(Util::getAppPath() + "files" + Util::toString(i) + ".xml.bz2");
	}

	for(Directory::MapIter j = directories.begin(); j != directories.end(); ++j) {
		delete j->second;
	}
}

string ShareManager::translateFileName(const string& aFile, bool adc, bool utf8) throw(ShareException) {
	RLock l(cs);
	if(aFile == "MyList.DcLst") {
		return getListFile();
	} else if(aFile == "files.xml.bz2") {
		return getBZXmlFile();
	} else {
		string file;

		if(adc) {
			// Check for tth root identifier
			if(aFile.compare(0, 4, "TTH/") == 0) {
				TTHValue v(aFile.substr(4));
				HashFileIter i = tthIndex.find(&v);
				if(i != tthIndex.end()) {
					file = i->second->getADCPath();
				} else {
					throw ShareException("File Not Available");
				}
			} else if(aFile.compare(0, 1, "/") == 0) {
				if(utf8) {
					file = Util::toAcp(aFile, file);
				}
			} else {
				throw ShareException("File Not Available");
			}
			// Remove initial '/'
			file.erase(0, 1);

			// Change to NMDC path separators
			for(string::size_type i = 0; i < file.length(); ++i) {
				if(file[i] == '/') {
					file[i] = '\\';
				}
			}
			// Ok, we now should have an adc equivalent name
		} else if(utf8) {
			file = Util::toAcp(aFile, file);
		} else {
			file = aFile;
		}

		string::size_type i = file.find('\\');
		if(i == string::npos)
			throw ShareException("File Not Available");
		
		string aDir = file.substr(0, i);

		RLock l(cs);
		StringMapIter j = dirs.find(aDir);
		if(j == dirs.end()) {
			throw ShareException("File Not Available");
		}
		
		if(!checkFile(j->second, file.substr(i + 1))) {
			throw ShareException("File Not Available");
		}
		
		return j->second + file.substr(i);
	}
}

bool ShareManager::checkFile(const string& dir, const string& aFile) {

	Directory::MapIter mi = directories.find(dir);
	if(mi == directories.end())
		return false;
	Directory* d = mi->second;

	string::size_type i;
	string::size_type j = 0;
	while( (i = aFile.find(PATH_SEPARATOR, j)) != string::npos) {
		mi = d->directories.find(aFile.substr(j, i-j));
		j = i + 1;
		if(mi == d->directories.end())
			return false;
		d = mi->second;
	}
	
	if(find_if(d->files.begin(), d->files.end(), Directory::File::StringComp(aFile.substr(j))) == d->files.end())
		return false;

	return true;
}

void ShareManager::load(SimpleXML* aXml) {
	WLock l(cs);

	if(aXml->findChild("Share")) {
		aXml->stepIn();
		while(aXml->findChild("Directory")) {
			loadDirs.push_back(aXml->getChildData());
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
	if(Util::stricmp(SETTING(TEMP_DOWNLOAD_DIRECTORY), aDirectory) == 0) {
		throw ShareException(STRING(DONT_SHARE_TEMP_DIRECTORY));
	}
	{
		WLock l(cs);
		
		string d = ((aDirectory[aDirectory.size() - 1] == PATH_SEPARATOR) ? 
			aDirectory.substr(0, aDirectory.size()-1) : aDirectory);
		
		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			if(Util::stricmp(d, i->first) == 0) {
				// Trying to share an already shared directory
				throw ShareException(STRING(DIRECTORY_ALREADY_SHARED));
			} else if(Util::findSubString(d, i->first + PATH_SEPARATOR) != string::npos) {
				// Trying to share a subdirectory
				throw ShareException(STRING(DIRECTORY_ALREADY_SHARED));
			} else if(Util::findSubString(i->first, d + PATH_SEPARATOR) != string::npos) {
				// Trying to share a parent directory
				throw ShareException(STRING(REMOVE_ALL_SUBDIRECTORIES));
			}
		}

		string dir = Util::toLower(d.substr(d.rfind(PATH_SEPARATOR) + 1));
		
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

	string d;
	if(aDirectory[aDirectory.size() - 1] == PATH_SEPARATOR) {
		d = aDirectory.substr(0, aDirectory.size()-1);
	} else {
		d = aDirectory;
	}

	Directory::MapIter i = directories.find(d);
	if(i != directories.end()) {
		delete i->second;
		directories.erase(i);
	}

	for(StringMapIter j = dirs.begin(); j != dirs.end(); ++j) {
		if(Util::stricmp(j->second.c_str(), d.c_str()) == 0) {
			dirs.erase(j);
			break;
		}
	}
	dirty = true;
}

void ShareManager::addFinishedFile(Directory* aParent, const string& aName, int64_t aSize) {
	// add file and tth to share (file list will only be updated by the next refresh)
	string::size_type l = aName.find(PATH_SEPARATOR);
	
	if(l != string::npos) {
		Directory::MapIter i = aParent->directories.find(aName.substr(0, l-1));
		if(i != aParent->directories.end())
			addFinishedFile(i->second, aName.substr(l+1), aSize);
		else {
			Directory* dir = new Directory(aName.substr(0, l-1));
			dir->addType(SearchManager::TYPE_DIRECTORY);
			dir->addSearchType(getMask(dir->getName()));
			bloom.add(Util::toLower(dir->getName()));

			aParent->directories[dir->getName()] = dir;
			addFinishedFile(dir, aName.substr(l+1), aSize);
		}
	} else {
		aParent->addSearchType(getMask(aName));
		aParent->addType(getType(aName));

		Directory::File::Iter file = aParent->files.insert(aParent->files.begin(), 
			Directory::File(aName, aSize, aParent, NULL));

		aParent->size+=aSize;
					
		bloom.add(Util::toLower(aName));
	}
}

ShareManager::Directory* ShareManager::buildTree(const string& aName, Directory* aParent) {
	Directory* dir = new Directory(aName.substr(aName.rfind(PATH_SEPARATOR) + 1), aParent);
	dir->addType(SearchManager::TYPE_DIRECTORY); // needed since we match our own name in directory searches
	dir->addSearchType(getMask(dir->getName()));
	bloom.add(Util::toLower(dir->getName()));

	Directory::File::Iter lastFileIter = dir->files.begin();
#ifdef _WIN32
	WIN32_FIND_DATA data;
	HANDLE hFind;
	
	hFind = FindFirstFile((aName + "\\*").c_str(), &data);
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			string name = data.cFileName;
			if(name == "." || name == "..")
				continue;
			if(name.find('$') != string::npos) {
				LogManager::getInstance()->message(STRING(FORBIDDEN_DOLLAR_FILE) + name + " (" + STRING(SIZE) + ": " + Util::toString(File::getSize(name)) + " " + STRING(B) + ") (" + STRING(DIRECTORY) + ": \"" + aName + "\")");
				continue;
			}
			if(!BOOLSETTING(SHARE_HIDDEN) && ((data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || (name[0] == '.')) )
				continue;
			if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				string newName = aName + PATH_SEPARATOR + name;
				if(Util::stricmp(newName + PATH_SEPARATOR, SETTING(TEMP_DOWNLOAD_DIRECTORY)) != 0) {
					dir->directories[name] = buildTree(newName, dir);
					dir->addSearchType(dir->directories[name]->getSearchTypes()); 
				}
			} else {
				// Not a directory, assume it's a file...make sure we're not sharing the settings file...
				if( (Util::stricmp(name.c_str(), "DCPlusPlus.xml") != 0) && 
					(Util::stricmp(name.c_str(), "Favorites.xml") != 0)) {

					int64_t size = (int64_t)data.nFileSizeLow | ((int64_t)data.nFileSizeHigh)<<32;
					TTHValue* root = HashManager::getInstance()->getTTH(aName + PATH_SEPARATOR + name, size, File::convertTime(&data.ftLastWriteTime));

					if(root != NULL) {
						dir->addSearchType(getMask(name));
						dir->addType(getType(name));
						lastFileIter = dir->files.insert(lastFileIter, Directory::File(name, size, dir, root));

						if(root != NULL)
							tthIndex.insert(make_pair(root, lastFileIter));

						dir->size+=size;

						bloom.add(Util::toLower(name));
					}
				}
			}
		} while(FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	

#else // _WIN32
	DIR *dirp = opendir(aName.c_str());
	if (dirp) {
		while (dirent* entry = readdir(dirp)) {
			string name = entry->d_name;
			if (name == "." || name == "..") {
				continue;
			}
			if(name.find('$') != string::npos) {
				LogManager::getInstance()->message(STRING(FORBIDDEN_DOLLAR_DIRECTORY) + name + " (" + STRING(DIRECTORY) + ": \"" + aName + "\")");
				continue;
			}
			if (name[0] == '.' && !BOOLSETTING(SHARE_HIDDEN)) {
				continue;
			}
			string pathname = aName + PATH_SEPARATOR + name;
			struct stat s;
			if (stat(pathname.c_str(), &s) == 0) {
				if (S_ISDIR(s.st_mode)) {
					//dir->addType(SearchManager::TYPE_DIRECTORY);
					dir->directories[name] = buildTree(pathname, dir);
					dir->addSearchType(dir->directories[name]->getSearchTypes()); 

				} else if (S_ISREG(s.st_mode)) {
					dir->addSearchType(getMask(name));
					dir->addType(getType(name));
					int64_t size = s.st_size;
					TTHValue* root = HashManager::getInstance()->getTTH(aName + PATH_SEPARATOR + name, size, 0);
					lastFileIter = dir->files.insert(lastFileIter, Directory::File(name, size, dir, root));
					dir->size += s.st_size;
					bloom.add(Util::toLower(name));
				}
			}
		}
		closedir(dirp);
	}
#endif // _WIN32

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
	LogManager::getInstance()->message(STRING(FILE_LIST_REFRESH_INITIATED));
	{
		WLock l(cs);

		
		if(refreshDirs) {
			StringList dirs = getDirectories();
			for(StringIter k = dirs.begin(); k != dirs.end(); ++k) {
				removeDirectory(*k);
			}
			bloom.clear();
            for(StringIter l = dirs.begin(); l != dirs.end(); ++l) {
				try {
					addDirectory(*l);
				} catch(...) {
				}
			}
			for(StringIter m = loadDirs.begin(); m != loadDirs.end(); ++m) {
				try {
					addDirectory(*m);
				} catch(...) {
				}

			}
			refreshDirs = false;
		} else {
			for(StringIter l = loadDirs.begin(); l != loadDirs.end(); ++l) {
				try {
					addDirectory(*l);
				} catch(...) {
				}
			}
			loadDirs.clear();
		}

		listN++;

		try {
			string indent;

			string newXmlName = Util::getAppPath() + "files" + Util::toString(listN) + ".xml.bz2";
			{
				FilteredOutputStream<BZFilter, true> newXmlFile(new File(newXmlName, File::WRITE, File::TRUNCATE | File::CREATE));
				newXmlFile.write(SimpleXML::utf8Header);
				newXmlFile.write("<FileListing Version=\"1\" Generator=\"" APPNAME " " VERSIONSTRING "\">\r\n");

				for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
					i->second->toString(tmp, &newXmlFile, indent);
				}
				newXmlFile.write("</FileListing>");
				newXmlFile.flush();
			}

			if(xFile != NULL) {
				delete xFile;
				xFile = NULL;
				File::deleteFile(getBZXmlFile());
			}
			xFile = new File(newXmlName, File::READ, File::OPEN);
			setBZXmlFile(newXmlName);
			bzXmlListLen = File::getSize(newXmlName);

			string newName = Util::getAppPath() + "MyList" + Util::toString(listN) + ".DcLst";
			CryptoManager::getInstance()->encodeHuffman(tmp, tmp2);
			File(newName, File::WRITE, File::CREATE | File::TRUNCATE).write(tmp2);

			if(lFile != NULL) {
				delete lFile;
				lFile = NULL;
				File::deleteFile(getListFile());
			}
			lFile = new File(newName, File::READ, File::OPEN);
			setListFile(newName);
			listLen = File::getSize(newName);
		} catch(const Exception&) {
			// No new file lists...
		}
		
		dirty = false;
		lastUpdate = GET_TICK();
	}

	LogManager::getInstance()->message(STRING(FILE_LIST_REFRESH_FINISHED));
	if(update) {
		ClientManager::getInstance()->infoUpdated();
	}
	return 0;
}
static const string& escaper(const string& n, string& tmp) {
	if(Util::needsUtf8(n) || SimpleXML::needsEscape(n, false, false)) {
		tmp = n;
		return SimpleXML::escape(Util::toUtf8(tmp), false, false);
	}
	return n;
}

#define LITERAL(n) n, sizeof(n)-1
void ShareManager::Directory::toString(string& tmp, OutputStream* xmlFile, string& indent) {
	string tmp2;

	tmp.append(indent);
	tmp.append(name);
	tmp.append(LITERAL("\r\n"));

	xmlFile->write(indent);
	xmlFile->write(LITERAL("<Directory Name=\""));
	xmlFile->write(escaper(name, tmp2));
	xmlFile->write(LITERAL("\">\r\n"));
	
	indent += '\t';
	for(MapIter i = directories.begin(); i != directories.end(); ++i) {
		i->second->toString(tmp, xmlFile, indent);
	}
	
	Directory::File::Iter j = files.begin();
	while(j != files.end()) {
		const Directory::File* f = &(*j);
		bool dupe = false;
		if(f->getTTH() != NULL) {
			dcassert(ShareManager::getInstance()->tthIndex.find(f->getTTH()) != ShareManager::getInstance()->tthIndex.end());
			dupe = (&(*ShareManager::getInstance()->tthIndex[f->getTTH()]) != f);
		}

		if(dupe) {
			size-=f->getSize();
			if(!(BOOLSETTING(LIST_DUPES))) {
				const Directory::File* d = &(*ShareManager::getInstance()->tthIndex[f->getTTH()]);
				LogManager::getInstance()->message(STRING(DUPLICATE_FILE_NOT_SHARED) + f->getParent()->getFullName() + f->getName() + " (" + STRING(SIZE) + ": " + Util::toString(f->getSize()) + " " + STRING(B) + ") " + STRING(DUPLICATE_MATCH) + d->getParent()->getFullName() + d->getName() );
			}
		}

		if(dupe && !(BOOLSETTING(LIST_DUPES))) {
				//j = files.erase(j);
				files.erase(j++);
		} else {
			tmp.append(indent);
			tmp.append(f->getName());
			tmp.append(LITERAL("|"));
			tmp.append(Util::toString(f->getSize()));
			tmp.append(LITERAL("\r\n"));

			xmlFile->write(indent);
			xmlFile->write(LITERAL("<File Name=\""));
			xmlFile->write(escaper(f->getName(), tmp2));
			xmlFile->write(LITERAL("\" Size=\""));
			xmlFile->write(Util::toString(f->getSize()));
			if(f->getTTH()) {
				tmp2.clear();
				xmlFile->write(LITERAL("\" TTH=\""));
				xmlFile->write(f->getTTH()->toBase32(tmp2));
			}
			xmlFile->write(LITERAL("\"/>\r\n"));

			++j;
		}
	}
	indent.erase(indent.length()-1);
	xmlFile->write(indent);
	xmlFile->write(LITERAL("</Directory>\r\n"));
}


// These ones we can look up as ints (4 bytes...)...

static const char* typeAudio[] = { ".mp3", ".mp2", ".mid", ".wav", ".ogg", ".wma" };
static const char* typeCompressed[] = { ".zip", ".ace", ".rar" };
static const char* typeDocument[] = { ".htm", ".doc", ".txt", ".nfo" };
static const char* typeExecutable[] = { ".exe" };
static const char* typePicture[] = { ".jpg", ".gif", ".png", ".eps", ".img", ".pct", ".psp", ".pic", ".tif", ".rle", ".bmp", ".pcx" };
static const char* typeVideo[] = { ".mpg", ".mov", ".asf", ".avi", ".pxp", ".wmv", ".ogm", ".mkv" };

static const string type2Audio[] = { ".au", ".aiff", ".flac" };
static const string type2Picture[] = { ".ai", ".ps", ".pict" };
static const string type2Video[] = { ".rm", ".divx", ".mpeg" };

#define IS_TYPE(x) ( type == (*((u_int32_t*)x)) )
#define IS_TYPE2(x) (Util::stricmp(aString.c_str() + aString.length() - x.length(), x.c_str()) == 0)

static bool checkType(const string& aString, int aType) {
	if(aType == SearchManager::TYPE_ANY)
		return true;

	if(aString.length() < 5)
		return false;
	
	const char* c = aString.c_str() + aString.length() - 3;
	u_int32_t type = '.' | (Util::toLower(c[0]) << 8) | (Util::toLower(c[1]) << 16) | (((u_int32_t)Util::toLower(c[2])) << 24);

	switch(aType) {
	case SearchManager::TYPE_AUDIO:
		{
			for(size_t i = 0; i < (sizeof(typeAudio) / sizeof(typeAudio[0])); i++) {
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
			for(size_t i = 0; i < (sizeof(typePicture) / sizeof(typePicture[0])); i++) {
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
			for(size_t i = 0; i < (sizeof(typeVideo) / sizeof(typeVideo[0])); i++) {
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
	if(aFileName[aFileName.length() - 1] == PATH_SEPARATOR) {
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

u_int32_t ShareManager::getMask(StringSearch::List& l) {
	u_int32_t mask = 0;
	int n = 1;

	for(StringIter i = words.begin(); i != words.end(); ++i, n++) {
		for(StringSearch::Iter j = l.begin(); j != l.end(); ++j) {
			if(Util::findSubString(j->getPattern(), *i) != string::npos) {
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
void ShareManager::Directory::search(SearchResult::List& aResults, StringSearch::List& aStrings, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults, u_int32_t mask) throw() {
	// Skip everything if there's nothing to find here (doh! =)
	if(!hasType(aFileType))
		return;

	if(!hasSearchType(mask))
		return;

	StringSearch::List* cur = &aStrings;
	auto_ptr<StringSearch::List> newStr;

	// Find any matches in the directory name
	for(StringSearch::Iter k = aStrings.begin(); k != aStrings.end(); ++k) {
		if(k->match(name)) {
			if(!newStr.get()) {
				newStr = auto_ptr<StringSearch::List>(new StringSearch::List(aStrings));
			}
			dcassert(find(newStr->begin(), newStr->end(), *k) != newStr->end());
			newStr->erase(find(newStr->begin(), newStr->end(), *k));
			u_int32_t xmask = ShareManager::getInstance()->getMask(k->getPattern());
			if(xmask != 1) {
				mask &= ~xmask;
			}
		}
	}

	if(newStr.get() != 0) {
		cur = newStr.get();
	}

	bool sizeOk = (aSearchType != SearchManager::SIZE_ATLEAST) || (aSize == 0);
	if( (cur->empty()) && 
		(((aFileType == SearchManager::TYPE_ANY) && sizeOk) || (aFileType == SearchManager::TYPE_DIRECTORY)) ) {
		// We satisfied all the search words! Add the directory...
		SearchResult* sr = new SearchResult(aClient, SearchResult::TYPE_DIRECTORY, 
			0, getFullName(), NULL);
		aResults.push_back(sr);
		ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
	}

	if(aFileType != SearchManager::TYPE_DIRECTORY) {
		for(File::Iter i = files.begin(); i != files.end(); ++i) {
			
			if(aSearchType == SearchManager::SIZE_ATLEAST && aSize > i->getSize()) {
				continue;
			} else if(aSearchType == SearchManager::SIZE_ATMOST && aSize < i->getSize()) {
				continue;
			}	
			StringSearch::Iter j = cur->begin();
			for(; j != cur->end() && j->match(i->getName()); ++j) 
				;	// Empty
			
			if(j != cur->end())
				continue;
			
			// Check file type...
			if(checkType(i->getName(), aFileType)) {
				
				SearchResult* sr = new SearchResult(aClient, SearchResult::TYPE_FILE, 
					i->getSize(), getFullName() + i->getName(), i->getTTH());
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

void ShareManager::search(SearchResult::List& results, const string& aString, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) {
	RLock l(cs);
	if(aFileType == SearchManager::TYPE_HASH) {
		if(aString.compare(0, 4, "TTH:") == 0) {
			TTHValue tth(aString.substr(4));
			HashFileIter i = tthIndex.find(&tth);
			if(i != tthIndex.end()) {
				dcassert(i->second->getTTH() != NULL);

				SearchResult* sr = new SearchResult(aClient, SearchResult::TYPE_FILE, 
					i->second->getSize(), i->second->getParent()->getFullName() + i->second->getName(), 
					i->second->getTTH());

				results.push_back(sr);
				ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
			}
		}
		return;
	}
	StringTokenizer t(Util::toLower(aString), '$');
	StringList& sl = t.getTokens();
	if(!bloom.match(sl))
		return;

	StringSearch::List ssl;
	for(StringList::iterator i = sl.begin(); i != sl.end(); ++i) {
		if(!i->empty()) {
			ssl.push_back(StringSearch(*i));
		}
	}
	if(ssl.empty())
		return;
	u_int32_t mask = getMask(sl);

	for(Directory::MapIter j = directories.begin(); (j != directories.end()) && (results.size() < maxResults); ++j) {
		j->second->search(results, ssl, aSearchType, aSize, aFileType, aClient, maxResults, mask);
	}
}

namespace {
	u_int16_t toCode(char a, char b) { return (u_int16_t)a | ((u_int16_t)b)<<8; }
}

ShareManager::AdcSearch::AdcSearch(const StringList& params) : include(&includeX), gt(0), 
	lt(numeric_limits<int64_t>::max()), hasRoot(false), isDirectory(false)
{
	for(StringIterC i = params.begin(); i != params.end(); ++i) {
		const string& p = *i;
		if(p.length() <= 2)
			continue;

		u_int16_t cmd = toCode(p[0], p[1]);
		if(toCode('T', 'L') == cmd) {
			hasRoot = true;
			root = p.substr(2);
			return;
		} else if(toCode('+', '+') == cmd) {
			includeX.push_back(StringSearch(p.substr(2)));		
		} else if(toCode('-', '-') == cmd) {
			exclude.push_back(StringSearch(p.substr(2)));
		} else if(toCode('E', 'X') == cmd) {
			ext.push_back(p.substr(2));
		} else if(toCode('>', '=') == cmd) {
			gt = Util::toInt64(p.substr(2));
		} else if(toCode('<', '=') == cmd) {
			lt = Util::toInt64(p.substr(2));
		} else if(toCode('=', '=') == cmd) {
			lt = gt = Util::toInt64(p.substr(2));
		} else if(toCode('D', 'O') == cmd) {
			isDirectory = (p[2] != '0');
		}
	}
}

void ShareManager::Directory::search(SearchResult::List& aResults, AdcSearch& aStrings, Client* aClient, StringList::size_type maxResults, u_int32_t mask) throw() {
	if(!hasSearchType(mask))
		return;

	StringSearch::List* cur = aStrings.include;
	StringSearch::List* old = aStrings.include;

	auto_ptr<StringSearch::List> newStr;

	// Find any matches in the directory name
	for(StringSearch::Iter k = cur->begin(); k != cur->end(); ++k) {
		if(k->match(name) && !aStrings.isExcluded(name)) {
			if(!newStr.get()) {
				newStr = auto_ptr<StringSearch::List>(new StringSearch::List(*cur));
			}
			dcassert(find(newStr->begin(), newStr->end(), *k) != newStr->end());
			newStr->erase(find(newStr->begin(), newStr->end(), *k));
			u_int32_t xmask = ShareManager::getInstance()->getMask(k->getPattern());
			if(xmask != 1) {
				mask &= ~xmask;
			}
		}
	}

	if(newStr.get() != 0) {
		cur = newStr.get();
	}

	bool sizeOk = (aStrings.gt == 0);
	if( cur->empty() && aStrings.ext.empty() && sizeOk ) {
		// We satisfied all the search words! Add the directory...
		SearchResult* sr = new SearchResult(aClient, SearchResult::TYPE_DIRECTORY, 
			0, getFullName(), NULL);
		aResults.push_back(sr);
		ShareManager::getInstance()->setHits(ShareManager::getInstance()->getHits()+1);
	}

	if(!aStrings.isDirectory) {
		for(File::Iter i = files.begin(); i != files.end(); ++i) {

			if(!(i->getSize() >= aStrings.gt)) {
				continue;
			} else if(!(i->getSize() <= aStrings.lt)) {
				continue;
			}	

			if(aStrings.isExcluded(i->getName()))
				continue;

			StringSearch::Iter j = cur->begin();
			for(; j != cur->end() && j->match(i->getName()); ++j) 
				;	// Empty

			if(j != cur->end())
				continue;

			// Check file type...
			if(aStrings.hasExt(i->getName())) {

				SearchResult* sr = new SearchResult(aClient, SearchResult::TYPE_FILE, 
					i->getSize(), getFullName() + i->getName(), i->getTTH());
				aResults.push_back(sr);
				ShareManager::getInstance()->addHits(1);
				if(aResults.size() >= maxResults) {
					return;
				}
			}
		}
	}

	for(Directory::MapIter l = directories.begin(); (l != directories.end()) && (aResults.size() < maxResults); ++l) {
		l->second->search(aResults, aStrings, aClient, maxResults, mask);
	}
	aStrings.include = old;
}

void ShareManager::search(SearchResult::List& results, const StringList& params, Client* aClient, StringList::size_type maxResults) {
	AdcSearch srch(params);	

	RLock l(cs);

	if(srch.hasRoot) {
		HashFileIter i = tthIndex.find(&srch.root);
		if(i != tthIndex.end()) {
			dcassert(i->second->getTTH() != NULL);

			SearchResult* sr = new SearchResult(aClient,
				SearchResult::TYPE_FILE, i->second->getSize(), i->second->getParent()->getFullName() + i->second->getName(), 
				i->second->getTTH());
			results.push_back(sr);
			ShareManager::getInstance()->addHits(1);
		}
		return;
	}

	for(StringSearch::Iter i = srch.includeX.begin(); i != srch.includeX.end(); ++i) {
		if(!bloom.match(i->getPattern()))
			return;
	}

	u_int32_t mask = getMask(srch.includeX);

	for(Directory::MapIter j = directories.begin(); (j != directories.end()) && (results.size() < maxResults); ++j) {
		j->second->search(results, srch, aClient, maxResults, mask);
	}
}

ShareManager::Directory* ShareManager::getDirectory(const string& fname) {
	for(Directory::MapIter mi = directories.begin(); mi != directories.end(); ++mi) {
		if(Util::strnicmp(fname, mi->first, mi->first.length()) == 0 && fname[mi->first.length()] == PATH_SEPARATOR) {
			Directory* d = mi->second;

			string::size_type i;
			string::size_type j = mi->first.length() + 1;
			while( (i = fname.find(PATH_SEPARATOR, j)) != string::npos) {
				mi = d->directories.find(fname.substr(j, i-j));
				j = i + 1;
				if(mi == d->directories.end())
					return NULL;
				d = mi->second;
			}
			return d;
		}
	}
	return NULL;
}

void ShareManager::on(DownloadManagerListener::Complete, Download* d) throw() {
	if(BOOLSETTING(ADD_FINISHED_INSTANTLY)) {
		// Check if finished download is supposed to be shared
		WLock l(cs);
		const string& n = d->getTarget();
		for(Directory::MapIter i = directories.begin(); i != directories.end(); i++) {
			if(strnicmp(i->first.c_str(), n.c_str(), i->first.size()) == 0 && n[i->first.size()] == PATH_SEPARATOR) {
				string s = n.substr(i->first.size()+1);
				try {
					addFinishedFile(i->second, s, d->getSize());
					// Schedule for hashing
					HashManager::getInstance()->getTTH(n, d->getSize(), 0);
					setDirty();
				} catch(const Exception&) {
					// Not a vital feature...
				}
				break;
			}
		}
	}
}

void ShareManager::on(HashManagerListener::TTHDone, const string& fname, TTHValue* root) throw() {
	WLock l(cs);
	Directory* d = getDirectory(fname);
	if(d != NULL) {
		Directory::File::Iter i = find_if(d->files.begin(), d->files.end(), Directory::File::StringComp(Util::getFileName(fname)));
		if(i != d->files.end()) {
			if(i->getTTH() != NULL) {
				dcassert(tthIndex.find(i->getTTH()) != tthIndex.end());
				tthIndex.erase(i->getTTH());
			}
			// Get rid of false constness...
			Directory::File* f = const_cast<Directory::File*>(&(*i));
			f->setTTH(root);
			tthIndex.insert(make_pair(root, i));
		} else {
			string name = Util::getFileName(fname);
			d->addSearchType(getMask(name));
			d->addType(getType(name));
			int64_t size = File::getSize(fname);
			Directory::File::Iter it = d->files.insert(Directory::File(name, size, d, root)).first;

			tthIndex.insert(make_pair(root, it));

			d->size+=size;

			bloom.add(Util::toLower(name));
		}
	}
}

void ShareManager::on(TimerManagerListener::Minute, u_int32_t tick) throw() {
	if(BOOLSETTING(AUTO_UPDATE_LIST)) {
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
 * $Id: ShareManager.cpp,v 1.93 2004/08/08 17:59:08 arnetheduck Exp $
 */

