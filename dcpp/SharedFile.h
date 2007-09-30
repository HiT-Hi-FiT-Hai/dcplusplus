#ifndef DCPLUSPLUS_DCPP_SHAREDFILE_H_
#define DCPLUSPLUS_DCPP_SHAREDFILE_H_

#include "File.h"
#include "CriticalSection.h"
#include "Segment.h"

namespace dcpp {

/** A file used by multiple writers */
class SharedFile : public IOStream {
public:
	SharedFile(const string& filename, const Segment& segment, int64_t totalSize);
	
	SharedFile(const SharedFile& sf, const Segment& segment_) : file(sf.file), segment(segment_), pos(0) { }
	
	using OutputStream::write;
	virtual size_t write(const void* buf, size_t len) throw(Exception);
	virtual size_t read(void* buf, size_t& len) throw(Exception);

	virtual size_t flush() throw(Exception);
private:
	struct FileData {
		FileData(const string& filename);
		File f;
		CriticalSection cs;
	};
	
	typedef std::tr1::shared_ptr<FileData> FileDataPtr;

	FileDataPtr file;
	
	Segment segment;
	
	/** Bytes written so far */
	size_t pos;
	
};

}
#endif /*SHAREDFILE_H_*/
