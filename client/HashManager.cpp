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

#include "HashManager.h"
#include "CryptoManager.h"
#include "SimpleXML.h"
#include "LogManager.h"

HashManager* Singleton<HashManager>::instance = NULL;

#define HASH_FILE_VERSION_STRING "1"
static const u_int32_t HASH_FILE_VERSION=1;

TTHValue* HashManager::getTTHRoot(const string& aFileName, int64_t aSize) {
	Lock l(cs);
	TTHValue* root = store.getTTHRoot(aFileName, aSize);
	if(root == NULL && BOOLSETTING(HASH_FILES)) {
		hasher.hashFile(aFileName);
	}
	return root;
}

void HashManager::hashDone(const string& aFileName, TigerTree& tth) {
	TTHValue* root = NULL;
	{
		Lock l(cs);
		store.addFile(aFileName, tth);
		root = store.getTTHRoot(aFileName, tth.getFileSize());
	}

	if(root != NULL) {
		fire(HashManagerListener::TTH_DONE, aFileName, root);
	}
	LogManager::getInstance()->message("Finished hashing " + aFileName);
}

void HashManager::HashStore::addFile(const string& aFileName, TigerTree& tth) {
	TTHIter i = indexTTH.find(aFileName);
	if(i == indexTTH.end()) {
		try {
			File f(dataFile, File::RW, File::OPEN);
			f.setPos(0);
			int64_t pos = 0;
			if(f.read(&pos, sizeof(pos)) != sizeof(pos))
				return;

			// Check if we should grow the file, we grow by a meg at a time...
			int64_t datsz = f.getSize();
			if((pos + tth.getLeaves().size() * TigerTree::HashValue::SIZE) >= datsz) {
				f.setPos(datsz + 1024*1024);
				f.setEOF();
			}
			f.setPos(pos);
			dcassert(tth.getLeaves().size() > 0);
			f.write(tth.getLeaves()[0].data, (tth.getLeaves().size() * TigerTree::HashValue::SIZE));
			int64_t p2 = f.getPos();
			f.setPos(0);
			f.write(&p2, sizeof(p2));
			indexTTH.insert(make_pair(aFileName, new FileInfo(tth.getRoot(), tth.getFileSize(), pos, tth.getBlockSize())));
			dirty = true;
		} catch(const FileException&) {
			// Oops, lost it...
		}

	} else {
		i->second->setRoot(tth.getRoot());
		i->second->setBlockSize(tth.getBlockSize());
		i->second->setSize(tth.getFileSize());
		dirty = true;
	}
}

static const string& escaper(const string& n, string& tmp) {
	tmp = n;
	SimpleXML::escape(tmp, true);
	return tmp;
}

#define LITERAL(x) x, sizeof(x)-1
#define CHECKESCAPE(n) SimpleXML::needsEscape(n, true) ? escaper(n, tmp) : n

