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

#ifndef _HASH_MANAGER
#define _HASH_MANAGER

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Singleton.h"
#include "MerkleTree.h"
#include "Thread.h"
#include "CriticalSection.h"
#include "Semaphore.h"
#include "TimerManager.h"
#include "Util.h"
#include "FastAlloc.h"
#include "Text.h"

STANDARD_EXCEPTION(HashException);

class HashManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> TTHDone;

	virtual void on(TTHDone, const string& /* fileName */, const TTHValue& /* root */) throw() = 0;
};

class HashLoader;

class HashManager : public Singleton<HashManager>, public Speaker<HashManagerListener>,
	private TimerManagerListener 
{
public:
	HashManager() {
		TimerManager::getInstance()->addListener(this);
	}
	virtual ~HashManager() {
		TimerManager::getInstance()->removeListener(this);
		hasher.join();
	}

	/**
	 * Check if the TTH tree associated with the filename is current.
	 */
	bool checkTTH(const string& aFileName, int64_t aSize, u_int32_t aTimeStamp);

	void stopHashing(const string& baseDir) {
		hasher.stopHashing(baseDir);
	}

	void setPriority(Thread::Priority p) {
		hasher.setThreadPriority(p);
	}
	/**
	 * @return TTH root
	 */
	const TTHValue& getTTH(const string& aFileName, int64_t aSize) throw(HashException);

	bool getTree(const TTHValue& root, TigerTree& tt);

	void addTree(const string& aFileName, const TigerTree& tt) {
		hashDone(aFileName, tt, -1);
	}

	void getStats(string& curFile, int64_t& bytesLeft, size_t& filesLeft) {
		hasher.getStats(curFile, bytesLeft, filesLeft);
	}

	/**
	 * Rebuild hash data file
	 */
	void rebuild() {
		hasher.scheduleRebuild();
	}

	void startup() {
		hasher.start();
		store.load();
	}

	void shutdown() {
		hasher.shutdown();
		hasher.join();
		Lock l(cs);
		store.save();
	}

private:

	class Hasher : public Thread {
	public:
		enum { MIN_BLOCK_SIZE = 64*1024 };
		Hasher() : stop(false), running(false), total(0), rebuild(false) { }

		void hashFile(const string& fileName, int64_t size) {
			Lock l(cs);
			if(w.insert(make_pair(fileName, size)).second) {
				s.signal();
				total += size;
			}
		}

		void stopHashing(const string& baseDir) {
			Lock l(cs);
			for(WorkIter i = w.begin(); i != w.end(); ) {
				if(Util::strnicmp(baseDir, i->first, baseDir.length()) == 0) {
					total -= i->second;
					w.erase(i++);
				} else {
					++i;
				}
			}
		}

		virtual int run();
#ifdef _WIN32
		bool fastHash(const string& fname, u_int8_t* buf, TigerTree& tth, int64_t size);
#endif
		void getStats(string& curFile, int64_t& bytesLeft, size_t& filesLeft) {
			Lock l(cs);
			curFile = file;
			filesLeft = w.size();
			if(running)
				filesLeft++;
			// Just in case...
			if(total < 0)
				total = 0;
			bytesLeft = total;
		}
		void shutdown() {
			stop = true;
			s.signal();
		}
		void scheduleRebuild() {
			rebuild = true;
			s.signal();
		}

	private:
		// Case-sensitive (faster), it is rather unlikely that case changes, and if it does it's harmless.
		// set because it's sorted (to avoid random hash order that would create quite strange shares while hashing)
		typedef map<string, int64_t> WorkMap;	
		typedef WorkMap::iterator WorkIter;

		WorkMap w;
		CriticalSection cs;
		Semaphore s;

		bool stop;
		bool running;
		bool rebuild;
		int64_t total;
		string file;
	};

	friend class Hasher;

	class HashStore {
	public:
		HashStore();
		void addFile(const string& aFileName, const TigerTree& tth, bool aUsed);

		void load();
		void save();

		void rebuild();

		bool checkTTH(const string& aFileName, int64_t aSize, u_int32_t aTimeStamp);

		const TTHValue* getTTH(const string& aFileName);
		bool getTree(const TTHValue& root, TigerTree& tth);
		bool isDirty() { return dirty; };
	private:
		/** Root -> tree mapping info, we assume there's only one tree for each root (a collision would mean we've broken tiger...) */
		struct TreeInfo {
			TreeInfo() : size(0), index(0), blockSize(0) { }
			TreeInfo(int64_t aSize, int64_t aIndex, int64_t aBlockSize) : size(aSize), index(aIndex), blockSize(aBlockSize) { }
			TreeInfo(const TreeInfo& rhs) : size(rhs.size), index(rhs.index), blockSize(rhs.blockSize) { }
			TreeInfo& operator=(const TreeInfo& rhs) { size = rhs.size; index = rhs.index; blockSize = rhs.blockSize; return *this; }

			GETSET(int64_t, size, Size);
			GETSET(int64_t, index, Index);
			GETSET(int64_t, blockSize, BlockSize);
		};

		/** File -> root mapping info */
		struct FileInfo {
		public:
			struct StringComp {
				const string& str;
				StringComp(const string& aStr) : str(aStr) { }
				bool operator()(const FileInfo& a) { return a.getFileName() == str; }	
			private:
				StringComp& operator=(const StringComp&);
			};

			FileInfo(const string& aFileName, const TTHValue& aRoot, u_int32_t aTimeStamp, bool aUsed) :
			  fileName(aFileName), root(aRoot), timeStamp(aTimeStamp), used(aUsed) { }

			GETSET(string, fileName, FileName);
			GETSET(TTHValue, root, Root);
			GETSET(u_int32_t, timeStamp, TimeStamp);
			GETSET(bool, used, Used);
		};

		typedef vector<FileInfo> FileInfoList;
		typedef FileInfoList::iterator FileInfoIter;

		typedef HASH_MAP<string, FileInfoList> DirMap;
		typedef DirMap::iterator DirIter;

		typedef HASH_MAP_X(TTHValue, TreeInfo, TTHValue::Hash, TTHValue::Hash, TTHValue::Less) TreeMap;
		typedef TreeMap::iterator TreeIter;

		friend class HashLoader;

		DirMap fileIndex;
		TreeMap treeIndex;

		string indexFile;
		string dataFile;

		bool dirty;

		void createDataFile(const string& name);
		int64_t addLeaves(const TigerTree::MerkleList& leaves);
	};

	friend class HashLoader;

	Hasher hasher;
	HashStore store;

	CriticalSection cs;

	void hashDone(const string& aFileName, const TigerTree& tth, int64_t speed);
	void doRebuild() {
		Lock l(cs);
		store.rebuild();
	}
	virtual void on(TimerManagerListener::Minute, u_int32_t) throw() {
		Lock l(cs);
		store.save();
	}
};

#endif // _HASH_MANAGER

/**
 * @file
 * $Id: HashManager.h,v 1.25 2004/12/19 18:15:43 arnetheduck Exp $
 */
