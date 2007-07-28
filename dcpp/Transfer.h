#ifndef DCPLUSPLUS_CLIENT_TRANSFER_H_
#define DCPLUSPLUS_CLIENT_TRANSFER_H_

#include "forward.h"
#include "MerkleTree.h"
#include "TimerManager.h"
#include "Util.h"

namespace dcpp {

class Transfer {
public:
	static const string TYPE_FILE;		///< File transfer
	static const string TYPE_LIST;		///< Partial file list
	static const string TYPE_TTHL;		///< TTH Leaves

	static const string USER_LIST_NAME;
	static const string USER_LIST_NAME_BZ;

	Transfer(UserConnection& conn);
	virtual ~Transfer() { };

	int64_t getPos() const { return pos; }
	void setPos(int64_t aPos) { pos = aPos; }

	void resetPos() { pos = getStartPos(); }
	void setStartPos(int64_t aPos) { startPos = aPos; pos = aPos; }
	int64_t getStartPos() const { return startPos; }

	void addPos(int64_t aBytes, int64_t aActual) { pos += aBytes; actual+= aActual; }

	enum { AVG_PERIOD = 30000 };
	void updateRunningAverage();

	int64_t getTotal() const { return getPos() - getStartPos(); }
	int64_t getActual() const { return actual; }

	int64_t getSize() const { return size; }
	void setSize(int64_t aSize) { size = aSize; }

	int64_t getAverageSpeed() const {
		int64_t diff = (int64_t)(GET_TICK() - getStart());
		return (diff > 0) ? (getTotal() * (int64_t)1000 / diff) : 0;
	}

	int64_t getSecondsLeft() {
		updateRunningAverage();
		int64_t avg = getRunningAverage();
		return (avg > 0) ? ((getSize() - getPos()) / avg) : 0;
	}

	int64_t getBytesLeft() const {
		return getSize() - getPos();
	}

	virtual void getParams(const UserConnection& aSource, StringMap& params);

	UserPtr getUser();

	UserConnection& getUserConnection() { return userConnection; }
	const UserConnection& getUserConnection() const { return userConnection; }

	GETSET(uint64_t, start, Start);
	GETSET(uint64_t, lastTick, LastTick);
	GETSET(int64_t, runningAverage, RunningAverage);
	GETSET(TTHValue, tth, TTH);
private:
	Transfer(const Transfer&);
	Transfer& operator=(const Transfer&);

	/** Bytes on last avg update */
	int64_t last;
	/** Total actual bytes transfered this session (compression?) */
	int64_t actual;
	/** Write position in file */
	int64_t pos;
	/** Starting position */
	int64_t startPos;
	/** Target size of this transfer */
	int64_t size;

	UserConnection& userConnection;
};

} // namespace dcpp

#endif /*TRANSFER_H_*/
