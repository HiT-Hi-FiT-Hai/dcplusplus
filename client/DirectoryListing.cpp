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

void DirectoryListing::loadFile(const string& name, bool doAdl) {
	string txt;

	// For now, we detect type by ending...
	string ext = Util::getFileExt(name);
	if(Util::stricmp(ext, ".DcLst") == 0) {
		size_t len = (size_t)::File::getSize(name);
		AutoArray<u_int8_t> buf(len);
		::File(name, ::File::READ, ::File::OPEN).read(buf, len);
		CryptoManager::getInstance()->decodeHuffman(buf, txt);
		load(txt, doAdl);
	} else if(Util::stricmp(ext, ".bz2") == 0) {
		::File ff(name, ::File::READ, ::File::OPEN);
		FilteredInputStream<UnBZFilter, false> f(&ff);
		const size_t BUF_SIZE = 64*1024;
		char buf[BUF_SIZE];
		u_int32_t len;
		for(;;) {
			size_t n = BUF_SIZE;
			len = f.read(buf, n);
			txt.append(buf, len);
			if(len < BUF_SIZE)
				break;
		}

		loadXML(txt, doAdl);
	}
}

void DirectoryListing::load(const string& in, bool doAdl) {
	StringTokenizer<string> t(in, '\n');

	StringList& tokens = t.getTokens();
	string::size_type indent = 0;

	// Prepare ADLSearch manager
	ADLSearchManager* pADLSearch = ADLSearchManager::getInstance();
	ADLSearchManager::DestDirList destDirs;

	StringMap params;
	if(doAdl) {
		params["nick"] = getUser()->getNick();
		pADLSearch->setUser(getUser());

		pADLSearch->PrepareDestinationDirectories(destDirs, root, params);
		pADLSearch->setBreakOnFirst(BOOLSETTING(ADLS_BREAK_ON_FIRST));
	}

	Directory* cur = root;
	string fullPath;
	
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
			if(doAdl)
				pADLSearch->StepUpDirectory(destDirs);
		}

		string::size_type k = tok.find('|', j);
		if(k != string::npos) {
			// this must be a file...
			cur->files.push_back(new File(cur, tok.substr(j, k-j), Util::toInt64(tok.substr(k+1))));

			// ADLSearch
			if(doAdl)
				pADLSearch->MatchesFile(destDirs, cur->files.back(), fullPath);
		} else {
			// A directory
			string name = tok.substr(j, tok.length()-j-1);
			fullPath += '\\';
			fullPath += name;

			Directory::Iter di = ::find(cur->directories.begin(), cur->directories.end(), name);
			if(di != cur->directories.end()) {
				cur = *di;
			} else {
				Directory* d = new Directory(cur, name);
				cur->directories.push_back(d);
				cur = d;
			}
			if(doAdl)
				pADLSearch->MatchesDirectory(destDirs, cur, fullPath);
			indent++;
		}
	}

	// Finalize ADLSearch manager
	if(doAdl)
		pADLSearch->FinalizeDestinationDirectories(destDirs, root);
}

class ListLoader : public SimpleXMLReader::CallBack {
public:
	ListLoader(DirectoryListing::Directory* root, const User::Ptr& user, bool aDoAdl) : cur(root), inListing(false), doAdl(aDoAdl) { 
		if(doAdl) {
			params["nick"] = user->getNick();
			ADLSearchManager::getInstance()->setUser(user);

			ADLSearchManager::getInstance()->PrepareDestinationDirectories(destDirs, root, params);
			ADLSearchManager::getInstance()->setBreakOnFirst(BOOLSETTING(ADLS_BREAK_ON_FIRST));
		}

		lastFileIter = cur->files.begin();
	};

	virtual ~ListLoader() { }

	virtual void startTag(const string& name, StringPairList& attribs, bool simple);
	virtual void endTag(const string& name, const string& data);
private:
	string fullPath;

	ADLSearchManager::DestDirList destDirs;
	DirectoryListing::Directory* cur;
	DirectoryListing::File::Iter lastFileIter;

	StringMap params;
	bool inListing;
	bool doAdl;
};

void DirectoryListing::loadXML(const string& xml, bool doAdl) {
	setUtf8(true);

	ListLoader ll(getRoot(), getUser(), doAdl);
	SimpleXMLReader(&ll).fromXML(xml);
}

static const string sFileListing = "FileListing";
static const string sDirectory = "Directory";
static const string sFile = "File";
static const string sName = "Name";
static const string sSize = "Size";
static const string sTTH = "TTH";

