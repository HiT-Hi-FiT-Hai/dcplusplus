/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

#include "BufferedSocket.h"
#include "User.h"
#include "Util.h"
#include "TimerManager.h"
#include "CriticalSection.h"

class Client;

class ClientListener  
{
public:
	typedef ClientListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum Types {
		
		BAD_PASSWORD,
		CONNECT_TO_ME,
		CONNECTED,
		CONNECTING,
		FAILED,
		FORCE_MOVE,
		GET_PASSWORD,
		HELLO,
		HUB_NAME,
		HUB_FULL,
		LOCK,
		LOGGED_IN,
		MESSAGE,
		MY_INFO,
		NICK_LIST,
		OP_LIST,
		PRIVATE_MESSAGE,
		REV_CONNECT_TO_ME,
		SEARCH,
		QUIT,
		UNKNOWN,
		VALIDATE_DENIED,
		SEARCH_FLOOD
	};
	
	virtual void onAction(Types, Client*) { };
	virtual void onAction(Types, Client*, const string&) { };
	virtual void onAction(Types, Client*, const string&, const string&) { };
	virtual void onAction(Types, Client*, const User::Ptr&) { };
	virtual void onAction(Types, Client*, const User::List&) { };
	virtual void onAction(Types, Client*, const User::Ptr&, const string&) { };
	virtual void onAction(Types, Client*, const string&, int, const string&, int, const string&) { };
	
};

class Client : public Speaker<ClientListener>, private BufferedSocketListener, private TimerManagerListener
{
	friend class ClientManager;
public:
	typedef Client* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;

	bool isConnected() { return socket.isConnected(); };

	void disconnect(bool rl = true) throw();

	void validateNick(const string& aNick) { send("$ValidateNick " + aNick + "|"); }
	void key(const string& aKey) { send("$Key " + aKey + "|"); };	
	void version(const string& aVersion) { send("$Version " + aVersion + "|"); };
	void getNickList() { send("$GetNickList|"); };
	void password(const string& aPass) { send("$MyPass " + aPass + "|"); };
	void getInfo(User::Ptr aUser) { send("$GetINFO " + aUser->getNick() + " " + getNick() + "|"); };
	void getInfo(User* aUser) { send("$GetINFO " + aUser->getNick() + " " + getNick() + "|"); };
//	void getInfo(const string& aNick) { send("$GetINFO " + aNick + " " +getNick() + "|"); };
	void sendMessage(const string& aMessage) { 	send("<" + getNick() + "> " + Util::validateMessage(aMessage) + "|"); }

	void search(int aSizeType, LONGLONG aSize, int aFileType, const string& aString);
	void searchResults(const string& aResults) { send(aResults); }
	
	
	void myInfo(const string& aNick, const string& aDescription, const string& aSpeed, const string& aEmail, const string& aBytesShared) {
		dcdebug("MyInfo %s...\n", aNick.c_str());
		lastHubs = hubs;
		lastUpdate = TimerManager::getTick();
		send("$MyINFO $ALL " + Util::validateNick(aNick) + " " + Util::validateMessage(aDescription) + 
			"<++ V:" VERSIONSTRING ",M:" + ((SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) ? string("A") : string("P")) + 
			",H:" + Util::toString(lastHubs) + ",S:" + Util::toString(SETTING(SLOTS)) + 
			">$ $" + aSpeed + "\x01$" + Util::validateMessage(aEmail) + '$' + aBytesShared + "$|");
	}

	void connectToMe(const User::Ptr& aUser) {
		dcdebug("Client::connectToMe %s\n", aUser->getNick().c_str());
		send("$ConnectToMe " + aUser->getNick() + " " + SETTING(SERVER) + ":" + Util::toString(SETTING(PORT)) + "|");
	}
	void connectToMe(User* aUser) {
		dcdebug("Client::connectToMe %s\n", aUser->getNick().c_str());
		send("$ConnectToMe " + aUser->getNick() + " " + SETTING(SERVER) + ":" + Util::toString(SETTING(PORT)) + "|");
	}
	void privateMessage(const User::Ptr& aUser, const string& aMessage) {
		send("$To: " + aUser->getNick() + " From: " + getNick() + " $" + Util::validateMessage(aMessage) + "|");
	}
	void privateMessage(User* aUser, const string& aMessage) {
		send("$To: " + aUser->getNick() + " From: " + getNick() + " $" + Util::validateMessage(aMessage) + "|");
	}
	void revConnectToMe(const User::Ptr& aUser) {
		dcdebug("Client::revConnectToMe %s\n", aUser->getNick().c_str());
		send("$RevConnectToMe " + getNick() + " " + aUser->getNick()  + "|");
	}
	void revConnectToMe(User* aUser) {
		dcdebug("Client::revConnectToMe %s\n", aUser->getNick().c_str());
		send("$RevConnectToMe " + getNick() + " " + aUser->getNick()  + "|");
	}
	
