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
#include "File.h"

#define HASH_FILE_VERSION_STRING "1"
static const u_int32_t HASH_FILE_VERSION=1;

TTHValue* HashManager::getTTH(const string& aFileName, int64_t aSize, u_int32_t aTimeStamp) {
	Lock l(cs);
	TTHValue* root = store.getTTH(aFileName, aSize, aTimeStamp);
	if(root == NULL) {
		hasher.hashFile(aFileName);
	}
	return root;
}

void HashManager::hashDone(const string& aFileName, TigerTree& tth) {
	TTHValue* root = NULL;
	{
		Lock l(cs);
		store.addFile(aFileName, tth, true);
		root = store.getTTH(aFileName, tth.getFileSize(), tth.getTimeStamp());
	}

	if(root != NULL) {
		fire(HashManagerListener::TTH_DONE, aFileName, root);
	}
	LogManager::getInstance()->message(STRING(HASHING_FINISHED) + aFileName);
}

void HashManager::HashStore::addFile(const string& aFileName, TigerTree& tth, bool aUsed) {
	TTHIter i = indexTTH.find(aFileName);
	if(i == indexTTH.end()) {
		try {
			int64_t pos = addLeaves(tth.getLeaves());
			if(pos == 0)
				return;
			indexTTH.insert(make_pair(aFileName, new FileInfo(tth.getRoot(), tth.getFileSize(), pos, tth.getBlockSize(), tth.getTimeStamp(), aUsed)));
			dirty = true;
		} catch(const FileException&) {
			// Oops, lost it...
		}
	} else {
		try {
			i->second->setRoot(tth.getRoot());
			i->second->setBlockSize(tth.getBlockSize());
			i->second->setSize(tth.getFileSize());
			i->second->setIndex(addLeaves(tth.getLeaves()));
		} catch(const FileException&) {
			i->second->setIndex(0);
		}
		dirty = true;
	}
}

int64_t HashManager::HashStore::addLeaves(TigerTree::MerkleList& leaves) {
	File f(dataFile, File::RW, File::OPEN);
	f.setPos(0);
	int64_t pos = 0;
	size_t n = sizeof(pos);
	if(f.read(&pos, n) != sizeof(pos))
		return 0;

	// Check if we should grow the file, we grow by a meg at a time...
	int64_t datsz = f.getSize();
	if((pos + leaves.size() * TTHValue::SIZE) >= datsz) {
		f.setPos(datsz + 1024*1024);
		f.setEOF();
	}
	f.setPos(pos);
	dcassert(leaves.size() > 0);
	f.write(leaves[0].data, (leaves.size() * TTHValue::SIZE));
	int64_t p2 = f.getPos();
	f.setPos(0);
	f.write(&p2, sizeof(p2));
	return pos;
}

void HashManager::HashStore::rebuild() {
	int64_t maxPos = sizeof(maxPos);
	try {
		File f(dataFile, File::RW, File::OPEN);
		size_t len =(size_t) (f.getSize() - 8);
		AutoArray<u_int8_t> buf(len);
		f.read((u_int8_t*)buf, len);
		f.setPos(0);
		f.write(&maxPos, sizeof(maxPos));
		f.close();
		size_t dataLen = 0;
		for(TTHIter i = indexTTH.begin(); i != indexTTH.end(); ++i) {
			FileInfo* fi = i->second;
			dataLen = TTHValue::SIZE * TigerTree::calcBlocks(fi->getSize(), fi->getBlockSize());
			if(fi->getUsed() && fi->getIndex() + dataLen < len) {
				TigerTree tt(fi->getSize(), fi->getTimeStamp(), fi->getBlockSize(), buf + fi->getIndex());
				if(tt.getRoot() == fi->getRoot()) {
					maxPos = addLeaves(tt.getLeaves());
					fi->setIndex(maxPos);
				} else {
					fi->setIndex(0);
				}
			} else {
				fi->setIndex(0);
			}
		}
		maxPos += dataLen;
		// Truncate file down to closest mb boundrary
		maxPos = ((maxPos + 1024*1024 - 1) / (1024*1024)) * 1024*1024;
		File f2(dataFile, File::WRITE, File::OPEN);
		f2.setPos(maxPos);
		f2.setEOF();
	} catch(const FileException&) {
	}
	dirty = true;
}

#define LITERAL(x) x, sizeof(x)-1
#define CHECKESCAPE(n) SimpleXML::escape(n, tmp, true)

