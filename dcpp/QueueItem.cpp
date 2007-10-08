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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "QueueItem.h"
#include "HashManager.h"
#include "Download.h"
#include "File.h"

namespace dcpp {

namespace {
	const string TEMP_EXTENSION = ".dctmp";

	string getTempName(const string& aFileName, const TTHValue& aRoot) {
		string tmp(aFileName);
		tmp += "." + aRoot.toBase32();
		tmp += TEMP_EXTENSION;
		return tmp;
	}
}

int QueueItem::countOnlineUsers() const {
	int n = 0;
	SourceConstIter i = sources.begin();
	for(; i != sources.end(); ++i) {
		if(i->getUser()->isOnline())
			n++;
	}
	return n;
}

void QueueItem::addSource(const UserPtr& aUser) {
	dcassert(!isSource(aUser));
	SourceIter i = getBadSource(aUser);
	if(i != badSources.end()) {
		sources.push_back(*i);
		badSources.erase(i);
	} else {
		sources.push_back(Source(aUser));
	}
}

void QueueItem::removeSource(const UserPtr& aUser, int reason) {
	SourceIter i = getSource(aUser);
	dcassert(i != sources.end());
	i->setFlag(reason);
	badSources.push_back(*i);
	sources.erase(i);
}

const string& QueueItem::getTempTarget() {
	if(!isSet(QueueItem::FLAG_USER_LIST) && tempTarget.empty()) {
		if(!SETTING(TEMP_DOWNLOAD_DIRECTORY).empty() && (File::getSize(getTarget()) == -1)) {
#ifdef _WIN32
			dcpp::StringMap sm;
			if(target.length() >= 3 && target[1] == ':' && target[2] == '\\')
				sm["targetdrive"] = target.substr(0, 3);
			else
				sm["targetdrive"] = Util::getConfigPath().substr(0, 3);
			setTempTarget(Util::formatParams(SETTING(TEMP_DOWNLOAD_DIRECTORY), sm, false) + getTempName(getTargetFileName(), getTTH()));
#else //_WIN32
			setTempTarget(SETTING(TEMP_DOWNLOAD_DIRECTORY) + getTempName(getTargetFileName(), getTTH()));
#endif //_WIN32
		}
	}
	return tempTarget;
}

Segment QueueItem::getNextSegment(int64_t  blockSize) const {
	if(getSize() == -1 || blockSize == 0) {
		return Segment(0, -1);
	}
	
	if(done.empty()) {
		Segment ret(0, blockSize > getSize() ? getSize() : blockSize);
		return ret;
	}
	
	for(SegmentIter i = done.begin(); i != done.end(); ++i) {
		int64_t end = i->getStart() + i->getSize();
		int64_t start = end - (end % blockSize);
		Segment block(start, start + blockSize > getSize() ? getSize() - start : blockSize);

		bool overlaps = false;
		for(DownloadList::const_iterator j = downloads.begin(); j != downloads.end(); ++j) {
			if(block.overlaps((*j)->getSegment())) {
				overlaps = true;
				break;
			}
		}
		if(!overlaps) {
			return block;
		}
	}
	
	return Segment(0, 0);
}

int64_t QueueItem::getDownloadedBytes() const {
	int64_t total = 0;
	for(SegmentSet::const_iterator i = done.begin(); i != done.end(); ++i) {
		total += i->getSize();
	}
	return total;
}

void QueueItem::addSegment(const Segment& segment) {
	done.insert(segment);

	// Consilidate segments
	if(done.size() == 1)
		return;
	
	for(SegmentSet::iterator i = ++done.begin() ; i != done.end(); ) {
		SegmentSet::iterator prev = i;
		prev--;
		if(prev->getEnd() == i->getStart()) {
			Segment big(prev->getStart(), prev->getSize() + i->getSize());
			done.erase(prev);
			done.erase(i++);
			done.insert(big);
		} else {
			++i;
		}
	}
}

}