	void kick(const User::Ptr& aUser, const string& aMsg) {
		dcdebug("Client::kick\n");
		static const char str[] = 
			"$To: %s From: %s $<%s> You are being kicked because: %s|<%s> %s is kicking %s because: %s|";
		string msg2 = Util::validateMessage(aMsg);

		char* tmp = new char[sizeof(str) + 2*aUser->getNick().length() + 2*msg2.length() + 4*getNick().length()];
		const char* u = aUser->getNick().c_str();
		const char* n = getNick().c_str();
		const char* m = msg2.c_str();
		sprintf(tmp, str, u, n, n, m, n, n, u, m);
		send(tmp);
		delete[] tmp;

/*		send("$To: " + aUser->getNick() + " $From: " + getNick() + " <" + getNick() + 
			"> You are being kicked because: " + Util::removeInvalid(aMsg) + 
			"|<" + getNick() + "> " + getNick() + "  is kicking " + aUser->getNick() + 
			" because: " + Util::removeInvalid(aMsg) + "|");
*/
		// Short, short break to allow the message to reach the client...
		Sleep(100);
		send("$Kick " + aUser->getNick() + "|");
		
	}

	void kick(User* aUser, const string& aMsg) {
		dcdebug("Client::kick\n");
		
		static const char str[] = 
			"$To: %s From: %s $<%s> You are being kicked because: %s|<%s> %s is kicking %s because: %s|";
		string msg2 = Util::validateMessage(aMsg);

		char* tmp = new char[sizeof(str) + 2*aUser->getNick().length() + 2*msg2.length() + 4*getNick().length()];
		const char* u = aUser->getNick().c_str();
		const char* n = getNick().c_str();
		const char* m = msg2.c_str();
		sprintf(tmp, str, u, n, n, m, n, n, u, m);
		send(tmp);
		delete[] tmp;

/*		send("$To: " + aUser->getNick() + " $From: " + getNick() + " <" + getNick() + 
			"> You are being kicked because: " + Util::removeInvalid(aMsg) + 
			"|<" + getNick() + "> " + getNick() + "  is kicking " + aUser->getNick() + 
			" because: " + Util::removeInvalid(aMsg) + "|");
*/
		// Short, short break to allow the message to reach the client...
		Sleep(100);
		send("$Kick " + aUser->getNick() + "|");
	}
	
	void opForceMove(const User::Ptr& aUser, const string& aServer, const string& aMsg) {
		dcdebug("Client::opForceMove\n");
		send("$OpForceMove $Who:" + aUser->getNick() + "$Where:" + aServer + "$Msg:" + aMsg + "|");
	}
	void connect(const string& aServer);
	void connect(const string& aServer, short aPort);
	
	void updated(User::Ptr& aUser) {
		fire(ClientListener::MY_INFO, this, aUser);
	}

	const string& getName() { return name; };
	const string& getServer() { return server; };

	int getUserCount() throw() {
		Lock l(cs);
		return users.size();
	}

	LONGLONG getAvailable() throw() {
		Lock l(cs);
		LONGLONG x = 0;
		for(User::NickIter i = users.begin(); i != users.end(); ++i) {
			x+=i->second->getBytesShared();
		}
		return x;
	}
	const string& getNick() {
		if(nick.empty()) {
			return SETTING(NICK);
		} else {
			return nick;
		}
	}
	void setNick(const string& aNick) {
		nick = aNick;
	}
	
	const string& getIp() {
		return socket.getIp();
	}
	
	GETSET(bool, op, Op);
	GETSETREF(string, defpassword, Password);
private:

	string nick;
	string server;
	short port;
	BufferedSocket socket;
	string name;
	DWORD lastActivity;

	CriticalSection cs;

	User::NickMap users;
	static long hubs;

	int lastHubs;
	bool counted;
	DWORD lastUpdate;
	
	typedef HASH_MAP<string, int> FloodMap;
	typedef FloodMap::iterator FloodIter;
	FloodMap searchFlood;
	
	DWORD lastSearchFlood;

	Client() : lastHubs(0), counted(true), lastSearchFlood(0), op(false), socket('|'), lastActivity(TimerManager::getTick()) {
		TimerManager::getInstance()->addListener(this);
		InterlockedIncrement(&hubs);

	};
	// No copying...
	Client(const Client&) { dcassert(0); };
	virtual ~Client() throw() {
		TimerManager::getInstance()->removeListener(this);
		socket.removeListener(this);
		removeListeners();
		
		if(counted)
			InterlockedDecrement(&hubs);
	};
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD aTick) {
		if(type == TimerManagerListener::SECOND) {
			if((lastActivity + 120 * 1000) < aTick) {
				// Nothing's happened for 60 seconds, check if we're connected, if not, try to connect...
				lastActivity = aTick;
				// Try to send something for the fun of it...
				if(isConnected()) {
					dcdebug("Testing writing...\n");
					socket.write("|", 1);
				} else {
					// Try to reconnect...
					connect(server, port);
				}
			}

			// Empty spam filter every 7 seconds...
			if(lastSearchFlood + 7 * 1000 < aTick) {
				Lock l(cs);
				searchFlood.clear();
				lastSearchFlood = aTick;
			}
		} 
	}

	// BufferedSocketListener
	virtual void onAction(BufferedSocketListener::Types type, const string& aLine) {
		switch(type) {
		case BufferedSocketListener::LINE:
			onLine(aLine); break;
		case BufferedSocketListener::FAILED:
			fire(ClientListener::FAILED, this, aLine);
			disconnect();
			break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(BufferedSocketListener::Types type) {
		switch(type) {
		case BufferedSocketListener::CONNECTED:
			lastActivity = TimerManager::getTick();
			fire(ClientListener::CONNECTED, this);
			break;
		}
	}
	void onLine(const string& aLine) throw();

	void send(const string& a) throw() {
		lastActivity = TimerManager::getTick();
		//dcdebug("Sending %d to %s: %.40s\n", a.size(), getName().c_str(), a.c_str());
		socket.write(a);
	}
};


#endif // !defined(AFX_Client_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)

/**
 * @file Client.h
 * $Id: Client.h,v 1.47 2002/04/07 16:08:14 arnetheduck Exp $
 * @if LOG
 * $Log: Client.h,v $
 * Revision 1.47  2002/04/07 16:08:14  arnetheduck
 * Fixes and additions
 *
 * Revision 1.46  2002/03/19 00:41:37  arnetheduck
 * 0.162, hub counting and cpu bug
 *
 * Revision 1.45  2002/03/14 16:17:35  arnetheduck
 * Oops, file buffering bug
 *
 * Revision 1.44  2002/03/13 23:06:07  arnetheduck
 * New info sent in the description part of myinfo...
 *
 * Revision 1.43  2002/03/13 20:35:25  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.42  2002/03/07 19:07:51  arnetheduck
 * Minor fixes + started code review
 *
 * Revision 1.41  2002/03/04 23:52:30  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.40  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.39  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.38  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.37  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.36  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.35  2002/02/03 01:06:56  arnetheduck
 * More bugfixes and some minor changes
 *
 * Revision 1.34  2002/02/02 17:21:27  arnetheduck
 * Fixed search bugs and some other things...
 *
 * Revision 1.33  2002/01/26 16:34:00  arnetheduck
 * Colors dialog added, as well as some other options
 *
 * Revision 1.32  2002/01/26 12:06:39  arnetheduck
 * Småsaker
 *
 * Revision 1.31  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.30  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.29  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.28  2002/01/16 20:56:26  arnetheduck
 * Bug fixes, file listing sort and some other small changes
 *
 * Revision 1.27  2002/01/15 00:41:54  arnetheduck
 * late night fixes...
 *
 * Revision 1.26  2002/01/13 22:50:47  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.25  2002/01/11 16:13:33  arnetheduck
 * Fixed some locks and bugs, added type field to the search frame
 *
 * Revision 1.24  2002/01/11 14:52:56  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.23  2002/01/08 00:24:10  arnetheduck
 * Last bugs fixed before 0.11
 *
 * Revision 1.22  2002/01/06 00:14:54  arnetheduck
 * Incoming searches almost done, just need some testing...
 *
 * Revision 1.21  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.19  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.18  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.17  2001/12/30 15:03:44  arnetheduck
 * Added framework to handle incoming searches
 *
 * Revision 1.16  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.15  2001/12/27 18:14:36  arnetheduck
 * Version 0.08, here we go...
 *
 * Revision 1.14  2001/12/21 23:52:30  arnetheduck
 * Last commit for five days
 *
 * Revision 1.13  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.12  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.11  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.10  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.9  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
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

