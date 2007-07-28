#ifndef DCPLUSPLUS_CLIENT_DOWNLOAD_H_
#define DCPLUSPLUS_CLIENT_DOWNLOAD_H_

#include "forward.h"
#include "Transfer.h"
#include "MerkleTree.h"
#include "ZUtils.h"
#include "FilteredFile.h"
#include "Flags.h"

namespace dcpp {

/**
 * Comes as an argument in the DownloadManagerListener functions.
 * Use it to retrieve information about the ongoing transfer.
 */
class Download : public Transfer, public Flags {
public:
	static const string ANTI_FRAG_EXT;

	enum {
		FLAG_USER_LIST = 0x01,
		FLAG_RESUME = 0x02,
		FLAG_ZDOWNLOAD = 0x04,
		FLAG_CALC_CRC32 = 0x08,
		FLAG_CRC32_OK = 0x10,
		FLAG_ANTI_FRAG = 0x20,
		FLAG_TREE_DOWNLOAD = 0x40,
		FLAG_TREE_TRIED = 0x100,
		FLAG_PARTIAL_LIST = 0x200,
		FLAG_TTH_CHECK = 0x400
	};

	Download(UserConnection& conn) throw();
	Download(UserConnection& conn, QueueItem& qi) throw();

	virtual void getParams(const UserConnection& aSource, StringMap& params);

	virtual ~Download();

	/** @return Target filename without path. */
	string getTargetFileName() {
		return Util::getFileName(getTarget());
	}

	/** @internal */
	string getDownloadTarget() {
		const string& tgt = (getTempTarget().empty() ? getTarget() : getTempTarget());
		return isSet(FLAG_ANTI_FRAG) ? tgt + ANTI_FRAG_EXT : tgt;
	}

	/** @internal */
	TigerTree& getTigerTree() { return tt; }
	string& getPFS() { return pfs; }
	/** @internal */
	AdcCommand getCommand(bool zlib);

	typedef CalcOutputStream<CRC32Filter, true> CrcOS;
	GETSET(string, source, Source);
	GETSET(string, target, Target);
	GETSET(string, tempTarget, TempTarget);
	GETSET(OutputStream*, file, File);
	GETSET(CrcOS*, crcCalc, CrcCalc);
	GETSET(bool, treeValid, TreeValid);

private:
	Download(const Download&);
	Download& operator=(const Download&);

	TigerTree tt;
	string pfs;
};

} // namespace dcpp

#endif /*DOWNLOAD_H_*/
