#ifndef DCPLUSPLUS_DCPP_DOWNLOAD_H_
#define DCPLUSPLUS_DCPP_DOWNLOAD_H_

#include "forward.h"
#include "Transfer.h"
#include "MerkleTree.h"
#include "Flags.h"
#include "Streams.h"

namespace dcpp {

/**
 * Comes as an argument in the DownloadManagerListener functions.
 * Use it to retrieve information about the ongoing transfer.
 */
class Download : public Transfer, public Flags {
public:
	static const string ANTI_FRAG_EXT;

	enum {
		FLAG_ZDOWNLOAD = 1 << 1,
		FLAG_CALC_CRC32 = 1 << 2,
		FLAG_CRC32_OK = 1 << 3,
		FLAG_ANTI_FRAG = 1 << 4,
		FLAG_TREE_TRIED = 1 << 5,
		FLAG_TTH_CHECK = 1 << 6,
		FLAG_XML_BZ_LIST = 1 << 7
	};

	Download(UserConnection& conn, const string& pfsDir) throw();
	Download(UserConnection& conn, QueueItem& qi, bool supportsTrees) throw();

	virtual void getParams(const UserConnection& aSource, StringMap& params);

	virtual ~Download();

	/** @return Target filename without path. */
	string getTargetFileName() {
		return Util::getFileName(getPath());
	}

	/** @internal */
	string getDownloadTarget() {
		const string& tgt = (getTempTarget().empty() ? getPath() : getTempTarget());
		return isSet(FLAG_ANTI_FRAG) ? tgt + ANTI_FRAG_EXT : tgt;
	}

	/** @internal */
	TigerTree& getTigerTree() { return tt; }
	string& getPFS() { return pfs; }
	/** @internal */
	AdcCommand getCommand(bool zlib);
	
	GETSET(string, tempTarget, TempTarget);
	GETSET(OutputStream*, file, File);
	GETSET(bool, treeValid, TreeValid);
private:
	Download(const Download&);
	Download& operator=(const Download&);

	TigerTree tt;
	string pfs;
};

} // namespace dcpp

#endif /*DOWNLOAD_H_*/
