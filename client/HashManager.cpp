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

#include "HashManager.h"
#include "ResourceManager.h"
#include "SimpleXML.h"
#include "LogManager.h"
#include "File.h"

#define HASH_FILE_VERSION_STRING "2"
static const u_int32_t HASH_FILE_VERSION=2;

bool HashManager::checkTTH(const string& aFileName, int64_t aSize, u_int32_t aTimeStamp) {
	Lock l(cs);
	if(!store.checkTTH(aFileName, aSize, aTimeStamp)) {
		hasher.hashFile(aFileName, aSize);
		return false;
	}
	return true;
}

const TTHValue& HashManager::getTTH(const string& aFileName, int64_t aSize) throw(HashException) {
	Lock l(cs);
	const TTHValue* tth = store.getTTH(aFileName);
	if(tth == NULL){
		hasher.hashFile(aFileName, aSize);
		throw HashException(Util::emptyString);
	}
	return *tth;
}

bool HashManager::getTree(const TTHValue& root, TigerTree& tt) {
	Lock l(cs);
	return store.getTree(root, tt);
}

void HashManager::hashDone(const string& aFileName, u_int32_t aTimeStamp, const TigerTree& tth, int64_t speed) {
	const TTHValue* root = NULL;
	{
		Lock l(cs);
		store.addFile(aFileName, aTimeStamp, tth, true);
		root = store.getTTH(aFileName);
	}

	if(root != NULL) {
		fire(HashManagerListener::TTHDone(), aFileName, *root);
	}

	string fn = aFileName;
	if(count(fn.begin(), fn.end(), PATH_SEPARATOR) >= 2) {
		string::size_type i = fn.rfind(PATH_SEPARATOR);
		i = fn.rfind(PATH_SEPARATOR, i-1);
		fn.erase(0, i);
		fn.insert(0, "...");
	}
	if(speed > 0) {
		LogManager::getInstance()->message(STRING(HASHING_FINISHED) + fn + " (" + Util::formatBytes(speed) + "/s)");
	} else if(speed <= 0) {
		LogManager::getInstance()->message(STRING(HASHING_FINISHED) + fn);
	}
}

void HashManager::HashStore::addFile(const string& aFileName, u_int32_t aTimeStamp, const TigerTree& tth, bool aUsed) {
	if(!addTree(tth))
		return;

	string fname = Text::toLower(Util::getFileName(aFileName));
	string fpath = Text::toLower(Util::getFilePath(aFileName));

	FileInfoList& fileList = fileIndex[fpath];

	FileInfoIter j = find_if(fileList.begin(), fileList.end(), FileInfo::StringComp(fname));
	if(j != fileList.end()) {
		fileList.erase(j);
	}

	fileList.push_back(FileInfo(fname, tth.getRoot(), aTimeStamp, aUsed));
	dirty = true;
}

bool HashManager::HashStore::addTree(const TigerTree& tth) {
	try {
		if(treeIndex.find(tth.getRoot()) == treeIndex.end()) {
			int64_t index = addLeaves(tth.getLeaves());
			if(index == STORE_FAILED)
				return false;

			dcassert((index != SMALL_TREE) || ((tth.getLeaves().size() == 1) && (tth.getRoot() == tth.getLeaves()[0])));

			treeIndex.insert(make_pair(tth.getRoot(), TreeInfo(tth.getFileSize(), index, tth.getBlockSize())));
		}
	} catch(const Exception& ) {
		return false;
	}
	dirty = true;
	return true;
}

