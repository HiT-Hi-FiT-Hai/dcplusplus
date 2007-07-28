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

#include "Download.h"

#include "UserConnection.h"
#include "QueueItem.h"

namespace dcpp {

Download::Download(UserConnection& conn) throw() : Transfer(conn), file(0),
crcCalc(NULL), treeValid(false) {
	conn.setDownload(this);
}

Download::Download(UserConnection& conn, QueueItem& qi) throw() : Transfer(conn),
	target(qi.getTarget()), tempTarget(qi.getTempTarget()), file(0),
	crcCalc(NULL), treeValid(false) 
{
	conn.setDownload(this);
	
	setTTH(qi.getTTH());
	setSize(qi.getSize());
	if(qi.isSet(QueueItem::FLAG_USER_LIST))
		setFlag(Download::FLAG_USER_LIST);
	if(qi.isSet(QueueItem::FLAG_RESUME))
		setFlag(Download::FLAG_RESUME);
}
Download::~Download() {
	getUserConnection().setDownload(0);
}

AdcCommand Download::getCommand(bool zlib) {
	AdcCommand cmd(AdcCommand::CMD_GET);
	if(isSet(FLAG_TREE_DOWNLOAD)) {
		cmd.addParam(Transfer::TYPE_TTHL);
	} else if(isSet(FLAG_PARTIAL_LIST)) {
		cmd.addParam(Transfer::TYPE_LIST);
	} else {
		cmd.addParam(Transfer::TYPE_FILE);
	}
	if(isSet(FLAG_PARTIAL_LIST) || isSet(FLAG_USER_LIST)) {
		cmd.addParam(Util::toAdcFile(getSource()));
	} else {
		cmd.addParam("TTH/" + getTTH().toBase32());
	}
	cmd.addParam(Util::toString(getPos()));
	cmd.addParam(Util::toString(getSize() - getPos()));

	if(zlib && BOOLSETTING(COMPRESS_TRANSFERS)) {
		cmd.addParam("ZL1");
	}

	return cmd;
}

void Download::getParams(const UserConnection& aSource, StringMap& params) {
	Transfer::getParams(aSource, params);
	params["target"] = getTarget();
	params["sfv"] = Util::toString(isSet(Download::FLAG_CRC32_OK) ? 1 : 0);
}

} // namespace dcpp