void HashManager::HashStore::save() {
	if(dirty) {
		try {
			File ff(indexFile + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
			BufferedOutputStream<false> f(&ff);

			string tmp;
			string b32tmp;

			f.write(SimpleXML::w1252Header);
			f.write(LITERAL("<HashStore version=\"" HASH_FILE_VERSION_STRING "\">\r\n"));
			for(TTHIter i = indexTTH.begin(); i != indexTTH.end(); ++i) {
				if(i->second->getIndex() == 0)
					continue;

				f.write(LITERAL("\t<File Name=\""));
				f.write(CHECKESCAPE(i->first));
				f.write(LITERAL("\" Size=\""));
				f.write(Util::toString(i->second->getSize()));
				f.write(LITERAL("\" TimeStamp=\""));
				f.write(Util::toString(i->second->getTimeStamp()));
				f.write(LITERAL("\"><Hash Type=\"TTH\" Index=\""));
				f.write(Util::toString(i->second->getIndex()));
				f.write(LITERAL("\" LeafSize=\""));
				f.write(Util::toString((u_int32_t)i->second->getBlockSize()));
				f.write(LITERAL("\" Root=\""));
				b32tmp.clear();
				f.write(i->second->getRoot().toBase32(b32tmp));
				f.write(LITERAL("\"/></File>\r\n"));
			}
			f.write(LITERAL("</HashStore>"));
			f.flush();
			ff.close();
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
	u_int32_t timeStamp;
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
static const string sTimeStamp = "TimeStamp";
static const string sRoot = "Root";

void HashLoader::startTag(const string& name, StringPairList& attribs, bool simple) {
	if(name == sFile) {
		file = getAttrib(attribs, sName, 0);
		size = Util::toInt64(getAttrib(attribs, sSize, 1));
		timeStamp = (u_int32_t)Util::toInt(getAttrib(attribs, sTimeStamp, 2));
	} else if(name == sHash) {
		const string& type = getAttrib(attribs, sType, 0);
		size_t blockSize = (size_t)Util::toInt(getAttrib(attribs, sBlockSize, 1));
		int64_t index = Util::toInt64(getAttrib(attribs, sIndex, 2));
		const string& root = getAttrib(attribs, sRoot, 3);
		if(!file.empty() && (type == sTTH) && (blockSize >= 1024) && (index >= 8) && !root.empty()) {
			/** @todo Verify root against data file */
			store.indexTTH.insert(make_pair(file, new HashManager::HashStore::FileInfo(TTHValue(root), size, index, blockSize, timeStamp, false)));
		}
	}
}

void HashLoader::endTag(const string& name, const string& data) {
	if(name == sFile) {
		file.clear();
	}
}

HashManager::HashStore::HashStore() : indexFile(Util::getAppPath() + "HashIndex.xml"), 
dataFile(Util::getAppPath() + "HashData.dat"), dirty(false) 
{ 
	if(File::getSize(dataFile) <= 0) {
		try {
			createDataFile(dataFile);
		} catch(const FileException&) {
			// ?
		}
	}
};

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
void HashManager::HashStore::createDataFile(const string& name) {
	try {
		File dat(name, File::WRITE, File::CREATE | File::TRUNCATE);
		dat.setPos(1024*1024);
		dat.setEOF();
		dat.setPos(0);
		int64_t start = sizeof(start);
		dat.write(&start, sizeof(start));

	} catch(const FileException&) {
		/** @todo All further hashing will unfortunately be wasted(!) */
	}
}

#define BUF_SIZE (128*1024)

int HashManager::Hasher::run() {
	setThreadPriority(Thread::LOW);

#ifdef _WIN32
	u_int8_t* buf = (u_int8_t*)VirtualAlloc(NULL, BUF_SIZE, MEM_COMMIT, PAGE_READWRITE);
	if(buf == NULL)
		return 1;
#else
	u_int8_t buf[BUF_SIZE];
#endif

	string fname;
	for(;;) {
		s.wait();
		if(stop)
			break;
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
			try {
#ifdef _WIN32
				bool fastFile = true;
				HANDLE h = INVALID_HANDLE_VALUE;
				DWORD x, y;
				if(!GetDiskFreeSpace(Util::getFilePath(fname).c_str(), &y, &x, &y, &y)) {
					fastFile = false;
				} else {
					if((BUF_SIZE % x) != 0) {
						fastFile = false;
					} else {
						h = ::CreateFile(fname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
						if(h == INVALID_HANDLE_VALUE)
							fastFile = false;
					}
				}
#endif

				File f(fname, File::READ, File::OPEN);
				size_t bs = max(TigerTree::calcBlockSize(f.getSize(), 10), (size_t)MIN_BLOCK_SIZE);
				TigerTree tth(bs, f.getLastModified());
				size_t n = 0;
				do {
#ifdef _WIN32
					if(fastFile) {
						if(!::ReadFile(h, buf, BUF_SIZE, (DWORD*)&n, NULL)) {
							fastFile = false;
							n = 0;
						}
					} else {
#endif
					size_t bufSize = BUF_SIZE;
					n = f.read(buf, bufSize);
#ifdef _WIN32
					}
#endif
					tth.update(buf, n);
				} while (n > 0 && !stop);
#ifdef _WIN32
				if(h != INVALID_HANDLE_VALUE) {
					CloseHandle(h);
					h = INVALID_HANDLE_VALUE;
				}
#endif
				f.close();
				tth.finalize();			
				HashManager::getInstance()->hashDone(fname, tth);
			} catch(const FileException&) {
				// Ignore, it'll be readded on the next share refresh...
			}
		}
	}
#ifdef _WIN32
	VirtualFree(buf, 0, MEM_RELEASE);
#endif
	return 0;
}

/**
 * @file
 * $Id: HashManager.cpp,v 1.12 2004/03/19 08:48:57 arnetheduck Exp $
 */