void HashManager::HashStore::save() {
	if(dirty) {
		try {
			BufferedFile f(indexFile + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
			string tmp;

			f.write(LITERAL("<?xml version=\"1.0\" encoding=\"windows-1252\" standalone=\"yes\"?>\r\n"));
			f.write(LITERAL("<HashStore version=\"" HASH_FILE_VERSION_STRING "\">"));
			for(TTHIter i = indexTTH.begin(); i != indexTTH.end(); ++i) {
				f.write(LITERAL("\t<File Name=\""));
				f.write(CHECKESCAPE(i->first));
				f.write(LITERAL("\" Size=\""));
				f.write(Util::toString(i->second->getSize()));
				f.write(LITERAL("\"><Hash Type=\"TTH\" Index=\""));
				f.write(Util::toString(i->second->getIndex()));
				f.write(LITERAL("\" LeafSize=\""));
				f.write(Util::toString((u_int32_t)i->second->getBlockSize()));
				f.write(LITERAL("\">"));
				f.write(i->second->getRoot().toBase32());
				f.write(LITERAL("</Hash></File>\r\n"));
			}
			f.write(LITERAL("</HashStore>"));
			f.flushBuffers();
			f.close();
			File::deleteFile(indexFile);
			File::renameFile(indexFile + ".tmp", indexFile);

			dirty = false;
		} catch(const FileException&) {
			// Too bad...
		}
	}
}

class HashLoader : public SimpleXMLReader::CallBack {
public:
	HashLoader(HashManager::HashStore& s) : store(s) { };
	virtual void startTag(const string& name, StringPairList& attribs, bool simple);
	virtual void endTag(const string& name, const string& data);
	
private:
	HashManager::HashStore& store;

	string file;
	int64_t size;
	size_t blockSize;
	string type;
	int64_t index;
};

void HashManager::HashStore::load() {
	try {
		HashLoader l(*this);
		SimpleXMLReader(&l).fromXML(File(indexFile, File::READ, File::OPEN).read());
	} catch(const Exception&) {
		// ...
	}
}

static const string sFile = "File";
static const string sName = "Name";
static const string sSize = "Size";
static const string sHash = "Hash";
static const string sType = "Type";
static const string sTTH = "TTH";
static const string sIndex = "Index";
static const string sBlockSize = "LeafSize";

void HashLoader::startTag(const string& name, StringPairList& attribs, bool simple) {
	if(name == sFile) {
		file = getAttrib(attribs, sName);
		size = Util::toInt64(getAttrib(attribs, sSize));
	} else if(name == sHash) {
		type = getAttrib(attribs, sType);
		blockSize = (size_t)Util::toInt(getAttrib(attribs, sBlockSize));
		index = Util::toInt64(getAttrib(attribs, sIndex));
		if(index < 8)
			return;
	}
}
void HashLoader::endTag(const string& name, const string& data) {
	if(name == sHash && !file.empty()) {
		// Check if it exists...
		if((type == sTTH) && (blockSize >= 1024) && (index >= 8)) {
			/** @todo Verify root against data file */
			store.indexTTH.insert(make_pair(file, new HashManager::HashStore::FileInfo(TTHValue(data), size, index, blockSize)));
		}
	} else if(name == sFile) {
		file.clear();
	}
}

/**
 * Creates the data files for storing hash values.
 * The data file is very simple in its format. The first 8 bytes
 * are filled with an int64_t (little endian) of the next write position
 * in the file counting from the start (so that file can be grown in chunks).
 * We start with a 1 mb file, and then grow it as needed to avoid fragmentation.
 * To find data inside the file, use the corresponding index file.
 * Since file is never deleted, space will eventually be wasted, so a rebuild
 * should occasionally be done.
 */
void HashManager::HashStore::createFiles() {
	try {
		File dat(dataFile, File::WRITE, File::CREATE | File::TRUNCATE);
		dat.setPos(1024*1024);
		dat.setEOF();
		dat.setPos(0);
		int64_t start = sizeof(start);
		dat.write(&start, sizeof(start));

	} catch(const FileException&) {
		/** @todo All further hashing will unfortunately be wasted(!) */
	}
}

int HashManager::Hasher::run() {
	setThreadPriority(Thread::LOW);

	string fname;
	for(;;) {
		s.wait();
		if(stop)
			return 0;

		{
			Lock l(cs);
			if(!w.empty()) {
				fname = *w.begin();
				w.erase(w.begin());
			} else {
				fname.clear();
			}
		}

		if(!fname.empty()) {
			TigerTree tth;
			try {
				tth.hashFile(fname, 10);
				HashManager::getInstance()->hashDone(fname, tth);

			} catch(const FileException&) {
				// Ignore, it'll be readded on the next share refresh...
			}
		}
	}
}

/**
 * @file
 * $Id: HashManager.cpp,v 1.4 2004/01/28 20:19:20 arnetheduck Exp $
 */
