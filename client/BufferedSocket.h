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

#if !defined(AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_)
#define AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Socket.h"
#include "Util.h"

class BufferedSocketListener {
public:
	typedef BufferedSocketListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	virtual void onBytesSent(DWORD bytes) { };
	virtual void onConnected() { };
	virtual void onLine(const string& aLine) { };
	virtual void onError(const string& aReason) { };
	virtual void onData(BYTE* aBuf, int aLen) { };
	virtual void onModeChange(int newMode) { };
	virtual void onTransmitDone() { };
};

class BufferedSocket : public Speaker<BufferedSocketListener>, public Socket  
{
public:
	enum {	
		MODE_LINE,
		MODE_DATA,
		MODE_DGRAM
	};

	void setDataMode(LONGLONG aBytes = -1) {
		mode = MODE_DATA;
		dataBytes = aBytes;
	}

	int getMode() { return mode; };
	
	virtual void bind(short aPort) {
		Socket::bind(aPort);
		mode = MODE_DGRAM;
		startReader();
	}

	virtual void connect(const string& aServer, short aPort) {
		server = aServer;
		port = aPort;
		startReader();
	}
	
	virtual void accept(const ServerSocket& aSocket);
	char getSeparator() { return separator; };
	void setSeparator(char aSeparator) { separator = aSeparator; };

	virtual void write(const string& aData) throw(SocketException) {
		write(aData.data(), aData.length());
	}
	virtual void write(const char* aBuf, int aLen) throw(SocketException) {
		cs.enter();
		try {
			Socket::write(aBuf, aLen);
		} catch(...) {
			cs.leave();
			throw;			
		}
		cs.leave();
	}
	
	BufferedSocket(char aSeparator = 0x0a) : separator(aSeparator), readerThread(NULL), readerEvent(NULL), mode(MODE_LINE),
		dataBytes(0) {
		writerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		readerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	};

	virtual ~BufferedSocket() {
		stopReader();
		CloseHandle(writerEvent);
		CloseHandle(readerEvent);
	}

	/**
	 * Send the file f over this socket. Note; reading is suspended until the whole file has
	 * been sent.
	 */
	void transmitFile(HANDLE f) throw(SocketException){
		file = f;
		SetEvent(writerEvent);
	}
private:
	BufferedSocket(const BufferedSocket& aSocket) {
		// Copy still not allowed
	}

	CriticalSection cs;
	
	string server;
	short port;
	int mode;
	LONGLONG dataBytes;

	char separator;

	HANDLE file;

	HANDLE writerEvent;
	
	HANDLE readerEvent;
	HANDLE readerThread;
	static DWORD WINAPI reader(void* p);
	static bool writer(BufferedSocket* bs);
	
	void startReader() {
		DWORD threadId;
		stopReader();
		
		readerThread=CreateThread(NULL, 0, &reader, this, 0, &threadId);
	}

	void stopReader() {
		if(readerThread != NULL) {
			dcassert(readerEvent != NULL);
			SetEvent(readerEvent);
			
			if(WaitForSingleObject(readerThread, 1000) == WAIT_TIMEOUT) {
				dcassert(0);
			}
			CloseHandle(readerThread);			
			readerThread = NULL;
		}
	}
	
	void fireBytesSent(DWORD aLen) {
		listenerCS.enter();
		BufferedSocketListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(BufferedSocketListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onBytesSent(aLen);
		}
	}
	void fireConnected() {
		listenerCS.enter();
		BufferedSocketListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(BufferedSocketListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onConnected();
		}
	}
	void fireData(BYTE* aBuf, int aLen) {
		listenerCS.enter();
		BufferedSocketListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(BufferedSocketListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onData(aBuf, aLen);
		}
	}
	void fireError(const string& aLine) {
		listenerCS.enter();
		BufferedSocketListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(BufferedSocketListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onError(aLine);
		}
	}
	void fireLine(const string& aLine) {
		listenerCS.enter();
		BufferedSocketListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(BufferedSocketListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onLine(aLine);
		}
	}
	void fireModeChange(int aNewMode) {
		listenerCS.enter();
		BufferedSocketListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(BufferedSocketListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onModeChange(aNewMode);
		}
	}
	void fireTransmitDone() {
		listenerCS.enter();
		BufferedSocketListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(BufferedSocketListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onTransmitDone();
		}
	}
	
};

#endif // !defined(AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_)

/**
 * @file BufferedSocket.h
 * $Id: BufferedSocket.h,v 1.12 2001/12/12 00:06:04 arnetheduck Exp $
 * @if LOG
 * $Log: BufferedSocket.h,v $
 * Revision 1.12  2001/12/12 00:06:04  arnetheduck
 * Updated the public hub listings, fixed some minor transfer bugs, reworked the
 * sockets to use only one thread (instead of an extra thread for sending files),
 * and fixed a major bug in the client command decoding (still have to fix this
 * one for the userconnections...)
 *
 * Revision 1.11  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.10  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.9  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.8  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.7  2001/12/07 20:03:01  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.6  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.5  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
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
 * Revision 1.2  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.1  2001/11/24 10:39:00  arnetheduck
 * New BufferedSocket creates reader threads and reports inbound data through a listener.
 *
 * @endif
 */