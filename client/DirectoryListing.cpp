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

#include "DirectoryListing.h"

#include "QueueManager.h"
#include "SearchManager.h"

#include "StringTokenizer.h"
#include "ADLSearch.h"
#include "SimpleXML.h"
#include "FilteredFile.h"
#include "BZUtils.h"
#include "CryptoManager.h"

void DirectoryListing::loadFile(const string& name) {
	string txt;

	// For now, we detect type by ending...
	string ext = Util::getFileExt(name);
	if(Util::stricmp(ext, ".DcLst") == 0) {
		int64_t len = ::File::getSize(name) + 1;
		AutoArray<u_int8_t> buf(len);
		::File(name, ::File::READ, ::File::OPEN).read(buf, len);
		CryptoManager::getInstance()->decodeHuffman(buf, txt);
		load(txt);
	} else if(Util::stricmp(ext, ".bz2") == 0) {
		FilteredFileReader<UnBZFilter> f(name, ::File::READ, ::File::OPEN);
		const int BUF_SIZE = 64*1024;
		char buf[BUF_SIZE];
		u_int32_t len;
		for(;;) {
			len = f.read(buf, BUF_SIZE);
			txt.append(buf, len);
			if(len < BUF_SIZE)
				break;
		}

		if(txt.compare(0, 5, "<?xml") == 0) {
			loadXML(txt);
		} else {
			load(txt);
		}
	}
}

void DirectoryListing::load(const string& in) {
	StringTokenizer t(in);

	StringList& tokens = t.getTokens();
	string::size_type indent = 0;

	// Prepare ADLSearch manager
	ADLSearchManager* pADLSearch = ADLSearchManager::getInstance();
	ADLSearchManager::DestDirList destDirs;

	StringMap params;
	params["nick"] = getUser()->getNick();
	pADLSearch->setUser(getUser());

	pADLSearch->PrepareDestinationDirectories(destDirs, root, params);
	pADLSearch->setBreakOnFirst(BOOLSETTING(ADLS_BREAK_ON_FIRST));

	Directory* cur = root;
	string fullPath;
	
	File::Iter lastFileIter = cur->files.begin();

	for(StringIter i = tokens.begin(); i != tokens.end(); ++i) 
	{
		string& tok = *i;
		string::size_type j = tok.find_first_not_of('\t');
		if(j == string::npos) {
			break;
		}

		while(j < indent) {
			// Wind up directory structure
			cur = cur->getParent();
			dcassert(cur != NULL);
			indent--;
			string::size_type l = fullPath.find_last_of('\\');
			if(l != string::npos) {
				fullPath.erase(fullPath.begin() + l, fullPath.end());
			}
			pADLSearch->StepUpDirectory(destDirs);

			lastFileIter = cur->files.begin();
		}

		string::size_type k = tok.find('|', j);
		if(k != string::npos) {
			// this must be a file...
			lastFileIter = cur->files.insert(lastFileIter, new File(cur, tok.substr(j, k-j), Util::toInt64(tok.substr(k+1))));

			// ADLSearch
			pADLSearch->MatchesFile(destDirs, *lastFileIter, fullPath);
		} else {
			// A directory
			Directory* d = new Directory(cur, tok.substr(j, tok.length()-j-1));
			Directory::Iter di = cur->directories.find(d);
			if(di != cur->directories.end()) {
				delete d;
				d = *di;
			}
			cur->directories.insert(d);
			cur = d;
			lastFileIter = cur->files.begin();

			indent++;
			fullPath += "\\" + d->getName();

			// ADLSearch
			pADLSearch->MatchesDirectory(destDirs, d, fullPath);
		}
	}

	// Finalize ADLSearch manager
	pADLSearch->FinalizeDestinationDirectories(destDirs, root);
}

class ListLoader : public SimpleXMLReader::CallBack {
public:
	ListLoader(DirectoryListing::Directory* root, const User::Ptr& user) : cur(root), inListing(false) { 
		params["nick"] = user->getNick();
		ADLSearchManager::getInstance()->setUser(user);

		ADLSearchManager::getInstance()->PrepareDestinationDirectories(destDirs, root, params);
		ADLSearchManager::getInstance()->setBreakOnFirst(BOOLSETTING(ADLS_BREAK_ON_FIRST));

		lastFileIter = cur->files.begin();
	};

	virtual void startTag(const string& name, StringPairList& attribs, bool simple);
	virtual void endTag(const string& name, const string& data);
private:
	string fullPath;

	ADLSearchManager::DestDirList destDirs;
	DirectoryListing::Directory* cur;
	DirectoryListing::File::Iter lastFileIter;

	StringMap params;
	bool inListing;
};

void DirectoryListing::loadXML(const string& xml) {
	setUtf8(true);

	ListLoader ll(getRoot(), getUser());
	SimpleXMLReader(&ll).fromXML(xml);
}

static const string sFileListing = "FileListing";
static const string sDirectory = "Directory";
static const string sFile = "File";
static const string sName = "Name";
static const string sSize = "Size";
static const string sTTHRoot = "TTHRoot";

