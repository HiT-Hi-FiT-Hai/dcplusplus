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

#if !defined(AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_)
#define AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UserConnection.h"
#include "Util.h"
#include "TimerManager.h"

class QueueItem;
class ConnectionQueueItem;

class Download : public Transfer, public Flags {
public:
	typedef Download* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	Download(ConnectionQueueItem* aCqi, bool aResume, bool aUserList) : Transfer(aCqi), rollbackBuffer(NULL), rollbackSize(0) { 
		if(aUserList)
			setFlag(Download::USER_LIST);
		if(aResume)
			setFlag(Download::RESUME);
	};
	~Download() {
		if(rollbackBuffer)
			delete rollbackBuffer;

		
	}

	enum {
		USER_LIST = 0x01,
		RESUME = 0x02,
		ROLLBACK = 0x04
	};

	BYTE* getRollbackBuffer() { return rollbackBuffer; };
	void setRollbackBuffer(int aSize) { 
		if(rollbackBuffer) 
			delete rollbackBuffer;

		if(aSize > 0) {
			rollbackBuffer = new BYTE[aSize];
		} else {
			rollbackBuffer = NULL;
		}
		rollbackSize = aSize;
	}

	int getRollbackSize() { return rollbackSize; };

	string getTargetFileName() {
		int i = getTarget().rfind('\\');
		if(i != string::npos) {
			return getTarget().substr(i + 1);
		} else {
			return getTarget();
		}
	};

	GETSETREF(string, source, Source);
	GETSETREF(string, target, Target);
private:
	BYTE* rollbackBuffer;
	int rollbackSize;

};


class DownloadManagerListener {
public:
	typedef DownloadManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	enum Types {
		/** This is the last message sent before a download is deleted. No more messages will be sent after it. */
		COMPLETE,
		/** This indicates some sort of failure with a particular download */
		FAILED,
		/** This is the first message sent before a download starts. No other messages will be sent before. */
		STARTING,
		/** Sent once a second if something has actually been downloaded. */
		TICK
	};

	virtual void onAction(Types, Download*) { };
	virtual void onAction(Types, Download*, const string&) { };
};

class DownloadManager : public Speaker<DownloadManagerListener>, private UserConnectionListener, private TimerManagerListener, public Singleton<DownloadManager>
{
public:

	void addConnection(UserConnection::Ptr conn) {
		conn->addListener(this);
		checkDownloads(conn);
	}

	void abortDownload(const string& aTarget);
private:
	
	CriticalSection cs;
	Download::List downloads;
	
	bool checkRollback(Download* aDownload, const BYTE* aBuf, int aLen) throw(FileException);
	void removeConnection(UserConnection::Ptr aConn, bool reuse = false);
	void removeDownload(Download* aDown, bool finished = false);
	
	friend class Singleton<DownloadManager>;
	DownloadManager() { 
		TimerManager::getInstance()->addListener(this);

	};
	virtual ~DownloadManager() {
		TimerManager::getInstance()->removeListener(this);
		dcassert(downloads.empty());
	};
	
	void checkDownloads(UserConnection* aConn);
	
	// UserConnectionListener
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn) {
		switch(type) {
		case UserConnectionListener::MAXED_OUT:
			onMaxedOut(conn); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line) {
		switch(type) {
		case UserConnectionListener::FILE_LENGTH:
			onFileLength(conn, line); break;
		case UserConnectionListener::FAILED:
			onFailed(conn, line); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const BYTE* data, int len) {
		switch(type) {
		case UserConnectionListener::DATA:
			onData(conn, data, len); break;
		}
	}

	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, int mode) {
		switch(type) {
		case UserConnectionListener::MODE_CHANGE:
			onModeChange(conn, mode); break;
		}
	}
	
	void onFailed(UserConnection* aSource, const string& aError);
	void onData(UserConnection* aSource, const BYTE* aData, int aLen);
	void onFileLength(UserConnection* aSource, const string& aFileLength);
	void onMaxedOut(UserConnection* aSource);
	void onModeChange(UserConnection* aSource, int aNewMode);
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD aTick) {
		switch(type) {
		case TimerManagerListener::SECOND:
			onTimerSecond(aTick); break;
		}
	}
	void onTimerSecond(DWORD aTick);

};

#endif // !defined(AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_)

/**
 * @file DownloadManger.h
 * $Id: DownloadManager.h,v 1.35 2002/03/04 23:52:31 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.h,v $
 * Revision 1.35  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.34  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.33  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.32  2002/02/04 01:10:29  arnetheduck
 * Release 0.151...a lot of things fixed
 *
 * Revision 1.31  2002/02/01 02:00:28  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.30  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
 * Revision 1.29  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.28  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.27  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.26  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.25  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.24  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.23  2002/01/07 23:05:48  arnetheduck
 * Resume rollback implemented
 *
 * Revision 1.22  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.20  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.19  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.18  2001/12/30 15:03:45  arnetheduck
 * Added framework to handle incoming searches
 *
 * Revision 1.17  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.16  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.15  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.14  2001/12/18 12:32:18  arnetheduck
 * Stability fixes
 *
 * Revision 1.13  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.12  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.11  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.10  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.9  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.8  2001/12/07 20:03:06  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.7  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
 * Revision 1.6  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.3  2001/11/29 19:10:54  arnetheduck
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
