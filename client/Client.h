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

#if !defined(AFX_Client_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)
#define AFX_Client_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ClientListener.h"
#include "BufferedSocket.h"
#include "User.h"
#include "Util.h"
#include "SearchManager.h"

class Client : public Speaker<ClientListener>, public BufferedSocketListener
{
public:
	enum {
		SEARCH_PLAIN,
		SEARCH_ATLEAST,
		SEARCH_ATMOST
	};
	typedef Client* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;

	Client() : socket('|') {
		clientList.push_back(this);
		listeners.insert(listeners.end(), staticListeners.begin(), staticListeners.end());
	};

	bool isConnected() { return socket.isConnected(); };
	static bool isConnected(const string& aServer) {
		for(Iter i = clientList.begin(); i != clientList.end(); ++i) {
			if((*i)->getServer() == aServer) {
				return true;
			}
		}
		return false;
	}
	virtual ~Client() {
		for(Iter i = clientList.begin(); i != clientList.end(); ++i) {
			if(*i == this) {
				clientList.erase(i);
				break;
			}				
		}
		socket.removeListener(this);
	};
	
	static void addStaticListener(ClientListener::Ptr aListener) {
		staticListenersCS.enter();
		staticListeners.push_back(aListener);
		staticListenersCS.leave();

		for(Iter i = clientList.begin(); i != clientList.end(); ++i) {
			(*i)->addListener(aListener);
		}
	}
	
	static void removeStaticListener(ClientListener::Ptr aListener) {
		for(Iter i = clientList.begin(); i != clientList.end(); ++i) {
			(*i)->removeListener(aListener);
		}
		staticListenersCS.enter();
		for(ClientListener::Iter j = staticListeners.begin(); j != staticListeners.end(); ++j) {
			if(*j == aListener) {
				staticListeners.erase(j);
				break;
			}
		}
		staticListenersCS.leave();
	}
	
	static void removeStaticListeners() {
		for(Iter i = clientList.begin(); i != clientList.end(); ++i) {
			for(ClientListener::Iter j = staticListeners.begin(); j != staticListeners.end(); ++j) {
				(*i)->removeListener(*j);
			}
		}
		staticListenersCS.enter();
		staticListeners.clear();
		staticListenersCS.leave();
	}

	void disconnect() {	
		User::NickMap tmp = users;
		users.clear();
		for(User::NickIter i = tmp.begin(); i != tmp.end(); ++i) {
			delete i->second;
		}
		socket.removeListener(this);
		socket.disconnect();
	}

	void validateNick(const string& aNick) {
		dcdebug("validateNick %s\n", aNick.c_str());
		send("$ValidateNick " + aNick + "|");
	}
	
	void key(const string& aKey) {
		dcdebug("key xxx\n");
		send("$Key " + aKey + "|");
	}
	
	void version(const string& aVersion) {
		dcdebug("version %s\n", aVersion.c_str());
		send("$Version " + aVersion + "|");
	}
	
	void getNickList() {
		dcdebug("getNickList\n");
		send("$GetNickList|");
	}

	void search(int aSearchType, LONGLONG aSize, int aFileType, const string& aString){
		char buf[MAX_PATH + 128];
		char c1 = (aSearchType == SearchManager::SIZE_DONTCARE) ? 'F' : 'T';
		char c2 = (aSearchType == SearchManager::SIZE_ATLEAST) ? 'F' : 'T';

		sprintf(buf, "$Search %s:%d %c?%c?%I64d?%d?%s|", Settings::getServer().c_str(), Settings::getPort(), c1, c2, aSize, aFileType+1, aString.c_str());
		send(buf);
	}

	void sendMessage(const string& aMessage) {
		dcdebug("sendMessage ...\n");
		int i;
		string tmp = aMessage;
		while( (i = tmp.find_first_of("|$")) != string::npos) {
			tmp.erase(i, 1);
		}
		send("<" + Settings::getNick() + "> " + tmp + "|");
	}
	void getInfo(User* aUser) {
//		dcdebug("GetInfo %s\n", aUser->getNick().c_str());
		send("$GetINFO " + aUser->getNick() + " " + Settings::getNick() + "|");
	}
	void getInfo(const string& aNick) {
		if(users.find(aNick) != users.end())
			send("$GetINFO " + aNick + " " + Settings::getNick() + "|");
	}
	
	void myInfo(const string& aNick, const string& aDescription, const string& aSpeed, const string& aEmail, const string& aBytesShared) {
		dcdebug("MyInfo %s...\n", aNick.c_str());
		send("$MyINFO $ALL " + aNick + " " + aDescription+ " $ $" + aSpeed + "\x05$" + aEmail + "$" + aBytesShared + "$|");
	}

	void connectToMe(User* aUser) {
		dcdebug("Client::connectToMe %s\n", aUser->getNick().c_str());
		send("$ConnectToMe " + aUser->getNick() + " " + Settings::getServer() + ":" + Settings::getPortString() + "|");
	}
	void revConnectToMe(User* aUser) {
		dcdebug("Client::revConnectToMe %s\n", aUser->getNick().c_str());
		send("$RevConnectToMe " + Settings::getNick() + " " + aUser->getNick()  + "|");
	}
	void connect(const string& aServer, short aPort = 411);

	bool userConnected(const string& aNick) {
		return !(users.find(aNick) == users.end());
	}

	const string& getName() { return name; };
	const string& getServer() { return server; };

