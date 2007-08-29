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

#include "Transfer.h"

#include "UserConnection.h"
#include "ResourceManager.h"
#include "ClientManager.h"

namespace dcpp {

const string Transfer::TYPE_FILE = "file";
const string Transfer::TYPE_LIST = "list";
const string Transfer::TYPE_TTHL = "tthl";

const string Transfer::USER_LIST_NAME = "files.xml";
const string Transfer::USER_LIST_NAME_BZ = "files.xml.bz2";

Transfer::Transfer(UserConnection& conn) : start(0), lastTick(GET_TICK()), runningAverage(0),
last(0), actual(0), pos(0), startPos(0), size(-1), userConnection(conn) { }

void Transfer::updateRunningAverage() {
	uint64_t tick = GET_TICK();
	// Update 4 times/sec at most
	if(tick > (lastTick + 250)) {
		uint32_t diff = tick - lastTick;
		int64_t tot = getTotal();
		if( ((tick - getStart()) < AVG_PERIOD) ) {
			runningAverage = getAverageSpeed();
		} else {
			int64_t bdiff = tot - last;
			int64_t avg = bdiff * (int64_t)1000 / diff;
			if(diff > AVG_PERIOD) {
				runningAverage = avg;
			} else {
				// Weighted average...
				runningAverage = ((avg * diff) + (runningAverage*(AVG_PERIOD-diff)))/AVG_PERIOD;
			}
		}
		last = tot;
	}
	lastTick = tick;
}

void Transfer::getParams(const UserConnection& aSource, StringMap& params) {
	params["userNI"] = Util::toString(ClientManager::getInstance()->getNicks(aSource.getUser()->getCID()));
	params["userI4"] = aSource.getRemoteIp();
	StringList hubNames = ClientManager::getInstance()->getHubNames(aSource.getUser()->getCID());
	if(hubNames.empty())
		hubNames.push_back(STRING(OFFLINE));
	params["hub"] = Util::toString(hubNames);
	StringList hubs = ClientManager::getInstance()->getHubs(aSource.getUser()->getCID());
	if(hubs.empty())
		hubs.push_back(STRING(OFFLINE));
	params["hubURL"] = Util::toString(hubs);
	params["fileSI"] = Util::toString(getSize());
	params["fileSIshort"] = Util::formatBytes(getSize());
	params["fileSIchunk"] = Util::toString(getTotal());
	params["fileSIchunkshort"] = Util::formatBytes(getTotal());
	params["fileSIactual"] = Util::toString(getActual());
	params["fileSIactualshort"] = Util::formatBytes(getActual());
	params["speed"] = Util::formatBytes(getAverageSpeed()) + "/s";
	params["time"] = Util::formatSeconds((GET_TICK() - getStart()) / 1000);
	params["fileTR"] = getTTH().toBase32();
}

UserPtr Transfer::getUser() {
	return getUserConnection().getUser();
}

} // namespace dcpp
