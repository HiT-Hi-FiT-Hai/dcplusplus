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

#if !defined(AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_)
#define AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Socket.h"
#include "Semaphore.h"
#include "Thread.h"
#include "Speaker.h"
#include "ZUtils.h"

class InputStream;

class BufferedSocketListener {
public:
	typedef BufferedSocketListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum Types {
		CONNECTING,
		CONNECTED,
		LINE,
		DATA,
		BYTES_SENT,
		MODE_CHANGE,
		TRANSMIT_DONE,
		FAILED
	};
	
	virtual void onAction(Types) throw() { };
	virtual void onAction(Types, u_int32_t, u_int32_t) throw() { };
	virtual void onAction(Types, const string&) throw() { };
	virtual void onAction(Types, const u_int8_t*, int) throw() { };
	virtual void onAction(Types, int) throw() { };
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
		SHUTDOWN,
		ACCEPTED
	};

	/**
	 * BufferedSocket factory
	 * @param sep Line separator
	 * @return An unconnected socket
	 */
	static BufferedSocket* getSocket(char sep) throw(SocketException) { 
		return new BufferedSocket(sep); 
	};

	static void putSocket(BufferedSocket* aSock) { 
		aSock->removeListeners(); 
		aSock->Socket::disconnect();
		Lock l(aSock->cs);
		aSock->addTask(SHUTDOWN); 
	};

	virtual void accept(const ServerSocket& srv) throw(SocketException) { 
		Socket::accept(srv); 
		Lock l(cs);
		addTask(ACCEPTED);
	}

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
	/**
	 * Should be called when data mode.
	 */
	void setLineMode() {
		dcassert(mode == MODE_DATA);
		dcassert(dataBytes == -1);
		mode = MODE_LINE;
	}
	int getMode() { return mode; };
	
	/**
	 * Connect to aAddress / aPort
	 * Note; this one doesn't actually throw, but it overrides one that does...
	 */
	virtual void connect(const string& aAddress, short aPort) throw(SocketException) {
		Lock l(cs);
		mode = MODE_LINE;
		address = aAddress;
		port = aPort;
		addTask(CONNECT);
	}
	
	void write(const string& aData) throw(SocketException) { write(aData.data(), aData.length()); };
	virtual void write(const char* aBuf, int aLen) throw();

	/**
	 * Send the file f over this socket.
	 */
	void transmitFile(InputStream* f) throw() {
		Lock l(cs);
		file = f;
		addTask(SEND_FILE);
	}

	GETSET(char, separator, Separator);
private:
	BufferedSocket(char aSeparator = 0x0a) throw(SocketException) : separator(aSeparator), port(0), mode(MODE_LINE), 
		dataBytes(0), inbufSize(64*1024), curBuf(0), file(NULL) {
		
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
		} catch(const ThreadException& e) {
			delete[] inbuf;
			for(int i = 0; i < BUFFERS; i++) {
				delete[] outbuf[i];
			}
			throw SocketException(e.getError());
		}
	};

	// Dummy...
	BufferedSocket(const BufferedSocket&);
	BufferedSocket& operator=(const BufferedSocket&);

	virtual ~BufferedSocket();

	bool fillBuffer(char* buf, int bufLen, u_int32_t timeout = 0) throw(SocketException);
	
	CriticalSection cs;

	Semaphore taskSem;
	vector<Tasks> tasks;
	string address;
	short port;
	int mode;
	int64_t dataBytes;
	
	string line;
	u_int8_t* inbuf;
	size_t inbufSize;
	enum {BUFFERS = 2};
	u_int8_t* outbuf[BUFFERS];
	int outbufSize[BUFFERS];
	int outbufPos[BUFFERS];
	int curBuf;

	InputStream* file;

	virtual void create(int) throw(SocketException) { dcassert(0); }; // Sockets are created implicitly
	virtual void bind(short) throw(SocketException) { dcassert(0); }; // Binding / UDP not supported...

	virtual int run();

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
		delete this;
	}
	
	void addTask(Tasks task) {
		tasks.push_back(task);
		taskSem.signal();
	}
};

#endif // !defined(AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_)

/**
 * @file
 * $Id: BufferedSocket.h,v 1.56 2004/02/23 17:42:16 arnetheduck Exp $
 */
