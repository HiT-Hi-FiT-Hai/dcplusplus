/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "HubManager.h"

#include "HttpConnection.h"
#include "StringTokenizer.h"

HubManager::HubEntry::List HubManager::publicHubs;

HubManager::HubManager()
{

}

HubManager::~HubManager()
{

}

/**
 * Return a list of hub entries for the currently known public hubs.
 * Locks execution until list has been downloaded.
 * @param aRefresh Refresh list from neomodus server.
 */
HubManager::HubEntry::List& HubManager::getPublicHubs(boolean aRefresh/* =false */) throw(HubException) {
	if(!aRefresh && publicHubs.size()>0) {
		return publicHubs;
	}

	publicHubs.clear();

	try {
		StringTokenizer tokens(HttpConnection::DownloadTextFile("http://www.neo-modus.com/PublicHubList.config"));
		StringList t = tokens.getTokens();

		for(StringIter i = t.begin(); i!=t.end();i++) {
			StringTokenizer hub(*i, '|');
			StringIter j = hub.getTokens().begin();
			string& name = *j++;
			string& server = *j++;
			string& desc = *j++;
			string& users = *j++;
			publicHubs.push_back(HubEntry(name, server, desc, users));
		}
	} catch(Exception e) {
		throw HubException(e, "Cannot get hub list");
	}

	return publicHubs;	
}

/**
 * @file HubManager.cpp
 * $Id: HubManager.cpp,v 1.1 2001/11/21 17:33:20 arnetheduck Exp $
 * @if LOG
 * $Log: HubManager.cpp,v $
 * Revision 1.1  2001/11/21 17:33:20  arnetheduck
 * Initial revision
 *
 * @endif
 */

