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
#include "Semaphore.h"
#include "Thread.h"

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
	
	virtual void onAction(Types) { };
	virtual void onAction(Types, u_int32_t) { };
	virtual void onAction(Types, const string&) { };
	virtual void onAction(Types, const u_int8_t*, int) { };
	virtual void onAction(Types, int) { };
};

class BufferedSocket : public Speaker<BufferedSocketListener>, public Socket, public Thread
{
public:
	enum {	
		MODE_LINE,
		MODE_DATA
	};

	enum Tasks {
		CONNECT,
		DISCONNECT,
		SEND_DATA,
		SEND_FILE,
		SHUTDOWN
	};

	/**
	 * BufferedSocket factory
	 * @param sep Line separator
	 * @return An unconnected socket
	 */
	static BufferedSocket* getSocket(char sep = '\n') throw(SocketException) { 
		return new BufferedSocket(sep); 
	};
	static BufferedSocket* accept(const ServerSocket& aSocket, char sep = '\n', BufferedSocketListener* l = NULL) throw(SocketException);
	
	static void putSocket(BufferedSocket* aSock) { 
		aSock->removeListeners(); 
		aSock->Socket::disconnect();
		Lock l(aSock->cs);
		aSock->addTask(SHUTDOWN); 
	};

	virtual void disconnect() {
		Lock l(cs);
		addTask(DISCONNECT);
	}
	
	/**
	 * Sets data mode for aBytes bytes long. Must be called within an action method...
	 */
	void setDataMode(int64_t aBytes = -1) {
		mode = MODE_DATA;
		dataBytes = aBytes;
	}

	int getMode() { return mode; };
	
	/**
	 * Connect to aServer / aPort
	 * Note; this one doesn't actually throw, but it overrides one that does...
	 */
	virtual void connect(const string& aServer, short aPort) throw(SocketException) {
		Lock l(cs);
		mode = MODE_LINE;
		server = aServer;
		port = aPort;
		addTask(CONNECT);
	}
	
	virtual void write(const string& aData) throw() {
		write(aData.data(), aData.length());
	}
	virtual void write(const char* aBuf, int aLen) throw();

	/**
	 * Send the file f over this socket. Note; reading is suspended until the whole file has
	 * been sent.
	 */
	void transmitFile(File* f, int64_t s) throw() {
		Lock l(cs);
		file = f;
		size = s;
		addTask(SEND_FILE);
	}

	GETSET(char, separator, Separator);
private:
	BufferedSocket(char aSeparator = 0x0a) throw(SocketException) : separator(aSeparator), port(0), mode(MODE_LINE), 
		dataBytes(0), inbufSize(16384), curBuf(0), file(NULL), size(0) {
		
		inbuf = new u_int8_t[inbufSize];
		
		// MSVC: Non-standard scope for i
		{
			for(int i = 0; i < BUFFERS; i++) {
				outbuf[i] = new u_int8_t[inbufSize];
				outbufPos[i] = 0;
				outbufSize[i] = inbufSize;
			}
		}
		try {
			start();
		} catch(ThreadException e) {
			delete[] inbuf;
			for(int i = 0; i < BUFFERS; i++) {
				delete[] outbuf[i];
			}
			throw SocketException(e.getError());
		}
	};
	
	virtual ~BufferedSocket() {
		delete[] inbuf;
		for(int i = 0; i < BUFFERS; i++) {
			delete[] outbuf[i];
		}
	}

	BufferedSocket(const BufferedSocket&) {
		// Copy still not allowed
	}

	CriticalSection cs;

	Semaphore taskSem;
	vector<Tasks> tasks;
	string server;
	short port;
	int mode;
	int64_t dataBytes;
	
	string line;
	u_int8_t* inbuf;
	int inbufSize;
	enum {BUFFERS = 2};
	u_int8_t* outbuf[BUFFERS];
	int outbufSize[BUFFERS];
	int outbufPos[BUFFERS];
	int curBuf;

	File* file;
	int64_t size;

	virtual void accept(const ServerSocket& ) throw(SocketException) { }; // We don't want people accepting BufferedSockets this way...
	virtual void create(int) throw(SocketException) { dcassert(0); }; // Sockets are created implicitly
	virtual void bind(short) throw(SocketException) { dcassert(0); }; // Binding / UDP not supported...

	// From Thread
	virtual int run() { threadRun(); return 0; };

	void threadRun();
	void threadConnect();
	void threadRead();
	bool threadSendFile();
	void threadSendData();
	void threadDisconnect();
	
	void fail(const string& aError) {
		Socket::disconnect();
		fire(BufferedSocketListener::FAILED, aError);
	}

	/**
	 * Shut down the socket and delete itself...no variables must be referenced after
	 * calling threadShutDown, the thread function should exit as soon as possible
	 */
	void threadShutDown() {
		removeListeners();
		Socket::disconnect();
		delete this;
	}
	
	void addTask(Tasks task) {
		tasks.push_back(task);
		taskSem.signal();
	}
};

#endif // !defined(AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_)

/**
 * @file BufferedSocket.h
 * $Id: BufferedSocket.h,v 1.42 2002/06/13 17:50:38 arnetheduck Exp $
 */