int64_t HashManager::HashStore::addLeaves(const TigerTree::MerkleList& leaves) {
	if(leaves.size() == 1)
		return SMALL_TREE;

	File f(dataFile, File::RW, File::OPEN);
	f.setPos(0);
	int64_t pos = 0;
	size_t n = sizeof(pos);
	if(f.read(&pos, n) != sizeof(pos))
		return STORE_FAILED;

	// Check if we should grow the file, we grow by a meg at a time...
	int64_t datsz = f.getSize();
	if((pos + (int64_t)(leaves.size() * TTHValue::SIZE)) >= datsz) {
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

bool HashManager::HashStore::getTree(const TTHValue& root, TigerTree& tth) {
	TreeIter i = treeIndex.find(root);
	if(i == treeIndex.end())
		return false;
	
	TreeInfo& ti = i->second;

	if(ti.getIndex() == SMALL_TREE) {
		tth = TigerTree(ti.getSize(), ti.getBlockSize(), i->first);
		return true;
	}

	try {
		File f(dataFile, File::READ, File::OPEN);

		f.setPos(ti.getIndex());
		size_t datalen = TigerTree::calcBlocks(ti.getSize(), ti.getBlockSize()) * TTHValue::SIZE;
		AutoArray<u_int8_t> buf(datalen);
		f.read((u_int8_t*)buf, datalen);
		tth = TigerTree(ti.getSize(), ti.getBlockSize(), buf);
		if(!(tth.getRoot() == i->first))
			return false;
	} catch(const FileException& ) {
		return false;
	}
	return true;
}

bool HashManager::HashStore::checkTTH(const string& aFileName, int64_t aSize, u_int32_t aTimeStamp) {
	string fname = Text::toLower(Util::getFileName(aFileName));
	string fpath = Text::toLower(Util::getFilePath(aFileName));
	DirIter i = fileIndex.find(fpath);
	if(i != fileIndex.end()) {
		FileInfoIter j = find_if(i->second.begin(), i->second.end(), FileInfo::StringComp(fname));
		if(j != i->second.end()) {
			FileInfo& fi = *j;
			TreeIter ti = treeIndex.find(fi.getRoot());
			if(ti == treeIndex.end() || ti->second.getSize() != aSize || fi.getTimeStamp() != aTimeStamp) {
				i->second.erase(j);
				dirty = true;
				return false;
			}
			return true;
		}
	} 
	return false;
}

const TTHValue* HashManager::HashStore::getTTH(const string& aFileName) {
	string fname = Text::toLower(Util::getFileName(aFileName));
	string fpath = Text::toLower(Util::getFilePath(aFileName));

	DirIter i = fileIndex.find(fpath);
	if(i != fileIndex.end()) {
		FileInfoIter j = find_if(i->second.begin(), i->second.end(), FileInfo::StringComp(fname));
		if(j != i->second.end()) {
			j->setUsed(true);
			return &(j->getRoot());
		}
	}
	return NULL;
}

void HashManager::HashStore::rebuild() {
	string tmpName = dataFile + ".tmp";
	try {
		File::renameFile(dataFile, tmpName);
		File src(tmpName, File::READ, File::OPEN);
		createDataFile(dataFile);
		File tgt(dataFile, File::WRITE, File::OPEN);
		tgt.setEndPos(0);

		TreeMap newIndex;
		for(DirIter i = fileIndex.begin(); i != fileIndex.end(); ++i) {
			for(FileInfoIter j = i->second.begin(); j != i->second.end(); ++j) {
				if(!j->getUsed())
					continue;

				TreeIter k = treeIndex.find(j->getRoot());
				if(k != treeIndex.end()) {
					newIndex[j->getRoot()] = k->second;
				}
			}
		}
		
		treeIndex = newIndex;

		vector<u_int8_t> buf;
		for(TreeIter i = treeIndex.begin(); i != treeIndex.end(); ++i) {
			src.setPos(i->second.getIndex());
			size_t datalen = TigerTree::calcBlocks(i->second.getSize(), i->second.getBlockSize()) * TTHValue::SIZE;
			buf.resize(datalen);
			src.read(&buf[0], datalen);
			tgt.write(&buf[0], datalen);
		}

		int64_t pos = tgt.getPos();
		tgt.setPos(0);
		tgt.write(&pos, sizeof(pos));
		// Set size to the nearest mb boundary
		tgt.setSize(((tgt.getSize() + 1024*1024 - 1) / 1024*1024) * 1024*1024);
	} catch(const FileException&) {
		try {
			File::renameFile(tmpName, dataFile);
		} catch(const FileException&) {
			// Too bad...
		}
	}

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

			f.write(SimpleXML::utf8Header);
			f.write(LITERAL("<HashStore Version=\"" HASH_FILE_VERSION_STRING "\">\r\n"));

			f.write(LITERAL("\t<Trees>\r\n"));

			for(TreeIter i = treeIndex.begin(); i != treeIndex.end(); ++i) {
				const TreeInfo& ti = i->second;
				f.write(LITERAL("\t\t<Hash Type=\"TTH\" Index=\""));
				f.write(Util::toString(ti.getIndex()));
				f.write(LITERAL("\" BlockSize=\""));
				f.write(Util::toString(ti.getBlockSize()));
				f.write(LITERAL("\" Size=\""));
				f.write(Util::toString(ti.getSize()));
				f.write(LITERAL("\" Root=\""));
				b32tmp.clear();
				f.write(i->first.toBase32(b32tmp));
				f.write(LITERAL("\"/>\r\n"));
			}

			f.write(LITERAL("\t</Trees>\r\n\t<Files>\r\n"));

			for(DirIter i = fileIndex.begin(); i != fileIndex.end(); ++i) {
				const string& dir = i->first;
				for(FileInfoIter j = i->second.begin(); j != i->second.end(); ++j) {
					const FileInfo& fi = *j;
					f.write(LITERAL("\t\t<File Name=\""));
					f.write(CHECKESCAPE(dir + fi.getFileName()));
					f.write(LITERAL("\" TimeStamp=\""));
					f.write(Util::toString(fi.getTimeStamp()));
					f.write(LITERAL("\" Root=\""));
					b32tmp.clear();
					f.write(fi.getRoot().toBase32(b32tmp));
					f.write(LITERAL("\"/>\r\n"));
				}
			}
			f.write(LITERAL("\t</Files>\r\n</HashStore>"));
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
	HashLoader(HashManager::HashStore& s) : store(s), size(0), timeStamp(0), version(HASH_FILE_VERSION), inTrees(false), inFiles(false), inHashStore(false) { };
	virtual void startTag(const string& name, StringPairList& attribs, bool simple);
	virtual void endTag(const string& name, const string& data);
	
private:
	HashManager::HashStore& store;

	string file;
	int64_t size;
	u_int32_t timeStamp;
	int version;

	bool inTrees;
	bool inFiles;
	bool inHashStore;
};

void HashManager::HashStore::load() {
	try {
		HashLoader l(*this);
		SimpleXMLReader(&l).fromXML(File(indexFile, File::READ, File::OPEN).read());
	} catch(const Exception&) {
		// ...
	}
}

static const string sHashStore = "HashStore";
static const string sversion = "version";		// Oops, v1 was like this
static const string sVersion = "Version";
static const string sTrees = "Trees";
static const string sFiles = "Files";
static const string sFile = "File";
static const string sName = "Name";
static const string sSize = "Size";
static const string sHash = "Hash";
static const string sType = "Type";
static const string sTTH = "TTH";
static const string sIndex = "Index";
static const string sLeafSize = "LeafSize";		// Residue from v1 as well
static const string sBlockSize = "BlockSize";
static const string sTimeStamp = "TimeStamp";
static const string sRoot = "Root";

void HashLoader::startTag(const string& name, StringPairList& attribs, bool simple) {
	if(!inHashStore && name == sHashStore) {
		version = Util::toInt(getAttrib(attribs, sVersion, 0));
		if(version == 0) {
			version = Util::toInt(getAttrib(attribs, sversion, 0));
		}
		inHashStore = !simple;
	} else if(version == 1) {
		if(name == sFile && !simple) {
			file = getAttrib(attribs, sName, 0);
			size = Util::toInt64(getAttrib(attribs, sSize, 1));
			timeStamp = Util::toUInt32(getAttrib(attribs, sTimeStamp, 2));
		} else if(name == sHash) {
			const string& type = getAttrib(attribs, sType, 0);
			int64_t blockSize = Util::toInt64(getAttrib(attribs, sLeafSize, 1));
			int64_t index = Util::toInt64(getAttrib(attribs, sIndex, 2));
			const string& root = getAttrib(attribs, sRoot, 3);
			if(!file.empty() && (type == sTTH) && (blockSize >= 1024) && (index >= 8) && !root.empty()) {
				string fname = Text::toLower(Util::getFileName(file));
				string fpath = Text::toLower(Util::getFilePath(file));
				
				store.fileIndex[fpath].push_back(HashManager::HashStore::FileInfo(fname, TTHValue(root), timeStamp, false));
				store.treeIndex[TTHValue(root)] = HashManager::HashStore::TreeInfo(size, index, blockSize);
			}
		}
	} else if(version == 2) {
		if(inTrees && name == sHash) {
			const string& type = getAttrib(attribs, sType, 0);
			int64_t index = Util::toInt64(getAttrib(attribs, sIndex, 1));
			int64_t blockSize = Util::toInt64(getAttrib(attribs, sBlockSize, 2));
			int64_t size = Util::toInt64(getAttrib(attribs, sSize, 3));
			const string& root = getAttrib(attribs, sRoot, 4);
			if(!root.empty() && type == sTTH && (index >= 8 || index == HashManager::SMALL_TREE) && blockSize >= 1024) {
				store.treeIndex[TTHValue(root)] = HashManager::HashStore::TreeInfo(size, index, blockSize);
			}
		} else if(inFiles && name == sFile) {
			file = getAttrib(attribs, sName, 0);
			timeStamp = Util::toUInt32(getAttrib(attribs, sTimeStamp, 1));
			const string& root = getAttrib(attribs, sRoot, 2);

			if(!file.empty() && size >= 0 && timeStamp > 0 && !root.empty()) {
				string fname = Text::toLower(Util::getFileName(file));
				string fpath = Text::toLower(Util::getFilePath(file));

				store.fileIndex[fpath].push_back(HashManager::HashStore::FileInfo(fname, TTHValue(root), timeStamp, false));
			}
		} else if(name == sTrees) {
			inTrees = !simple;
		} else if(name == sFiles) {
			inFiles = !simple;
		}
	}
}

void HashLoader::endTag(const string& name, const string&) {
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

#define BUF_SIZE (256*1024)

#ifdef _WIN32
bool HashManager::Hasher::fastHash(const string& fname, u_int8_t* buf, TigerTree& tth, int64_t size) {
	HANDLE h = INVALID_HANDLE_VALUE;
	DWORD x, y;
	if(!GetDiskFreeSpace(Text::toT(Util::getFilePath(fname)).c_str(), &y, &x, &y, &y)) {
		return false;
	} else {
		if((BUF_SIZE % x) != 0) {
			return false;
		} else {
			h = ::CreateFile(Text::toT(fname).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
			if(h == INVALID_HANDLE_VALUE)
				return false;
		}
	}
	DWORD hn = 0;
	DWORD rn = 0;
	u_int8_t* hbuf = buf + BUF_SIZE;
	u_int8_t* rbuf = buf;

	OVERLAPPED over = { 0 };
	over.hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	
	bool ok = false;

	u_int32_t lastRead = GET_TICK();
	if(!::ReadFile(h, hbuf, BUF_SIZE, &hn, &over)) {
		if(GetLastError() == ERROR_HANDLE_EOF) {
			hn = 0;
		} else if(GetLastError() == ERROR_IO_PENDING) {
			if(!GetOverlappedResult(h, &over, &hn, TRUE)) {
				if(GetLastError() == ERROR_HANDLE_EOF) {
					hn = 0;
				} else {
					goto cleanup;
				}
			}
		} else {
			goto cleanup;
		}
	}

	over.Offset = hn;
	size -= hn;
	BOOL res = TRUE;
	for(;;) {
		if(size > 0) {
			// Start a new overlapped read
			ResetEvent(over.hEvent);
			if(SETTING(MAX_HASH_SPEED) > 0) {
				u_int32_t now = GET_TICK();
				u_int32_t minTime = hn * 1000LL / (SETTING(MAX_HASH_SPEED) * 1024LL * 1024LL);
				if(lastRead + minTime > now) {
					u_int32_t diff = now - lastRead;
					Thread::sleep(minTime - diff);
				} 
				lastRead = lastRead + minTime;
			} else {
				lastRead = GET_TICK();
			}
			res = ReadFile(h, rbuf, BUF_SIZE, &rn, &over);
		} else {
			rn = 0;
		}

		tth.update(hbuf, hn);
		total -= hn;

		if(size == 0) {
			ok = true;
			break;
		}

		if (!res) { 
			// deal with the error code 
			switch (GetLastError()) { 
			case ERROR_IO_PENDING: 
				if(!GetOverlappedResult(h, &over, &rn, TRUE)) {
					dcdebug("Error 0x%x: %s\n", GetLastError(), Util::translateError(GetLastError()).c_str());
					goto cleanup;
				}
				break;
			default:
				dcdebug("Error 0x%x: %s\n", GetLastError(), Util::translateError(GetLastError()).c_str());
				goto cleanup;
			}
		}

		*((u_int64_t*)&over.Offset) += rn;
		size -= rn;

		swap(rbuf, hbuf);
		swap(rn, hn);
	}

cleanup:
	::CloseHandle(over.hEvent);
	::CloseHandle(h);
	return ok;
}
#endif

int HashManager::Hasher::run() {
	setThreadPriority(Thread::IDLE);

	u_int8_t* buf = NULL;
	bool virtualBuf = true;

	string fname;
	bool last = false;
	for(;;) {
		s.wait();
		if(stop)
			break;
		if(rebuild) {
			HashManager::getInstance()->doRebuild();
			rebuild = false;
			LogManager::getInstance()->message(STRING(HASH_REBUILT));
			continue;
		}
		{
			Lock l(cs);
			if(!w.empty()) {
				file = fname = w.begin()->first;
				w.erase(w.begin());
				last = w.empty();
			} else {
				last = true;
				fname.clear();
			}
		}
		running = true;

		if(!fname.empty()) {
			int64_t size = File::getSize(fname);
			int64_t sizeLeft = size;
#ifdef _WIN32
			if(buf == NULL) {
				virtualBuf = true;
				buf = (u_int8_t*)VirtualAlloc(NULL, 2*BUF_SIZE, MEM_COMMIT, PAGE_READWRITE);
			}
#endif
			if(buf == NULL) {
				virtualBuf = false;
				buf = new u_int8_t[BUF_SIZE];
			}
			try {
				File f(fname, File::READ, File::OPEN);
				int64_t bs = max(TigerTree::calcBlockSize(f.getSize(), 10), MIN_BLOCK_SIZE);
#ifdef _WIN32
				u_int32_t start = GET_TICK();
#endif
				u_int32_t timestamp = f.getLastModified();
				TigerTree slowTTH(bs);
				TigerTree* tth = &slowTTH;
				size_t n = 0;
#ifdef _WIN32
				TigerTree fastTTH(bs);
				tth = &fastTTH;
				if(!virtualBuf || !fastHash(fname, buf, fastTTH, size)) {
					tth = &slowTTH;
#endif
					u_int32_t lastRead = GET_TICK();

					do {
						size_t bufSize = BUF_SIZE;
						if(SETTING(MAX_HASH_SPEED) > 0) {
							u_int32_t now = GET_TICK();
							u_int32_t minTime = n * 1000LL / (SETTING(MAX_HASH_SPEED) * 1024LL * 1024LL);
							if(lastRead + minTime > now) {
								Thread::sleep(minTime - (now - lastRead));
							}
						}
						n = f.read(buf, bufSize);
						tth->update(buf, n);

						total -= n;
						sizeLeft -= n;
					} while (n > 0 && !stop);
#ifdef _WIN32
				} else {
					sizeLeft = 0;
				}
#endif
				f.close();
				tth->finalize();
#ifdef _WIN32
				u_int32_t end = GET_TICK();
				int64_t speed = 0;
				if(end > start) {
					speed = size * 1000LL / (end - start);
				}
#else
				int64_t speed = 0;
#endif				
				HashManager::getInstance()->hashDone(fname, timestamp, *tth, speed);
			} catch(const FileException&) {
				// Ignore, it'll be readded on the next share refresh...
			}

			total -= sizeLeft;
		}
		running = false;
		if(buf != NULL && (last || stop)) {
			if(virtualBuf) {
#ifdef _WIN32
				VirtualFree(buf, 0, MEM_RELEASE);
#endif
			} else {
				delete buf;
			}
			buf = NULL;
		}
	}
	return 0;
}

/**
 * @file
 * $Id: HashManager.cpp,v 1.47 2005/04/24 08:13:11 arnetheduck Exp $
 */
