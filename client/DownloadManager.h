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

#if !defined(AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_)
#define AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UserConnection.h"
#include "TimerManager.h"

class QueueItem;
class ConnectionQueueItem;
class ZDecompressor;

class Download : public Transfer, public Flags {
public:
	typedef Download* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	enum {
		FLAG_USER_LIST = 0x01,
		FLAG_RESUME = 0x02,
		FLAG_ROLLBACK = 0x04,
		FLAG_ZDOWNLOAD = 0x08,
		FLAG_CALC_CRC32 = 0x10,
		FLAG_CRC32_OK = 0x20,
		FLAG_ANTI_FRAG = 0x40
	};

	Download(QueueItem* qi) throw();

	virtual ~Download() {
		if(rollbackBuffer)
			delete[] rollbackBuffer;
		dcassert(comp == NULL);
	}

	u_int8_t* getRollbackBuffer() { return rollbackBuffer; };

	void setRollbackBuffer(int aSize) { 
		if(rollbackBuffer) 
			delete[] rollbackBuffer;

		rollbackBuffer = (aSize > 0) ? new u_int8_t[aSize] : NULL;
		rollbackSize = aSize;
	}

	int getRollbackSize() { return rollbackSize; };

	string getTargetFileName() {
		string::size_type i = getTarget().rfind('\\');
		if(i != string::npos) {
			return getTarget().substr(i + 1);
		} else {
			return getTarget();
		}
	};

	GETSETREF(string, source, Source);
	GETSETREF(string, target, Target);
	GETSETREF(string, tempTarget, TempTarget);
	GETSET(ZDecompressor*, comp, Comp);

	int64_t bytesLeft;
private:

	Download() { };

	u_int8_t* rollbackBuffer;
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
		/** This indicates some sort of failure with a particular download. No more messages will be sent after it */
		FAILED,
		/** This is the first message sent before a download starts. No other messages will be sent before. */
		STARTING,
		/** Sent once a second if something has actually been downloaded. */
		TICK
	};

	virtual void onAction(Types, Download*) throw() { };
	virtual void onAction(Types, Download*, const string&) throw() { };
	virtual void onAction(Types, const Download::List&) throw() { };
};

class DownloadManager : public Speaker<DownloadManagerListener>, private UserConnectionListener, private TimerManagerListener, public Singleton<DownloadManager>
{
public:

	void addConnection(UserConnection::Ptr conn) {
		conn->addListener(this);
		checkDownloads(conn);
	}

	void abortDownload(const string& aTarget);
	int getAverageSpeed() {
		Lock l(cs);
		int avg = 0;
		for(Download::Iter i = downloads.begin(); i != downloads.end(); ++i) {
			Download* d = *i;
			avg += (int)d->getRunningAverage();
		}
		return avg;
	}
	int getDownloads() {
		Lock l(cs);
		return downloads.size();
	}
private:

	enum { MOVER_LIMIT = 10*1024*1024 };
	class FileMover : public Thread {
	public:
		FileMover() : active(false) { };
		virtual ~FileMover() { join(); };

		void moveFile(const string& source, const string& target);
		virtual int run();
	private:
		typedef pair<string, string> FilePair;
		typedef vector<FilePair> FileList;
		typedef FileList::iterator FileIter;

		bool active;

		FileList files;
		CriticalSection cs;
	} mover;
	
	CriticalSection cs;
	Download::List downloads;
	
	bool checkRollback(Download* aDownload, const u_int8_t* aBuf, int aLen) throw(FileException);
	void removeConnection(UserConnection::Ptr aConn, bool reuse = false);
	void removeDownload(Download* aDown, bool finished = false);
	
	friend class Singleton<DownloadManager>;
	DownloadManager() { 
		TimerManager::getInstance()->addListener(this);
	};

	virtual ~DownloadManager() {
		TimerManager::getInstance()->removeListener(this);
		while(true) {
			{
				Lock l(cs);
				if(downloads.empty())
					break;
			}
			Thread::sleep(100);
		}
	};
	
	void checkDownloads(UserConnection* aConn);
	bool handleData(UserConnection* aSource, const u_int8_t* aData, int aLen);
	void handleEndData(UserConnection* aSource);
	
	// UserConnectionListener
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn) throw();
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line) throw();
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const u_int8_t* data, int len) throw();
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, int mode) throw();
	
	void onFileNotAvailable(UserConnection* aSource) throw();
	void onFailed(UserConnection* aSource, const string& aError);
	void onData(UserConnection* aSource, const u_int8_t* aData, int aLen);
	void onFileLength(UserConnection* aSource, const string& aFileLength);
	void onMaxedOut(UserConnection* aSource);
	void onModeChange(UserConnection* aSource, int aNewMode);
	void onSending(UserConnection* aSource);
	
	bool prepareFile(UserConnection* aSource, int64_t newSize = -1);
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t aTick) throw();
	void onTimerSecond(u_int32_t aTick);

};

#endif // !defined(AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_)

/**
 * @file
 * $Id: DownloadManager.h,v 1.52 2003/05/21 12:08:43 arnetheduck Exp $
 */
