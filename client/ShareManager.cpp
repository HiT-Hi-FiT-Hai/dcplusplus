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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "ShareManager.h"
#include "CryptoManager.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "UploadManager.h"
#include "ClientManager.h"

ShareManager* ShareManager::instance = NULL;

enum {
	MAX_RESULTS = 5
};

string ShareManager::translateFileName(const string& aFile) throw(ShareException) {
	if(aFile == "MyList.DcLst") {
		return getListFile();
	} else {
		string::size_type i = aFile.find('\\');
		if(i == string::npos)
			throw ShareException("File Not Available");
		
		string aDir = aFile.substr(0, i);

		StringMapIter j = dirs.find(aDir);
		if(j == dirs.end()) {
			throw ShareException("File Not Available");
		}
		
		// Make sure they're not trying something funny with the path and the get command...
		if(aFile.find("..\\") != string::npos) {
			throw ShareException("Get Lost");
		}
		
		if(Util::findSubString(aFile, "DCPlusPlus.xml") != string::npos) {
			throw ShareException("Don't think so");
		}

		if(!checkFile(j->second, aFile.substr(i + 1))) {
			throw ShareException("File Not Available");
		}
		
		return j->second + aFile.substr(i);
	}

	throw ShareException("File Not Available");
}

bool ShareManager::checkFile(const string& dir, const string& aFile) {
	Lock l(cs);

	string::size_type i = 0;

	Directory::MapIter mi = directories.find(dir);
	if(mi == directories.end())
		return false;
	Directory* d = mi->second;
	string aDir;

	int j = 0;
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
	Lock l(cs);

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
	Lock l(cs);
	aXml->addTag("Share");
	aXml->stepIn();
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		aXml->addTag("Directory", i->first);
	}
	aXml->stepOut();
}

void ShareManager::addDirectory(const string& aDirectory) throw(ShareException) {
	if(aDirectory.size() == 0) {
		throw ShareException("No directory specified");
	}

	{
		Lock l(cs);

		string d;
		if(aDirectory[aDirectory.size() - 1] == '\\') {
			d = aDirectory.substr(0, aDirectory.size()-1);
		} else {
			d = aDirectory;
		}

		for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
			if(d.find(i->first + '\\') != string::npos) {
				throw ShareException("Directory already shared.");
			} else if(i->first.find(d + '\\') != string::npos) {
				throw ShareException("Remove all subdirectories before adding this one.");
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
	Lock l(cs);
	Directory::MapIter i = directories.find(aDirectory);
	if(i != directories.end()) {
		delete i->second;
		directories.erase(i);
	}
	
	dirty = true;
}

ShareManager::Directory* ShareManager::buildTree(const string& aName, Directory* aParent) {
	Directory* dir = new Directory(aName.substr(aName.rfind('\\') + 1), aParent);

	WIN32_FIND_DATA data;
	HANDLE hFind;
	
	hFind = FindFirstFile((aName + "\\*").c_str(), &data);
	
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			string name = data.cFileName;
			if(name == "." || name == "..")
				continue;
			if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				dir->directories[name] = buildTree(aName + '\\' + name, dir);
			} else {

				if( !((!BOOLSETTING(SHARE_HIDDEN)) && (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) ) {

					// Not a directory, assume it's a file...make sure we're not sharing the settings file...
					if(stricmp(name.c_str(), "DCPlusPlus.xml") != 0) {
						dir->files[name] = (LONGLONG)data.nFileSizeLow | ((LONGLONG)data.nFileSizeHigh)<<32;
						dir->size+=(LONGLONG)data.nFileSizeLow | ((LONGLONG)data.nFileSizeHigh)<<32;
					}
				}
			}
		} while(FindNextFile(hFind, &data));
	}
	
	FindClose(hFind);
	return dir;
}

StringList ShareManager::getDirectories() {
	Lock l(cs);

	StringList tmp;
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		tmp.push_back(i->first);
	}
	return tmp;
}

void ShareManager::refresh(bool dirs /* = false */, bool aUpdate /* = true */) throw(ShareException) {
	update = aUpdate;
	refreshDirs = dirs;
	if(dirty) {
		if(refreshThread) {
			WaitForSingleObject(refreshThread, INFINITE);
			CloseHandle(refreshThread);
		}
		
		DWORD id;
		refreshThread = CreateThread(NULL, 0, refresher, this, NULL, &id);
		
		// We don't want the compression to take up useful CPU time...
		SetThreadPriority(refreshThread, THREAD_PRIORITY_LOWEST);
	}
}

