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

#include "ShareManager.h"
#include "CryptoManager.h"
#include "SimpleXML.h"

ShareManager* ShareManager::instance = NULL;

string ShareManager::translateFileName(const string& aFile) throw(ShareException) {
	if(aFile == "MyList.DcLst") {
		return Settings::getAppPath() + "\\MyList.DcLst";
	} else {
		string::size_type i = aFile.find('\\');
		if(i == string::npos)
			throw ShareException("File Not Available");
		
		string aDir = aFile.substr(0, i);
		StringMapIter j = dirs.find(aDir);
		if(j == dirs.end()) {
			throw ShareException("File Not Available");
		}

		return j->second + aFile.substr(i);
	}

	throw ShareException("File Not Available");
}

void ShareManager::load(SimpleXML* aXml) {
	if(aXml->findChild("Share")) {
		aXml->stepIn();
		while(aXml->findChild("Directory")) {
			string name = aXml->getChildData();
			directories[name] = buildTree(name, NULL);
			string dir = name.substr(name.rfind('\\') + 1);
			dirs[dir] = name;
		}
		aXml->stepOut();
	}
}

void ShareManager::save(SimpleXML* aXml) {
	aXml->addTag("Share");
	aXml->stepIn();
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		aXml->addTag("Directory", i->first);
	}
	aXml->stepOut();
}

void ShareManager::addDirectory(const string& aDirectory) throw(ShareException) {
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		if(aDirectory.find(i->first) != string::npos) {
			throw ShareException("Directory already shared.");
		} else if(i->first.find(aDirectory) != string::npos) {
			throw ShareException("Remove all subdirectories before adding this one.");
		}
	}

	directories[aDirectory] = buildTree(aDirectory, NULL);
	string dir = aDirectory.substr(aDirectory.rfind('\\') + 1);
	dirs[dir] = aDirectory;
}

void ShareManager::removeDirectory(const string& aDirectory) {
	Directory::MapIter i = directories.find(aDirectory);
	if(i != directories.end()) {
		delete i->second;
		directories.erase(i);
	}
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
				dir->directories[name] = buildTree(aName + "\\" + name, dir);
			} else {
				// Not a directory, assume it's a file...
				dir->files[name] = (LONGLONG)data.nFileSizeLow | ((LONGLONG)data.nFileSizeHigh)<<32;
				dir->size+=(LONGLONG)data.nFileSizeLow | ((LONGLONG)data.nFileSizeHigh)<<32;
			}
			
		} while(FindNextFile(hFind, &data));
	}
	
	FindClose(hFind);
	return dir;
}

StringList ShareManager::getDirectories() {
	StringList tmp;
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		tmp.push_back(i->first);
	}
	return tmp;
}


void ShareManager::refresh() throw(ShareException) {
	string tmp, tmp2;
	DWORD d;
	
	for(Directory::MapIter i = directories.begin(); i != directories.end(); ++i) {
		tmp = tmp + i->second->toString();
	}

	CryptoManager::getInstance()->encodeHuffman(tmp, tmp2);

	HANDLE hf = CreateFile((Settings::getAppPath() + "\\MyList.DcLst").c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hf == INVALID_HANDLE_VALUE) {
		throw ShareException("Could not write MyList.DcLst");
	}
	WriteFile(hf, tmp2.c_str(), tmp2.length(), &d, NULL);
	CloseHandle(hf);
	listLen = tmp2.length();
}

string ShareManager::Directory::toString(int ident /* = 0 */) {
	string tmp(ident, '\t');
	tmp = tmp + name + "\r\n";
	char buf[24];

	for(MapIter i = directories.begin(); i != directories.end(); ++i) {
		tmp = tmp + i->second->toString(ident + 1);
	}
	for(map<string, LONGLONG>::iterator j = files.begin(); j != files.end(); ++j) {
		tmp = tmp + string(ident + 1, '\t') + j->first + "|" + _i64toa(j->second, buf, 10) + "\r\n";
	}

	return tmp;
}

/**
 * @file ShareManager.cpp
 * $Id: ShareManager.cpp,v 1.3 2001/12/04 21:50:34 arnetheduck Exp $
 * @if LOG
 * $Log: ShareManager.cpp,v $
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