void ListLoader::startTag(const string& name, StringPairList& attribs, bool) {
	if(inListing) {
		if(name == sFile) {
			const string& n = getAttrib(attribs, sName, 0);
			if(n.empty())
				return;
			const string& s = getAttrib(attribs, sSize, 1);
			if(s.empty())
				return;
			const string& h = getAttrib(attribs, sTTH, 2);
			DirectoryListing::File* f = h.empty() ? new DirectoryListing::File(cur, n, Util::toInt64(s)) : new DirectoryListing::File(cur, n, Util::toInt64(s), h);
			cur->files.push_back(f);
			if(doAdl)
				ADLSearchManager::getInstance()->MatchesFile(destDirs, f, fullPath);
		} else if(name == sDirectory) {
			const string& n = getAttrib(attribs, sName, 0);
			if(n.empty()) {
				throw SimpleXMLException("Directory missing name attribute");
			}
			DirectoryListing::Directory* d = new DirectoryListing::Directory(cur, n);
			cur->directories.push_back(d);
			cur = d;
			fullPath += '\\';
			fullPath += d->getName();

			// ADLSearch
			if(doAdl)
				ADLSearchManager::getInstance()->MatchesDirectory(destDirs, d, fullPath);
		}
	} else if(name == sFileListing) {
		inListing = true;
	}
}

void ListLoader::endTag(const string& name, const string&) {
	if(inListing) {
		if(name == sDirectory) {
			cur = cur->getParent();
			dcassert(fullPath.find('\\') != string::npos);
			fullPath.erase(fullPath.rfind('\\'));
			if(doAdl)
				ADLSearchManager::getInstance()->StepUpDirectory(destDirs);
			lastFileIter = cur->files.begin();
		} else if(name == sFileListing) {
			// cur should be root now...
			if(doAdl)
				ADLSearchManager::getInstance()->FinalizeDestinationDirectories(destDirs, cur);
			inListing = false;
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

static inline const string& escaper(const string& n, string& tmp, bool utf8) {
	return utf8 ? n : Util::toUtf8(n, tmp);
}

void DirectoryListing::download(Directory* aDir, const string& aTarget) {
	string tmp;
	string target = (aDir == getRoot()) ? aTarget : aTarget + escaper(aDir->getName(), tmp, getUtf8()) + '\\';
	// First, recurse over the directories
	Directory::List& lst = aDir->directories;
	sort(lst.begin(), lst.end(), Directory::DirSort());
	for(Directory::Iter j = lst.begin(); j != lst.end(); ++j) {
		download(*j, target);
	}
	// Then add the files
	File::List& l = aDir->files;
	sort(l.begin(), l.end(), File::FileSort());
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

void DirectoryListing::download(File* aFile, const string& aTarget, bool view /* = false */) {
	int flags = (getUtf8() ? QueueItem::FLAG_SOURCE_UTF8 : 0) |
		(view ? (QueueItem::FLAG_TEXT | QueueItem::FLAG_CLIENT_VIEW) : QueueItem::FLAG_RESUME);

	if(getUtf8()) {
		QueueManager::getInstance()->add(getPath(aFile) + aFile->getName(), aFile->getSize(), user, aTarget, 
			aFile->getTTH(), Util::emptyString, flags);
	} else {
		string tmp;
		QueueManager::getInstance()->add(Util::toUtf8(getPath(aFile) + aFile->getName(), tmp), aFile->getSize(), user, aTarget, 
			aFile->getTTH(), Util::emptyString, flags);
	}
}

DirectoryListing::Directory* DirectoryListing::find(const string& aName, Directory* current) {
	string::size_type end = aName.find('\\');
	dcassert(end != string::npos);
	string name = aName.substr(0, end);

	Directory::Iter i = ::find(current->directories.begin(), current->directories.end(), name);
	if(i != current->directories.end()) {
		if(end == (aName.size() - 1))
			return *i;
		else
			return find(aName.substr(end + 1), *i);
	}
	return NULL;
}

int64_t DirectoryListing::Directory::getTotalSize(bool adl) {
	int64_t x = getSize();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		if(!(adl && (*i)->getAdls()))
			x += (*i)->getTotalSize(adls);
	}
	return x;
}

int DirectoryListing::Directory::getTotalFileCount(bool adl) {
	int x = getFileCount();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		if(!(adl && (*i)->getAdls()))
			x += (*i)->getTotalFileCount(adls);
	}
	return x;
}

/**
 * @file
 * $Id: DirectoryListing.cpp,v 1.32 2004/09/06 12:32:42 arnetheduck Exp $
 */