DWORD WINAPI ShareManager::refresher(void* p) {
	ShareManager* sm = (ShareManager*)p;

	string tmp, tmp2;
	{
		Lock l(sm->cs);

		if(sm->refreshDirs) {
			StringList dirs = sm->getDirectories();
			for(StringIter k = dirs.begin(); k != dirs.end(); ++k) {
				sm->removeDirectory(*k);
				sm->addDirectory(*k);
			}
			sm->refreshDirs = false;
		}
		
		for(Directory::MapIter i = sm->directories.begin(); i != sm->directories.end(); ++i) {
			tmp = tmp + i->second->toString();
		}
		
		CryptoManager::getInstance()->encodeHuffman(tmp, tmp2);
		try {
			File f(sm->getListFile(), File::WRITE, File::CREATE | File::TRUNCATE);
			f.write(tmp2);
		} catch(Exception e) {
			// ...
			return 1;
		}
		
		sm->listLen = tmp2.length();
		sm->dirty = false;
	}

	if(sm->update) {
		ClientManager::getInstance()->infoUpdated();
	}
	return 0;
}

string ShareManager::Directory::toString(int ident /* = 0 */) {
	string tmp(ident, '\t');
	tmp += name + "\r\n";
	char buf[24];

	for(MapIter i = directories.begin(); i != directories.end(); ++i) {
		tmp += i->second->toString(ident + 1);
	}
	for(map<string, LONGLONG>::iterator j = files.begin(); j != files.end(); ++j) {
		tmp += string(ident + 1, '\t') + j->first + "|" + _i64toa(j->second, buf, 10) + "\r\n";
	}

	return tmp;
}

#define IS_TYPE(x) (Util::findSubString(aString, x) != -1)

bool checkType(const string& aString, int aType) {
	bool found = true;
	switch(aType) {
	case SearchManager::TYPE_ANY: break;
	case SearchManager::TYPE_AUDIO:
		if(!( IS_TYPE(".mp3") || IS_TYPE(".mp2") || IS_TYPE(".mid") ||
			IS_TYPE(".wav") || IS_TYPE(".au") || IS_TYPE(".aiff") ) ) {
			found = false;
		}
		break;
	case SearchManager::TYPE_COMPRESSED:
		if(!( IS_TYPE(".zip") || IS_TYPE(".ace") || IS_TYPE(".rar") ) ) {
			found = false;
		}
		break;
	case SearchManager::TYPE_DOCUMENT:
		if(!( IS_TYPE(".htm") || IS_TYPE(".doc") ) ) {
			found = false;
		}
		break;
	case SearchManager::TYPE_EXECUTABLE:
		if(!( IS_TYPE(".exe") ) ) {
			found = false;
		}
		break;
	case SearchManager::TYPE_FOLDER:
		found = false;
		break;
	case SearchManager::TYPE_PICTURE:
		if(!( IS_TYPE(".eps") || IS_TYPE(".ai") || IS_TYPE(".ps") ||
			IS_TYPE(".img") || IS_TYPE(".pct") || IS_TYPE(".pict") ||
			IS_TYPE(".psp") || IS_TYPE(".pic") || IS_TYPE(".png") ||
			IS_TYPE(".tif") || IS_TYPE(".rle") || IS_TYPE(".bmp") ||
			IS_TYPE(".pcx") ) ) {
			found = false;
		}
		break;
	case SearchManager::TYPE_VIDEO:
		if(!( IS_TYPE(".mpg") || IS_TYPE(".mov") || IS_TYPE(".mpeg") ||
			IS_TYPE(".asf") || IS_TYPE(".avi") || IS_TYPE(".rm") ||
			IS_TYPE(".pxp") ) ) {
			found = false;
		}
		break;
	}
	return found;		
}

