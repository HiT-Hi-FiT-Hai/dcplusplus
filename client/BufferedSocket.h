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

	BufferedSocket(char aSeparator = 0x0a) : separator(aSeparator), readerThread(NULL), readerEvent(NULL), mode(MODE_LINE),
		writerEvent(NULL), writerThread(NULL) {

	};

	virtual ~BufferedSocket() {
		stopWriter();
		stopReader();
	}

	void transmitFile(HANDLE f) throw(SocketException){
		file = f;
		startWriter();
	}
private:
	BufferedSocket(const BufferedSocket& aSocket) {
		// Copy still not allowed
	}
	
	string server;
	short port;
	int mode;
	LONGLONG dataBytes;

	char separator;

	HANDLE file;

	HANDLE writerEvent;
	HANDLE writerThread;
	
	HANDLE readerEvent;
	HANDLE readerThread;
	static DWORD WINAPI reader(void* p);
	static DWORD WINAPI writer(void* p);
	
	void startReader() {
		DWORD threadId;
		stopReader();
		
		readerEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
		readerThread=CreateThread(NULL, 0, &reader, this, 0, &threadId);
	}

	void startWriter() {
		DWORD threadId;
		stopWriter();
		
		writerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		writerThread = CreateThread(NULL, 0, &writer, this, 0, &threadId);
		SetThreadPriority(writerThread, THREAD_PRIORITY_LOWEST);
	}
	
	void stopWriter() {
		if(writerThread != NULL) {
			dcassert(writerEvent != NULL);
			SetEvent(writerEvent);
			
			if(WaitForSingleObject(writerThread, 1000) == WAIT_TIMEOUT) {
				MessageBox(NULL, _T("BufferedSocket: Unable to stop writer thread!!!"), _T("Internal error"), MB_OK | MB_ICONERROR);
			}
			
			writerThread = NULL;
		}

		if(writerEvent != NULL) {
			CloseHandle(writerEvent);
			writerEvent = NULL;
		}
	}

	void stopReader() {
		if(readerThread != NULL) {
			dcassert(readerEvent != NULL);
			SetEvent(readerEvent);
			
			if(WaitForSingleObject(readerThread, 1000) == WAIT_TIMEOUT) {
				MessageBox(NULL, _T("BufferedSocket: Unable to stop reader thread!!!"), _T("Internal error"), MB_OK | MB_ICONERROR);
			}
			
			readerThread = NULL;
		}
		if(readerEvent != NULL) {
			CloseHandle(readerEvent);
			readerEvent = NULL;
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
 * $Id: BufferedSocket.h,v 1.8 2001/12/08 14:25:49 arnetheduck Exp $
 * @if LOG
 * $Log: BufferedSocket.h,v $
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