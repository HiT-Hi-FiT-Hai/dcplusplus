/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_TRANSFER_H_
#define DCPLUSPLUS_DCPP_TRANSFER_H_

#include "forward.h"
#include "MerkleTree.h"
#include "TimerManager.h"
#include "Util.h"
#include "CriticalSection.h"
#include "Segment.h"

namespace dcpp {

class Transfer {
public:
	enum Type {
		TYPE_FILE,
		TYPE_FULL_LIST,
		TYPE_PARTIAL_LIST,
		TYPE_TREE,
		TYPE_LAST
	};
	
	static const string names[TYPE_LAST];

	static const string USER_LIST_NAME;
	static const string USER_LIST_NAME_BZ;

	Transfer(UserConnection& conn);
	virtual ~Transfer() { };

	int64_t getPos() const { return getStartPos() + pos; }

	void resetPos() { pos = 0; }
	
	int64_t getStartPos() const { return getSegment().getStart(); }

	void addPos(int64_t aBytes, int64_t aActual) { pos += aBytes; actual+= aActual; }

	enum { SAMPLES = 15 };
	
	/** Record a sample for average calculation */
	void tick();

	int64_t getActual() const { return actual; }

	int64_t getSize() const { return getSegment().getSize(); }

	double getAverageSpeed() const;

	int64_t getSecondsLeft() {
		double avg = getAverageSpeed();
		return (avg > 0) ? (getBytesLeft() / avg) : 0;
	}

	int64_t getBytesLeft() const {
		return getSize() - getPos();
	}

	virtual void getParams(const UserConnection& aSource, StringMap& params);

	UserPtr getUser();

	UserConnection& getUserConnection() { return userConnection; }
	const UserConnection& getUserConnection() const { return userConnection; }

	GETSET(Segment, segment, Segment);
	GETSET(TTHValue, tth, TTH);
	GETSET(Type, type, Type);
	GETSET(int64_t, total, Total);
	GETSET(uint64_t, start, Start);
private:
	
	typedef std::pair<uint64_t, int64_t> Sample;
	typedef deque<Sample> SampleList;
	
	Transfer(const Transfer&);
	Transfer& operator=(const Transfer&);
	
	SampleList samples;
	mutable CriticalSection cs;
	
	/** Bytes transferred over socket */
	int64_t actual;
	/** Bytes transferred to/from file */
	int64_t pos;

	UserConnection& userConnection;
};

} // namespace dcpp

#endif /*TRANSFER_H_*/