void ListLoader::startTag(const string& name, StringPairList& attribs, bool simple) {
	if(inListing) {
		if(name == sFile) {
			const string& n = getAttrib(attribs, sName);
			if(n.empty())
				return;
			const string& s = getAttrib(attribs, sSize);
			if(s.empty())
				return;
			const string& h = getAttrib(attribs, sTTHRoot);
			DirectoryListing::File* f = h.empty() ? new DirectoryListing::File(cur, n, Util::toInt64(s)) : new DirectoryListing::File(cur, n, Util::toInt64(s), h);
			lastFileIter = cur->files.insert(lastFileIter, f);
			ADLSearchManager::getInstance()->MatchesFile(destDirs, f, fullPath);
		} else if(name == sDirectory) {
			const string& n = getAttrib(attribs, sName);
			if(n.empty())
				return;
			DirectoryListing::Directory* d = new DirectoryListing::Directory(cur, n);
			DirectoryListing::Directory::Iter di = cur->directories.find(d);
			if(di != cur->directories.end()) {
				delete d;
				d = *di;
			}
			cur->directories.insert(d);
			cur = d;
			lastFileIter = cur->files.begin();

			fullPath += '\\';
			fullPath += d->getName();

			// ADLSearch
			ADLSearchManager::getInstance()->MatchesDirectory(destDirs, d, fullPath);
		}
	} else if(name == sFileListing) {
		inListing = true;
	}
}

void ListLoader::endTag(const string& name, const string& data) {
	if(inListing) {
		if(name == sDirectory) {
			cur = cur->getParent();
			dcassert(fullPath.find('\\') != string::npos);
			fullPath.erase(fullPath.rfind('\\'));
			ADLSearchManager::getInstance()->StepUpDirectory(destDirs);
			lastFileIter = cur->files.begin();
		} else if(name == sFileListing) {
			// cur should be root now...
			ADLSearchManager::getInstance()->FinalizeDestinationDirectories(destDirs, cur);
		}
	}
}

string DirectoryListing::getPath(Directory* d) {
	string dir;
	dir.reserve(128);
	dir.append(d->getName());
	dir.append(1, '\\');

	Directory* cur = d->getParent();
	while(cur!=root) {
		dir.insert(0, cur->getName() + '\\');
		cur = cur->getParent();
	}
	return dir;
}

static const string& escaper(const string& s, string& tmp, bool utf8) {
	if(utf8 && Util::needsAcp(s)) {
		tmp = s;
		return Util::toAcp(tmp);
	}
	return s;
}

void DirectoryListing::download(Directory* aDir, const string& aTarget) {
	string tmp;
	string target = (aDir == getRoot()) ? aTarget : aTarget + escaper(aDir->getName(), tmp, getUtf8()) + '\\';
	// First, recurse over the directories
	for(Directory::Iter j = aDir->directories.begin(); j != aDir->directories.end(); ++j) {
		download(*j, target);
	}
	// Then add the files
	for(File::Iter i = aDir->files.begin(); i != aDir->files.end(); ++i) {
		File* file = *i;
		try {
			download(file, target + escaper(file->getName(), tmp, getUtf8()));
		} catch(const QueueException&) {
			// Catch it here to allow parts of directories to be added...
		} catch(const FileException&) {
			//..
		}
	}
}

void DirectoryListing::download(const string& aDir, const string& aTarget) {
	dcassert(aDir.size() > 2);
	dcassert(aDir[aDir.size() - 1] == '\\');
	Directory* d = find(aDir, getRoot());
	if(d != NULL)
		download(d, aTarget);
}

DirectoryListing::Directory* DirectoryListing::find(const string& aName, Directory* current) {
	string::size_type end = aName.find('\\');
	dcassert(end != string::npos);
	Name n(aName.substr(0, end));
	Directory::Iter i = current->directories.find((Directory*)&n);
	if(i != current->directories.end()) {
		if(end == (aName.size() - 1))
			return *i;
		else
			return find(aName.substr(end + 1), *i);
	}
	return NULL;
}

int64_t DirectoryListing::Directory::getTotalSize(bool adls) {
	int64_t x = getSize();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		if(!(adls && (*i)->getAdls()))
			x += (*i)->getTotalSize(adls);
	}
	return x;
}

int DirectoryListing::Directory::getTotalFileCount(bool adls) {
	int x = getFileCount();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		if(!(adls && (*i)->getAdls()))
			x += (*i)->getTotalFileCount(adls);
	}
	return x;
}

void DirectoryListing::download(File* aFile, const string& aTarget, bool view /* = false */) {
	string tmp;	
	int flags = getUtf8() ? QueueItem::FLAG_SOURCE_UTF8 : 0;
	flags |= (view ? (QueueItem::FLAG_TEXT | QueueItem::FLAG_CLIENT_VIEW) : QueueItem::FLAG_RESUME);
	QueueManager::getInstance()->add(getPath(aFile) + escaper(aFile->getName(), tmp, getUtf8()), aFile->getSize(), user, aTarget, 
		aFile->getTTHRoot(), Util::emptyString, flags);
}

/**
 * @file
 * $Id: DirectoryListing.cpp,v 1.24 2004/02/16 13:21:39 arnetheduck Exp $
 */
