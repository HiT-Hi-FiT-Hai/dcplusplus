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

class ClientListener  
{
public:
	typedef ClientListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	virtual void onConnecting(const string& aServer) { };
	/**
	 * Hubname received.
	 * @param aHubName The hubname...
	 */
	virtual void onHubName(const string& aHubName) { };
	/**
	 * Lock message received, suggested action: key(makeKey(aLock)); validateNick(aNick);
	 * @param aLock Lock string
	 * @param aPk PK part of the lock
	 */
	virtual void onLock(const string& aLock, const string& aPk) { };
	/**
	 * Suggestion to move to aHub received.
	 * @param aHub Suggested hub.
	 */
	virtual void onForceMove(const string& aHub) { };
	/**
	 * Connection denied, hub is full.
	 */
	virtual void onHubFull() { };
	/**
	 * Nick validation denied
	 */
	virtual void onValidateDenied() { };
	/**
	 * New user arrived. If this is the user's nick, send myinfo.
	 * @param aNick Nick of the new user.
	 */
	virtual void onHello(const string& aNick) { };
	/**
	 * Another user disconnected.
	 * @param aNick Nick of the user.
	 */
	virtual void onQuit(const string& aNick) { };
	/**
	 * Detailed information about a user.
	 */
	virtual void onMyInfo(const string& aNick, const string& aDescription, const string& aSpeed, 
		const string& aEmail, const string& aBytesShared) { };
	virtual void onMessage(const string& aMessage) { };
	virtual void onUnknown(const string& aCommand) { };
	virtual void onNickList(StringList& aNicks) { };
	virtual void onConnected() { };
	virtual void onError(const string& aReason) { };
	virtual void onOpList(StringList& aOps) { };
	virtual void onPrivateMessage(const string& aFrom, const string& aMessage) { };
	virtual void onSearch(const string& aSeeker, int aSearchType, const string& aSize, 
		int aFileType, const string& aString) { };
	virtual void onConnectToMe(const string& aServer, const string& aPort) { };
	virtual void onRevConnectToMe(const string& aNick) { };
};

#endif // !defined(AFX_CLIENTLISTENER_H__607F5375_97B0_47CD_B53B_D230ABF23E7E__INCLUDED_)

/**
 * @file ClientListener.h
 * $Id: ClientListener.h,v 1.3 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: ClientListener.h,v $
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

