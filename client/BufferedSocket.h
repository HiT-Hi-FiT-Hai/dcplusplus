/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

class InputStream;

class BufferedSocketListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Connecting;
	typedef X<1> Connected;
	typedef X<2> Line;
	typedef X<3> Data;
	typedef X<4> BytesSent;
	typedef X<5> ModeChange;
	typedef X<6> TransmitDone;
	typedef X<7> Failed;
	typedef X<8> Shutdown;

	virtual void on(Connecting) throw() { }
	virtual void on(Connected) throw() { }
	virtual void on(Line, const string&) throw() { }
	virtual void on(Data, u_int8_t*, size_t) throw() { }
	virtual void on(BytesSent, size_t, size_t) throw() { }
	virtual void on(ModeChange) throw() { }
	virtual void on(TransmitDone) throw() { }
	virtual void on(Failed, const string&) throw() { }
	virtual void on(Shutdown) throw() { }
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
	 * @param esc A preceding backslash escapes any character, including the separator
	 * @return An unconnected socket
	 */
	static BufferedSocket* getSocket(char sep, bool esc = false) throw(SocketException) { 
		return new BufferedSocket(sep, esc); 
	};

	static void putSocket(BufferedSocket* aSock) { 
		aSock->removeListeners(); 
		aSock->Socket::disconnect();
		aSock->shutdown();
	};

	virtual void shutdown() {
		Lock l(cs);
		addTask(SHUTDOWN);
	}

	virtual void accept(const ServerSocket& srv) throw(SocketException) { 
		Socket::accept(srv); 
		Lock l(cs);
		addTask(ACCEPTED);
	}

	virtual void disconnect() throw() {
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
	virtual void write(const char* aBuf, size_t aLen) throw();

	/**
	 * Send the file f over this socket.
	 */
	void transmitFile(InputStream* f) throw() {
		Lock l(cs);
		file = f;
		addTask(SEND_FILE);
	}

	GETSET(char, separator, Separator);
	GETSET(bool, usesEscapes, UsesEscapes);
private:
	BufferedSocket(char aSeparator = 0x0a, bool aUsesEscapes = false) throw(SocketException);

	// Dummy...
	BufferedSocket(const BufferedSocket&);
	BufferedSocket& operator=(const BufferedSocket&);

	virtual ~BufferedSocket() throw();

	bool fillBuffer(char* buf, int bufLen, u_int32_t timeout = 0) throw(SocketException);
	
	CriticalSection cs;

	Semaphore taskSem;
	vector<Tasks> tasks;
	string address;
	short port;
	int mode;
	int64_t dataBytes;
	
	string line;
	bool escaped;
	u_int8_t* inbuf;
	size_t inbufSize;
	enum {BUFFERS = 2};
	u_int8_t* outbuf[BUFFERS];
	size_t outbufSize[BUFFERS];
	size_t outbufPos[BUFFERS];
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
		fire(BufferedSocketListener::Failed(), aError);
	}

	/**
	 * Shut down the socket and delete itself...no variables must be referenced after
	 * calling threadShutDown, the thread function should exit as soon as possible
	 */
	void threadShutDown() {
		fire(BufferedSocketListener::Shutdown());
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
 * $Id: BufferedSocket.h,v 1.65 2005/01/06 18:19:48 arnetheduck Exp $
 */
