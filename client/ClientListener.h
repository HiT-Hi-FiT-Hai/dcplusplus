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

#if !defined(AFX_CLIENTLISTENER_H__607F5375_97B0_47CD_B53B_D230ABF23E7E__INCLUDED_)
#define AFX_CLIENTLISTENER_H__607F5375_97B0_47CD_B53B_D230ABF23E7E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "User.h"

class Client;

class ClientListener  
{
public:
	typedef ClientListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	/**
	 * Connecting to a hub.
	 */
	virtual void onClientConnecting(Client* aClient) { };
	/**
	 * Hubname received.
	 * @param aHubName The hubname...
	 */
	virtual void onClientHubName(Client* aClient) { };
	/**
	 * Lock message received, suggested action: key(makeKey(aLock)); validateNick(aNick);
	 * @param aLock Lock string
	 * @param aPk PK part of the lock
	 */
	virtual void onClientLock(Client* aClient, const string& aLock, const string& aPk) { };
	/**
	 * Suggestion to move to aHub received.
	 * @param aHub Suggested hub.
	 */
	virtual void onClientForceMove(Client* aClient, const string& aHub) { };
	/**
	 * Connection denied, hub is full.
	 */
	virtual void onClientHubFull(Client* aClient) { };
	/**
	 * Nick validation denied
	 */
	virtual void onClientValidateDenied(Client* aClient) { };
	/**
	 * New user arrived. If this is the user's nick, send myinfo.
	 * @param aUser User that connected (Only the nick is valid so far...)
	 */
	virtual void onClientHello(Client* aClient, User::Ptr& aUser) { };
	/**
	 * Another user disconnected.
	 * @param aUser User disconnected.
	 */
	virtual void onClientQuit(Client* aClient, User::Ptr& aUser) { };
	/**
	 * Password requested.
	 */
	virtual void onClientGetPassword(Client* aClient) { };
	/**
	 * The user supplied a bad password.
	 */
	virtual void onClientBadPassword(Client* aClient) { };
	/**
	 * Successfully logged in (after sending password).
	 */
	virtual void onClientLoggedIn(Client* aClient) { };
	/**
	 * Detailed information about a user received/updated.
	 * @param aUser Full info about the user.
	 */
	virtual void onClientMyInfo(Client* aClient, User::Ptr& aUser) { };
	virtual void onClientMessage(Client* aClient, const string& aMessage) { };
	virtual void onClientUnknown(Client* aClient, const string& aCommand) { };
	virtual void onClientNickList(Client* aClient, StringList& aNicks) { };
	virtual void onClientConnected(Client* aClient) { };
	virtual void onClientError(Client* aClient, const string& aReason) { };
	virtual void onClientOpList(Client* aClient, StringList& aOps) { };
	virtual void onClientPrivateMessage(Client* aClient, const string& aFrom, const string& aMessage) { };
	virtual void onClientPrivateMessage(Client* aClient, User::Ptr& aFrom, const string& aMessage) { };
	virtual void onClientSearch(Client* aClient, const string& aSeeker, int aSearchType, const string& aSize, 
		int aFileType, const string& aString) { };
	virtual void onClientConnectToMe(Client* aClient, const string& aServer, const string& aPort) { };
	virtual void onClientRevConnectToMe(Client* aClient, User::Ptr& aUser) { };
};

#endif // !defined(AFX_CLIENTLISTENER_H__607F5375_97B0_47CD_B53B_D230ABF23E7E__INCLUDED_)

/**
 * @file ClientListener.h
 * $Id: ClientListener.h,v 1.11 2002/01/08 00:24:10 arnetheduck Exp $
 * @if LOG
 * $Log: ClientListener.h,v $
 * Revision 1.11  2002/01/08 00:24:10  arnetheduck
 * Last bugs fixed before 0.11
 *
 * Revision 1.10  2001/12/30 15:03:44  arnetheduck
 * Added framework to handle incoming searches
 *
 * Revision 1.9  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.8  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.7  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.6  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.5  2001/12/07 20:03:04  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.4  2001/11/29 19:10:54  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.3  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.2  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */

