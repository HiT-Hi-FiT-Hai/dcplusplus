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

#if !defined(AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_)
#define AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UserConnection.h"
#include "ConnectionManager.h"
#include "ShareManager.h"
#include "Util.h"

class Upload : public Transfer, public Flags {
public:
	enum Flags {
		USER_LIST = 0x01,
		SMALL_FILE = USER_LIST << 1
	};

	typedef Upload* Ptr;
	typedef map<UserConnection::Ptr, Ptr> Map;
	typedef Map::iterator MapIter;
	
	Upload(ConnectionQueueItem* aQI) : Transfer(aQI) { };

	void setUser(User::Ptr& aUser) { user = aUser; if(user) nick = user->getNick(); };
	User::Ptr& getUser() { return user; };
	
	GETSETREF(string, fileName, FileName);
	GETSETREF(string, nick, Nick);
private:
	User::Ptr user;	

};

class UploadManagerListener {
public:
	typedef UploadManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum Types {
		COMPLETE,
		FAILED,
		STARTING,
		TICK
	};

	virtual void onAction(Types, Upload*) { };
	virtual void onAction(Types, Upload*, const string&) { };

};

class UploadManager : private UserConnectionListener, public Speaker<UploadManagerListener>, private TimerManagerListener, public Singleton<UploadManager>
{
public:
	void removeUpload(Upload* aUpload);
	void removeUpload(UserConnection* aUpload);
	
	int getUploads() { Lock l(cs); return uploads.size(); };
	int getRunning() { return running; };
	int getFreeSlots() { return max((SETTING(SLOTS) - running), 0); }
	int getFreeExtraSlots() { return max(3 - getExtra(), 0); };
	
	void reserveSlot(const User::Ptr& aUser) {
		Lock l(cs);
		reservedSlots[aUser] = TimerManager::getTick();
	}

	bool isExtra(Upload* u) {
		if(u->isSet(Upload::SMALL_FILE) || u->isSet(Upload::USER_LIST))
			return true;
		else
			return false;
	}

	void addConnection(UserConnection::Ptr conn) {
		conn->addListener(this);
		{
			Lock l(cs);
			connections.push_back(conn);
		}
	}

	void removeConnection(UserConnection::Ptr aConn) {
		{
			Lock l(cs);

			UserConnection::Iter i = find(connections.begin(), connections.end(), aConn);
			if(i == connections.end()) {
				dcdebug("UploadManager::removeConnection Unknown connection\n");
				return;
			}
			connections.erase(i);
				
			if(aConn->isSet(UserConnection::FLAG_HASSLOT)) {
				running--;
				aConn->unsetFlag(UserConnection::FLAG_HASSLOT);
			} 
			if(aConn->isSet(UserConnection::FLAG_HASEXTRASLOT)) {
				extra--;
				aConn->unsetFlag(UserConnection::FLAG_HASEXTRASLOT);
			}
				
		}

		aConn->removeListener(this);
		ConnectionManager::getInstance()->putUploadConnection(aConn);
	}

	void removeConnections() {

		UserConnection::List tmp;
		{
			Lock l(cs);
			tmp = connections;
			connections.clear();
		}

		for(UserConnection::Iter i = tmp.begin(); i != tmp.end(); ++i) {
			(*i)->removeListener(this);
			ConnectionManager::getInstance()->putUploadConnection(*i);
		}
	}
	
	GETSET(int, running, Running);
	GETSET(int, extra, Extra);
private:
	UserConnection::List connections;
	Upload::Map uploads;
	CriticalSection cs;
	map<User::Ptr, DWORD> reservedSlots;

	friend class Singleton<UploadManager>;
	UploadManager() : running(0), extra(0) { 
		TimerManager::getInstance()->addListener(this);
	};
	~UploadManager() {
		TimerManager::getInstance()->removeListener(this);
		{
			Lock l(cs);
			for(Upload::MapIter j = uploads.begin(); j != uploads.end(); ++j) {
				delete j->second;
			}
			uploads.clear();
		}

		removeConnections();
	}

	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD aTick) {
		switch(type) {
		case TimerManagerListener::SECOND: 
			{
				Lock l(cs);
				for(Upload::MapIter i = uploads.begin(); i != uploads.end(); ++i) {
					fire(UploadManagerListener::TICK, i->second);
				}
			}
			break;
		case TimerManagerListener::MINUTE:
			{
				Lock l(cs);
				for(Upload::MapIter i = uploads.begin(); i != uploads.end(); ++i) {
					UserConnection* c = i->first;
					if(!c->getUser()->isOnline()) {
						ConnectionManager::getInstance()->updateUser(c);
					}
				}
				for(map<User::Ptr, DWORD>::iterator j = reservedSlots.begin(); j != reservedSlots.end();) {
					if(j->second + 600 * 1000 < aTick) {
						reservedSlots.erase(j++);
					} else {
						++j;
					}
				}
			}	
			break;
		}
	}
	
	// UserConnectionListener
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn) {
		switch(type) {
		case UserConnectionListener::TRANSMIT_DONE:
			onTransmitDone(conn); break;
		case UserConnectionListener::SEND:
			onSend(conn); break;
		case UserConnectionListener::GET_LIST_LENGTH:
			conn->listLen(ShareManager::getInstance()->getListLenString()); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, DWORD bytes) {
		switch(type) {
		case UserConnectionListener::BYTES_SENT:
			onBytesSent(conn, bytes); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line) {
		switch(type) {
		case UserConnectionListener::FAILED:
			onFailed(conn, line); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line, LONGLONG resume) {
		switch(type) {
		case UserConnectionListener::GET:
			onGet(conn, line, resume); break;
		}
	}
	
	void onBytesSent(UserConnection* aSource, DWORD aBytes);
	void onFailed(UserConnection* aSource, const string& aError);
	void onTransmitDone(UserConnection* aSource);
	void onGet(UserConnection* aSource, const string& aFile, LONGLONG aResume);
	void onSend(UserConnection* aSource);
	
};

#endif // !defined(AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_)

/**
 * @file UploadManger.h
 * $Id: UploadManager.h,v 1.38 2002/02/18 23:48:32 arnetheduck Exp $
 * @if LOG
 * $Log: UploadManager.h,v $
 * Revision 1.38  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.37  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.36  2002/02/02 17:21:27  arnetheduck
 * Fixed search bugs and some other things...
 *
 * Revision 1.35  2002/02/01 02:00:46  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.34  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
 * Revision 1.33  2002/01/23 08:45:37  arnetheduck
 * New files for the notepad
 *
 * Revision 1.32  2002/01/22 00:10:37  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
 *
 * Revision 1.31  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.30  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.29  2002/01/16 20:56:27  arnetheduck
 * Bug fixes, file listing sort and some other small changes
 *
 * Revision 1.28  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.27  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.26  2002/01/11 16:13:33  arnetheduck
 * Fixed some locks and bugs, added type field to the search frame
 *
 * Revision 1.25  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.24  2002/01/06 00:14:54  arnetheduck
 * Incoming searches almost done, just need some testing...
 *
 * Revision 1.23  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.21  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.20  2002/01/02 16:12:33  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.19  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.18  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.17  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.16  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.15  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.14  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.13  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.12  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.11  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.10  2001/12/07 20:03:26  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.9  2001/12/05 19:40:13  arnetheduck
 * More bugfixes.
 *
 * Revision 1.8  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
 * Revision 1.7  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.6  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.3  2001/11/29 19:10:55  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */
