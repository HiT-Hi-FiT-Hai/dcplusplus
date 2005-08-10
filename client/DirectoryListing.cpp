/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "ClientManager.h"

#include "StringTokenizer.h"
#include "SimpleXML.h"
#include "FilteredFile.h"
#include "BZUtils.h"
#include "CryptoManager.h"
#include "ResourceManager.h"

#ifdef ff
#undef ff
#endif

User::Ptr DirectoryListing::getUserFromFilename(const string& fileName) {
	// General file list name format: [username].[CID].[xml|xml.bz2|DcLst]

	string name = Util::getFileName(fileName);

	// Strip off any extensions
	if(Util::stricmp(name.c_str() + name.length() - 6, ".DcLst") == 0) {
		name.erase(name.length() - 6);
	}

	if(Util::stricmp(name.c_str() + name.length() - 4, ".bz2") == 0) {
		name.erase(name.length() - 4);
	}

	if(Util::stricmp(name.c_str() + name.length() - 4, ".xml") == 0) {
		name.erase(name.length() - 4);
	}

	// Find CID
	string::size_type i = name.rfind('.');
	if(i == string::npos) {
		return NULL;
	}

	size_t n = name.length() - (i + 1);
	// CID's always 13 chars long...
	if(n != 13)
		return NULL;

	CID cid(name.substr(i + 1));
	if(cid.isZero())
		return NULL;

	User::Ptr p = ClientManager::getInstance()->getUser(cid);
	if(p->getFirstNick().empty())
		p->setFirstNick(name.substr(0, i));
	return p;
}

void DirectoryListing::loadFile(const string& name) {
	string txt;

	// For now, we detect type by ending...
	string ext = Util::getFileExt(name);
	if(Util::stricmp(ext, ".DcLst") == 0) {
		size_t len = (size_t)::File::getSize(name);
		if(len == (size_t)-1)
			return;
		AutoArray<u_int8_t> buf(len);
		::File(name, ::File::READ, ::File::OPEN).read(buf, len);
		CryptoManager::getInstance()->decodeHuffman(buf, txt, len);
		load(txt);
		return;
	} 
	
	if(Util::stricmp(ext, ".bz2") == 0) {
		::File ff(name, ::File::READ, ::File::OPEN);
		FilteredInputStream<UnBZFilter, false> f(&ff);
		const size_t BUF_SIZE = 64*1024;
		char buf[BUF_SIZE];
		size_t len;
		for(;;) {
			size_t n = BUF_SIZE;
			len = f.read(buf, n);
			txt.append(buf, len);
			if(len < BUF_SIZE)
				break;
		}
	} else if(Util::stricmp(ext, ".xml") == 0) {
		int64_t sz = ::File::getSize(name);
		if(sz == -1 || sz >= txt.max_size())
			throw(FileException(CSTRING(FILE_NOT_AVAILABLE)));
		txt.resize((size_t) sz);
		size_t n = txt.length();
		::File(name, ::File::READ, ::File::OPEN).read(&txt[0], n);
	}

	loadXML(txt, false);
}

void DirectoryListing::load(const string& in) {
	StringTokenizer<string> t(in, '\n');

	StringList& tokens = t.getTokens();
	string::size_type indent = 0;

	root->setComplete(true);

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
		}

		string::size_type k = tok.find('|', j);
		if(k != string::npos) {
			// this must be a file...
			cur->files.push_back(new File(cur, tok.substr(j, k-j), Util::toInt64(tok.substr(k+1))));
		} else {
			// A directory
			string name = tok.substr(j, tok.length()-j-1);
			fullPath += '\\';
			fullPath += name;

			Directory::Iter di = ::find(cur->directories.begin(), cur->directories.end(), name);
			if(di != cur->directories.end()) {
				cur = *di;
			} else {
				Directory* d = new Directory(cur, name, false, true);
				cur->directories.push_back(d);
				cur = d;
			}
			indent++;
		}
	}
}

class ListLoader : public SimpleXMLReader::CallBack {
public:
	ListLoader(DirectoryListing::Directory* root, bool aUpdating) : cur(root), base("/"), inListing(false), updating(aUpdating) { 
	};

	virtual ~ListLoader() { }

	virtual void startTag(const string& name, StringPairList& attribs, bool simple);
	virtual void endTag(const string& name, const string& data);

	const string& getBase() const { return base; }
private:
	DirectoryListing::Directory* cur;

	StringMap params;
	string base;
	bool inListing;
	bool updating;
};

string DirectoryListing::loadXML(const string& xml, bool updating) {
	setUtf8(true);

	ListLoader ll(getRoot(), updating);
	SimpleXMLReader(&ll).fromXML(xml);
	return ll.getBase();
}

static const string sFileListing = "FileListing";
static const string sBase = "Base";
static const string sDirectory = "Directory";
static const string sIncomplete = "Incomplete";
static const string sFile = "File";
static const string sName = "Name";
static const string sSize = "Size";
static const string sTTH = "TTH";

void ListLoader::startTag(const string& name, StringPairList& attribs, bool simple) {
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
		} else if(name == sDirectory) {
			const string& n = getAttrib(attribs, sName, 0);
			if(n.empty()) {
				throw SimpleXMLException("Directory missing name attribute");
			}
			bool incomp = getAttrib(attribs, sIncomplete, 1) == "1";
			DirectoryListing::Directory* d = NULL;
			if(updating) {
				for(DirectoryListing::Directory::Iter i  = cur->directories.begin(); i != cur->directories.end(); ++i) {
					if((*i)->getName() == n) {
						d = *i;
						if(!d->getComplete())
							d->setComplete(!incomp);
						break;
					}
				}
			}
			if(d == NULL) {
				d = new DirectoryListing::Directory(cur, n, false, !incomp);
				cur->directories.push_back(d);
			}
			cur = d;

			if(simple) {
				// To handle <Directory Name="..." />
				endTag(name, Util::emptyString);
			}
		}
	} else if(name == sFileListing) {
		const string& b = getAttrib(attribs, sBase, 2);
		if(b.size() >= 1 && b[0] == '/' && b[b.size()-1] == '/') {
			base = b;
		}
		StringList sl = StringTokenizer<string>(base.substr(1), '/').getTokens();
		for(StringIter i = sl.begin(); i != sl.end(); ++i) {
			DirectoryListing::Directory* d = NULL;
			for(DirectoryListing::Directory::Iter j = cur->directories.begin(); j != cur->directories.end(); ++j) {
				if((*j)->getName() == *i) {
					d = *j;
					break;
				}
			}
			if(d == NULL) {
				d = new DirectoryListing::Directory(cur, *i, false, false);
				cur->directories.push_back(d);
			}
			cur = d;
		}
		cur->setComplete(true);
		inListing = true;

		if(simple) {
			// To handle <Directory Name="..." />
			endTag(name, Util::emptyString);
		}
	}
}

void ListLoader::endTag(const string& name, const string&) {
	if(inListing) {
		if(name == sDirectory) {
			cur = cur->getParent();
		} else if(name == sFileListing) {
			// cur should be root now...
			inListing = false;
		}
	}
}

string DirectoryListing::getPath(const Directory* d) const {
	if(d == root)
		return "";

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
	return utf8 ? n : (tmp.clear(), Text::acpToUtf8(n, tmp));
}

void DirectoryListing::download(Directory* aDir, const string& aTarget, bool highPrio) {
	string tmp;
	string target = (aDir == getRoot()) ? aTarget : aTarget + escaper(aDir->getName(), tmp, getUtf8()) + PATH_SEPARATOR;
	// First, recurse over the directories
	Directory::List& lst = aDir->directories;
	sort(lst.begin(), lst.end(), Directory::DirSort());
	for(Directory::Iter j = lst.begin(); j != lst.end(); ++j) {
		download(*j, target, highPrio);
	}
	// Then add the files
	File::List& l = aDir->files;
	sort(l.begin(), l.end(), File::FileSort());
	for(File::Iter i = aDir->files.begin(); i != aDir->files.end(); ++i) {
		File* file = *i;
		try {
			download(file, target + escaper(file->getName(), tmp, getUtf8()), false, highPrio);
		} catch(const QueueException&) {
			// Catch it here to allow parts of directories to be added...
		} catch(const FileException&) {
			//..
		}
	}
}

void DirectoryListing::download(const string& aDir, const string& aTarget, bool highPrio) {
	dcassert(aDir.size() > 2);
	dcassert(aDir[aDir.size() - 1] == PATH_SEPARATOR);
	Directory* d = find(aDir, getRoot());
	if(d != NULL)
		download(d, aTarget, highPrio);
}

void DirectoryListing::download(File* aFile, const string& aTarget, bool view, bool highPrio) {
	int flags = (view ? (QueueItem::FLAG_TEXT | QueueItem::FLAG_CLIENT_VIEW) : QueueItem::FLAG_RESUME);

	QueueManager::getInstance()->add(aTarget, aFile->getSize(), aFile->getTTH(), getUser(), 
		getPath(aFile) + aFile->getName(), getUtf8(), flags);

	if(highPrio)
		QueueManager::getInstance()->setPriority(aTarget, QueueItem::HIGHEST);
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

struct HashContained {
	HashContained(const HASH_SET<TTHValue, TTHValue::Hash>& l) : tl(l) { }
	const HASH_SET<TTHValue, TTHValue::Hash>& tl;
	bool operator()(const DirectoryListing::File::Ptr i) const {
		bool r = !tl.count(*(i->getTTH()));
		if (r) DeleteFunction<const DirectoryListing::File*>()(i);
		return r;
	}
private:
	HashContained& operator=(HashContained&);
};

struct DirectoryEmpty {
	bool operator()(const DirectoryListing::Directory::Ptr i) const {
		bool r = i->getFileCount() + i->directories.size() == 0;
		if (r) DeleteFunction<const DirectoryListing::Directory*>()(i);
		return r;
	}
};

void DirectoryListing::Directory::filterList(DirectoryListing& dirList) {
		DirectoryListing::Directory* d = dirList.getRoot();

		HASH_SET<TTHValue, TTHValue::Hash> l;
		d->getHashList(l);
		filterList(l);
}

void DirectoryListing::Directory::filterList(const HASH_SET<TTHValue, TTHValue::Hash>& l) {
	for(Iter i = directories.begin(); i != directories.end(); ++i) (*i)->filterList(l);
	directories.erase(std::remove_if(directories.begin(),directories.end(),DirectoryEmpty()),directories.end());
	files.erase(std::remove_if(files.begin(),files.end(),HashContained(l)),files.end());
}

void DirectoryListing::Directory::getHashList(HASH_SET<TTHValue, TTHValue::Hash>& l) {
	for(Iter i = directories.begin(); i != directories.end(); ++i) (*i)->getHashList(l);
	for(DirectoryListing::File::Iter i = files.begin(); i != files.end(); ++i) l.insert(*(*i)->getTTH());
}

int64_t DirectoryListing::Directory::getTotalSize(bool adl) {
	int64_t x = getSize();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		if(!(adl && (*i)->getAdls()))
			x += (*i)->getTotalSize(adls);
	}
	return x;
}

size_t DirectoryListing::Directory::getTotalFileCount(bool adl) {
	size_t x = getFileCount();
	for(Iter i = directories.begin(); i != directories.end(); ++i) {
		if(!(adl && (*i)->getAdls()))
			x += (*i)->getTotalFileCount(adls);
	}
	return x;
}

/**
 * @file
 * $Id: DirectoryListing.cpp,v 1.53 2005/08/10 17:30:55 arnetheduck Exp $
 */
