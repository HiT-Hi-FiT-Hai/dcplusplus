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

#if !defined(AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_)
#define AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Socket.h"
#include "Util.h"

class File;

class BufferedSocketListener {
public:
	typedef BufferedSocketListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum Types {
		BYTES_SENT,
		CONNECTED,
		LINE,
		FAILED,
		DATA,
		MODE_CHANGE,
		TRANSMIT_DONE
	};
	
	virtual void onAction(Types type) { };
	virtual void onAction(Types type, DWORD) { };
	virtual void onAction(Types type, const string&) { };
	virtual void onAction(Types type, const BYTE*, int) { };
	virtual void onAction(Types type, int) { };
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
		Lock l(cs);
		Socket::write(aBuf, aLen);
	}
	
	BufferedSocket(char aSeparator = 0x0a) : inbuf(NULL), inbufSize(4096), outbufPos(0), outbuf(NULL), outbufSize(4096), file(NULL), separator(aSeparator), readerThread(NULL), mode(MODE_LINE),
		dataBytes(0) {
		writerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		readerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	};

	virtual ~BufferedSocket() {
		stopReader();
		CloseHandle(writerEvent);
		CloseHandle(readerEvent);
		delete inbuf;
		delete outbuf;
	}

	/**
	 * Send the file f over this socket. Note; reading is suspended until the whole file has
	 * been sent.
	 */
	void transmitFile(File* f) throw() {
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

	BYTE* inbuf;
	int inbufSize;

	BYTE* outbuf;
	int outbufSize;
	int outbufPos;

	char separator;

	File* file;

	HANDLE fileWriterEvent;
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
			SetEvent(readerEvent);
			
			if(WaitForSingleObject(readerThread, 5000) == WAIT_TIMEOUT) {
				dcassert(0);
			}
			// Make sure the event is reset in case the thread had already stopped...
			ResetEvent(readerEvent);
			CloseHandle(readerThread);			
			readerThread = NULL;
		}
	}
	
};

#endif // !defined(AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_)

/**
 * @file BufferedSocket.h
 * $Id: BufferedSocket.h,v 1.23 2002/01/26 14:59:22 arnetheduck Exp $
 * @if LOG
 * $Log: BufferedSocket.h,v $
 * Revision 1.23  2002/01/26 14:59:22  arnetheduck
 * Fixed disconnect crash
 *
 * Revision 1.22  2002/01/20 22:54:45  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.21  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.20  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.19  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.18  2002/01/11 14:52:56  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.17  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.16  2001/12/21 23:52:30  arnetheduck
 * Last commit for five days
 *
 * Revision 1.15  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.14  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.13  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
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