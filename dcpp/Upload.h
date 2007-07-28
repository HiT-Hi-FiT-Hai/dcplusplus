#ifndef UPLOAD_H_
#define UPLOAD_H_

#include "forward.h"
#include "Transfer.h"
#include "Flags.h"

namespace dcpp {

class Upload : public Transfer, public Flags {
public:
	enum Flags {
		FLAG_USER_LIST = 0x01,
		FLAG_TTH_LEAVES = 0x02,
		FLAG_ZUPLOAD = 0x04,
		FLAG_PARTIAL_LIST = 0x08,
		FLAG_PENDING_KICK = 0x10
	};

	Upload(UserConnection& conn);
	virtual ~Upload();

	virtual void getParams(const UserConnection& aSource, StringMap& params);

	GETSET(string, sourceFile, SourceFile);
	GETSET(InputStream*, stream, Stream);
};

} // namespace dcpp

#endif /*UPLOAD_H_*/