void ShareManager::Directory::search(SearchResult::List& aResults, StringList& aStrings, int aSearchType, LONGLONG aSize, int aFileType, Client* aClient) {
	bool found = true;

	if( (aFileType == SearchManager::TYPE_ANY) || (aFileType == SearchManager::TYPE_FOLDER) ) {
		for(StringIter k = aStrings.begin(); k != aStrings.end(); ++k) {
			if(Util::findSubString(name, *k) == -1) {
				found = false;
				break;
			}
		}
	} else {
		found = false;
	}

	if(found && aSearchType == SearchManager::SIZE_DONTCARE) {
		for(map<string, LONGLONG>::iterator i = files.begin(); i != files.end() && (aResults.size() < MAX_RESULTS); ++i) {
			if(checkType(i->first, aFileType)) {
				SearchResult* sr = new SearchResult();
				sr->setFile(getFullName() + i->first);
				sr->setSize(i->second);
				int slots = UploadManager::getInstance()->getFreeSlots();
				sr->setFreeSlots(slots <= 0 ? 0 : slots);
				sr->setSlots(SETTING(SLOTS));
				sr->setNick(aClient->getNick());
				sr->setHubAddress(aClient->getServer());
				sr->setHubName(aClient->getName());
				aResults.push_back(sr);
			}
		}	
	} else {
		
		for(map<string, LONGLONG>::iterator i = files.begin(); i != files.end(); ++i) {
			found = true;
			for(StringIter j = aStrings.begin(); (j != aStrings.end()); ++j) {
				if(aSearchType == SearchManager::SIZE_ATLEAST && i->second < aSize) {
					found = false;
					break;
				} else if(aSearchType == SearchManager::SIZE_ATMOST && i->second > aSize) {
					found = false;
					break;
				} else if(Util::findSubString(i->first, *j) == -1) {
					found = false;
					break;
				}
			}

			// Check file type...
			if(found && checkType(i->first, aFileType)) {

				SearchResult* sr = new SearchResult();
				sr->setFile(getFullName() + i->first);
				sr->setSize(i->second);
				int slots = UploadManager::getInstance()->getFreeSlots();
				sr->setFreeSlots(slots <= 0 ? 0 : slots);
				sr->setSlots(SETTING(SLOTS));
				sr->setNick(SETTING(NICK));
				sr->setNick(aClient->getNick());
				sr->setHubAddress(aClient->getIp());
				sr->setHubName(aClient->getName());
				aResults.push_back(sr);

				if(aResults.size() >= MAX_RESULTS) {
					break;
				}
			}
		}
	}

	for(Directory::MapIter l = directories.begin(); (l != directories.end()) && (aResults.size() < MAX_RESULTS); ++l) {
		l->second->search(aResults, aStrings, aSearchType, aSize, aFileType, aClient);
	}
}
SearchResult::List ShareManager::search(const string& aString, int aSearchType, LONGLONG aSize, int aFileType, Client* aClient) {
	Lock l(cs);

	StringTokenizer t(aString, '$');
	StringList& sl = t.getTokens();
	SearchResult::List results;

	for(Directory::MapIter i = directories.begin(); i != directories.end() && results.size() < MAX_RESULTS; ++i) {
		i->second->search(results, sl, aSearchType, aSize, aFileType, aClient);
	}
	return results;
}

/**
 * @file ShareManager.cpp
 * $Id: ShareManager.cpp,v 1.25 2002/02/09 18:13:51 arnetheduck Exp $
 * @if LOG
 * $Log: ShareManager.cpp,v $
 * Revision 1.25  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.24  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.23  2002/02/02 17:21:27  arnetheduck
 * Fixed search bugs and some other things...
 *
 * Revision 1.22  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.21  2002/01/26 16:34:01  arnetheduck
 * Colors dialog added, as well as some other options
 *
 * Revision 1.20  2002/01/26 14:59:23  arnetheduck
 * Fixed disconnect crash
 *
 * Revision 1.19  2002/01/26 12:06:40  arnetheduck
 * Småsaker
 *
 * Revision 1.18  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
 * Revision 1.17  2002/01/22 00:10:37  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
 *
 * Revision 1.16  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.15  2002/01/19 19:07:39  arnetheduck
 * Last fixes before 0.13
 *
 * Revision 1.14  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.13  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.12  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.11  2002/01/09 19:01:35  arnetheduck
 * Made some small changed to the key generation and search frame...
 *
 * Revision 1.10  2002/01/06 11:13:07  arnetheduck
 * Last fixes before 0.10
 *
 * Revision 1.9  2002/01/06 00:14:54  arnetheduck
 * Incoming searches almost done, just need some testing...
 *
 * Revision 1.8  2002/01/02 16:12:33  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.7  2001/12/30 17:41:16  arnetheduck
 * Fixed some XML parsing bugs
 *
 * Revision 1.6  2001/12/30 15:03:45  arnetheduck
 * Added framework to handle incoming searches
 *
 * Revision 1.5  2001/12/21 23:52:30  arnetheduck
 * Last commit for five days
 *
 * Revision 1.4  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.3  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.2  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
 *
 * Revision 1.1  2001/12/02 23:51:22  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * @endif
 */

