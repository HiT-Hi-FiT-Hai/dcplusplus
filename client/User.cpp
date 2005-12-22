/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#include "User.h"
#include "Client.h"
#include "StringTokenizer.h"

OnlineUser::OnlineUser(const User::Ptr& ptr, Client& client_) : user(ptr), identity(ptr, client_.getHubUrl()), client(&client_) { 

}

void Identity::getParams(StringMap& sm, const string& prefix, bool compatibility) const {
	for(InfMap::const_iterator i = info.begin(); i != info.end(); ++i) {
		sm[prefix + string((char*)(&i->first), 2)] = i->second;
	}
	if(user)
		sm[prefix + "CID"] = user->getCID().toBase32();

	if(compatibility) {
		if(prefix == "my") {
			sm["mynick"] = getNick();
			sm["mycid"] = user->getCID().toBase32();
		} else {
			sm["nick"] = getNick();
			sm["cid"] = user->getCID().toBase32();
		}
	}
}

const bool Identity::supports(const string& name) const {
	string su = get("SU");
	StringTokenizer<string> st(su, ',');
	for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
		if(*i == name)
			return true;
	}
	return false;
}

/**
 * @file
 * $Id: User.cpp,v 1.49 2005/12/22 19:47:33 arnetheduck Exp $
 */
