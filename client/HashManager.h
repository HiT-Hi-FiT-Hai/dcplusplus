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

class HashManagerListener {
public:
	typedef HashManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	enum Types {
		TTH_DONE
	};

	virtual void onAction(Types, const string& /* fileName */, TTHValue* /* root */) throw() = 0;
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
	 * Retrieves TTH root or queue's file for hashing.
	 * @return TTH root if available, otherwise NULL
	 */
	TTHValue* getTTH(const string& aFileName, int64_t aSize, u_int32_t aTimeStamp);

	/**
	 * Rebuild hash data file
	 */
	void rebuild() {
		Lock l(cs);
		store.rebuild();
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
		Hasher() : stop(false) { }

		void hashFile(const string& fileName) {
			Lock l(cs);
			w.insert(fileName);
			s.signal();
		}
		virtual int run();
		void shutdown() {
			stop = true;
			s.signal();
		}

	private:
		typedef set<string, noCaseStringLess> WorkSet;
		typedef WorkSet::iterator WorkIter;

		WorkSet w;
		CriticalSection cs;
		Semaphore s;

		bool stop;

	};

	friend class Hasher;

	class HashStore {
	public:
		HashStore();
		void addFile(const string& aFileName, TigerTree& tth, bool aUsed);

		void load();
		void save();

		void rebuild();

		TTHValue* getTTH(const string& aFileName, int64_t aSize, u_int32_t aTimeStamp) {
			TTHIter i = indexTTH.find(aFileName);
			if(i != indexTTH.end()) {
				if(i->second->getSize() == aSize && i->second->getTimeStamp() == aTimeStamp) {
					i->second->setUsed(true);
					return &(i->second->getRoot());
				} else {
					delete i->second;
					indexTTH.erase(i);
					dirty = true;
				}
			}
			return NULL;
		}

		//bool getTTH(const string& aFileName, TigerTree& tth);
		bool isDirty() { return dirty; };
	private:
		class FileInfo : public FastAlloc<FileInfo> {
		public:
			FileInfo(const TTHValue& aRoot, int64_t aSize, int64_t aIndex, size_t aBlockSize, u_int32_t aTimeStamp, bool aUsed) :
			  root(aRoot), size(aSize), index(aIndex), blockSize(aBlockSize), timeStamp(aTimeStamp), used(aUsed) { }

			TTHValue& getRoot() { return root; }
			void setRoot(const TTHValue& aRoot) { root = aRoot; }
		private:
			TTHValue root;
			GETSET(int64_t, size, Size)
			GETSET(int64_t, index, Index);
			GETSET(size_t, blockSize, BlockSize);
			GETSET(u_int32_t, timeStamp, TimeStamp);
			GETSET(bool, used, Used);
		};

		typedef HASH_MAP_X(string, FileInfo*, noCaseStringHash, noCaseStringEq, noCaseStringLess) TTHMap;
		typedef TTHMap::iterator TTHIter;

		friend class HashLoader;

		TTHMap indexTTH;

		string indexFile;
		string dataFile;

		bool dirty;

		void createDataFile(const string& name);
		int64_t addLeaves(TigerTree::MerkleList& leaves);
	};

	friend class HashLoader;

	Hasher hasher;
	HashStore store;

	CriticalSection cs;

	void hashDone(const string& aFileName, TigerTree& tth);

	virtual void onAction(TimerManagerListener::Types type, u_int32_t) throw() {
		if(type == TimerManagerListener::MINUTE) {
			Lock l(cs);
			store.save();
		}
	}
};

#endif // _HASH_MANAGER

/**
 * @file
 * $Id: HashManager.h,v 1.8 2004/03/02 09:30:19 arnetheduck Exp $
 */
