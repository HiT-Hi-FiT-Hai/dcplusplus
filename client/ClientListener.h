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
	virtual void onForceMove(const string& aHub) { };
	virtual void onHubFull() { };
	virtual void onValidateDenied() { };
	virtual void onHello(const string& aNick) { };
	virtual void onQuit(const string& aNick) { };
	virtual void onMyInfo(const string& aNick, const string& aDescription, const string& aSpeed, 
		const string& aEmail, const string& aBytesShared) { };
	virtual void onMessage(const string& aMessage) { };
	virtual void onLostConnection(const string& aMessage) { };
	virtual void onUnknown(const string& aCommand) { };
	virtual void onNickList(StringList& aNicks) { };
	virtual void onConnectionFailed(const string& aReason) { };
	virtual void onOpList(StringList& aOps) { };
	virtual void onPrivateMessage(const string& aFrom, const string& aMessage) { };
};

#endif // !defined(AFX_CLIENTLISTENER_H__607F5375_97B0_47CD_B53B_D230ABF23E7E__INCLUDED_)

/**
 * @file ClientListener.h
 * $Id: ClientListener.h,v 1.1 2001/11/21 17:33:20 arnetheduck Exp $
 * @if LOG
 * $Log: ClientListener.h,v $
 * Revision 1.1  2001/11/21 17:33:20  arnetheduck
 * Initial revision
 *
 * @endif
 */

