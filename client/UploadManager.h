/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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
#include "Singleton.h"

#include "ClientManagerListener.h"

class Upload : public Transfer, public Flags {
public:
	enum Flags {
		FLAG_USER_LIST = 0x01,
		FLAG_SMALL_FILE = 0x02,
		FLAG_ZUPLOAD = 0x04
	};

	typedef Upload* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	Upload() { };
	virtual ~Upload() { };
	
	User::Ptr& getUser() { dcassert(getUserConnection() != NULL); return getUserConnection()->getUser(); };
	
	GETSETREF(string, fileName, FileName);
	GETSETREF(string, localFileName, LocalFileName);
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

	virtual void onAction(Types, Upload*) throw() { };
	virtual void onAction(Types, const Upload::List&) throw() { };
	virtual void onAction(Types, Upload*, const string&) throw() { };

};

class UploadManager : private ClientManagerListener, private UserConnectionListener, public Speaker<UploadManagerListener>, private TimerManagerListener, public Singleton<UploadManager>
{
public:
	
	int getUploads() { Lock l(cs); return uploads.size(); };
	int getAverageSpeed() {
		Lock l(cs);
		int avg = 0;
		for(Upload::Iter i = uploads.begin(); i != uploads.end(); ++i) {
			Upload* u = *i;
			avg += (int)u->getRunningAverage();
		}
		return avg;
	}
	
	int getRunning() { return running; };
	int getFreeSlots() { return max((SETTING(SLOTS) - running), 0); }
	int getFreeExtraSlots() { return max(3 - getExtra(), 0); };
	
	void reserveSlot(const User::Ptr& aUser) {
		Lock l(cs);
		reservedSlots[aUser] = GET_TICK();
	}

	void addConnection(UserConnection::Ptr conn) {
		conn->addListener(this);
		conn->setState(UserConnection::STATE_GET);
	}

	GETSET(int, running, Running);
	GETSET(int, extra, Extra);
	GETSET(u_int32_t, lastAutoGrant, LastAutoGrant);
private:
	Upload::List uploads;
	CriticalSection cs;

	typedef HASH_MAP<User::Ptr, u_int32_t, User::HashFunction> SlotMap;
	typedef SlotMap::iterator SlotIter;
	SlotMap reservedSlots;

	friend class Singleton<UploadManager>;
	UploadManager() throw();
	virtual ~UploadManager() throw();

	void removeConnection(UserConnection::Ptr aConn);
	void removeUpload(Upload* aUpload) {
		Lock l(cs);
		dcassert(find(uploads.begin(), uploads.end(), aUpload) != uploads.end());
		uploads.erase(find(uploads.begin(), uploads.end(), aUpload));

		if(aUpload->getFile()) {
			delete aUpload->getFile();
			aUpload->setFile(NULL);
		}
		aUpload->setUserConnection(NULL);
		delete aUpload;
	}

	// ClientManagerListener
	virtual void onAction(ClientManagerListener::Types type, const User::Ptr& aUser) throw();
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t aTick) throw();
	void onTimerMinute(u_int32_t aTick);

	// UserConnectionListener
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn) throw();
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, u_int32_t bytes, u_int32_t actual) throw();
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line) throw();
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line, int64_t resume) throw();
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line, int64_t resume, int64_t bytes) throw();
	
	void onBytesSent(UserConnection* aSource, u_int32_t aBytes, u_int32_t aActual);
	void onFailed(UserConnection* aSource, const string& aError);
	void onTransmitDone(UserConnection* aSource);
	void onGet(UserConnection* aSource, const string& aFile, int64_t aResume);
	void onGetZBlock(UserConnection* aSource, const string& aFile, int64_t aResume, int64_t aBytes);
	void onSend(UserConnection* aSource);

	bool prepareFile(UserConnection* aSource, const string& aFile, int64_t aResume);
};

#endif // !defined(AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_)

/**
 * @file
 * $Id: UploadManager.h,v 1.57 2003/11/21 17:00:54 arnetheduck Exp $
 */