	User* getUser(const string& aNick) {
		User::NickIter j = users.find(aNick);
		if(j != users.end()) {
			return j->second;
		} else {
			return NULL;
		}
		dcassert(0);
	}
	static User* findUser(const string& aNick) {
		for(Iter i = clientList.begin(); i != clientList.end(); ++i) {
			User::NickIter j = (*i)->users.find(aNick);
			if(j != (*i)->users.end()) {
				return j->second;
			}
		}
		return NULL;
	}
	
	int getUserCount() {
		return users.size();
	}

	LONGLONG getAvailable() {
		LONGLONG x = 0;
		for(User::NickIter i = users.begin(); i != users.end(); ++i) {
			x+=i->second->getBytesShared();
		}
		return x;
	}
	
	static int getTotalUserCount() {
		int c = 0;
		for(Iter i = clientList.begin(); i != clientList.end(); ++i) {
			c+=(*i)->getUserCount();
		}
		return c;
	}

	static List& getList() { return clientList; }
protected:
	
	/** A list of listeners that receive all client messages (from all clients) */
	static ClientListener::List staticListeners;
	static CriticalSection staticListenersCS;

	string server;
	short port;
	BufferedSocket socket;
	string name;

	User::NickMap users;

	static List clientList;

	virtual void onLine(const string& aLine);
	
	virtual void onError(const string& aReason) {
		fireError(aReason);
		disconnect();
	}

	virtual void onConnected() {
		fireConnected();
	}

	void send(const string& a) {
		socket.write(a);
	}
	
	void fireConnected() {
		dcdebug("fireConnected\n");
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientConnected(this);
		}
	}
	void fireConnecting() {
		dcdebug("fireConnecting\n");
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientConnecting(this);
		}
	}
	void fireConnectToMe(const string& aServer, const string& aPort) {
		dcdebug("fireConnectToMe %s:%s\n", aServer.c_str(), aPort.c_str());
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientConnectToMe(this, aServer, aPort);
		}
	}
	void fireError(const string& aMessage) {
		dcdebug("fireError %s\n", aMessage.c_str());
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientError(this, aMessage);
		}
	}
	void fireForceMove(const string& aServer) {
		dcdebug("fireForceMove %s\n", aServer.c_str());
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientForceMove(this, aServer);
		}
	}
	void fireHello(User* aUser) {
		//dcdebug("fireHello\n");
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientHello(this, aUser);
		}
	}
	void fireHubFull() {
		dcdebug("fireHubFull\n");
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientHubFull(this);
		}
	}
	void fireHubName() {
		dcdebug("fireHubName\n");
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientHubName(this);
		}
		listenerCS.leave();
	}
	void fireLock(const string& aLock, const string& aPk) {
		dcdebug("fireLock %s\n", aLock.c_str());
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientLock(this, aLock, aPk);
		}
	}
	void fireMessage(const string& aMessage) {
		// dcdebug("fireMessage %s\n", aMessage.c_str());
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientMessage(this, aMessage);
		}
	}
	void fireMyInfo(User* aUser) {
		//dcdebug("fireMyInfo\n");
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientMyInfo(this, aUser);
		}
	}
	void fireNickList(StringList& aList) {
		dcdebug("fireNickList ... \n");
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientNickList(this, aList);
		}
	}
	void fireOpList(StringList& aList) {
		dcdebug("fireOpList ... \n");
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientOpList(this, aList);
		}
	}
	void firePrivateMessage(const string& aFrom, const string& aMessage) {
		dcdebug("firePM %s ...\n", aFrom.c_str());
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientPrivateMessage(this, aFrom, aMessage);
		}
	}
	void fireQuit(User* aUser) {
		//dcdebug("fireQuit\n");
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientQuit(this, aUser);
		}
	}
	void fireRevConnectToMe(User* aUser) {
		dcdebug("fireRevConnectToMe %s\n", aUser->getNick().c_str());
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientRevConnectToMe(this, aUser);
		}
	}
	void fireSearch(const string& aSeeker, int aSearchType, const string& aSize, int aFileType, const string& aString) {
		//dcdebug("fireSearch\n");
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientSearch(this, aSeeker, aSearchType, aSize, aFileType, aString);
		}
	}
	void fireUnknown(const string& aString) {
		dcdebug("fireUnknown %s\n", aString.c_str());
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientUnknown(this, aString);
		}
	}
	void fireValidateDenied() {
		dcdebug("fireValidateDenied\n");
		listenerCS.enter();
		ClientListener::List tmp = listeners;
		listenerCS.leave();
		for(ClientListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onClientValidateDenied(this);
		}
	}
};


#endif // !defined(AFX_Client_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)

/**
 * @file Client.h
 * $Id: Client.h,v 1.8 2001/12/12 00:06:04 arnetheduck Exp $
 * @if LOG
 * $Log: Client.h,v $
 * Revision 1.8  2001/12/12 00:06:04  arnetheduck
 * Updated the public hub listings, fixed some minor transfer bugs, reworked the
 * sockets to use only one thread (instead of an extra thread for sending files),
 * and fixed a major bug in the client command decoding (still have to fix this
 * one for the userconnections...)
 *
 * Revision 1.7  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.6  2001/12/07 20:03:02  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.5  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.4  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.3  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.2  2001/11/29 19:10:54  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.1  2001/11/27 22:10:08  arnetheduck
 * Renamed DCClient* to Client*
 *
 * Revision 1.5  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.4  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.3  2001/11/24 10:34:02  arnetheduck
 * Updated to use BufferedSocket instead of handling threads by itself.
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

