#pragma once

#include "Singleton.h"
#include "MerkleTree.h"
#include "TigerHash.h"

class HashManager : public Singleton<HashManager> {
public:
	HashManager(void);
	virtual ~HashManager(void);

private:
	typedef MerkleTree<TigerHash> TigerTree;
	typedef TigerTree::HashValue TTH;


};
