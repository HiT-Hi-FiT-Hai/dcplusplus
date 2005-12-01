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

#if !defined(BUFFERED_SOCKET_H)
#define BUFFERED_SOCKET_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Semaphore.h"
#include "Thread.h"
#include "Speaker.h"
#include "Util.h"
#include "Socket.h"

class InputStream;
class Socket;
class SocketException;

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

	virtual void on(Connecting) throw() { }
	virtual void on(Connected) throw() { }
	virtual void on(Line, const string&) throw() { }
	virtual void on(Data, u_int8_t*, size_t) throw() { }
	virtual void on(BytesSent, size_t, size_t) throw() { }
	virtual void on(ModeChange) throw() { }
	virtual void on(TransmitDone) throw() { }
	virtual void on(Failed, const string&) throw() { }
};

class BufferedSocket : public Speaker<BufferedSocketListener>, public Thread
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
	 * BufferedSocket factory, each BufferedSocket may only be used to create one connection
	 * @param sep Line separator
	 * @return An unconnected socket
	 */
	static BufferedSocket* getSocket(char sep) throw() { 
		return new BufferedSocket(sep); 
	};

	static void putSocket(BufferedSocket* aSock) { 
		aSock->removeListeners(); 
		aSock->shutdown();
	};

	void shutdown() {
		Lock l(cs);
		addTask(SHUTDOWN, 0);
	}

	void accept(const Socket& srv, bool secure) throw(SocketException, ThreadException);
	void connect(const string& aAddress, short aPort, bool secure, bool proxy) throw(ThreadException);

	void disconnect() throw() {
		Lock l(cs);
		addTask(DISCONNECT, 0);
	}
	
	/** Sets data mode for aBytes bytes long. Must be called within an action method... */
	void setDataMode(int64_t aBytes = -1) {
		mode = MODE_DATA;
		dataBytes = aBytes;
	}
	/** Should be called when data mode. */
	void setLineMode() {
		dcassert(mode == MODE_DATA);
		dcassert(dataBytes == -1);
		mode = MODE_LINE;
	}
	int getMode() { return mode; };
	const string& getIp() { return sock ? sock->getIp() : Util::emptyString; }
	bool isConnected() { return sock && sock->isConnected(); }
	
	void write(const string& aData) throw(SocketException) { write(aData.data(), aData.length()); };
	virtual void write(const char* aBuf, size_t aLen) throw();

	/** Send the file f over this socket. */
	void transmitFile(InputStream* f) throw() {
		Lock l(cs);
		addTask(SEND_FILE, new SendFileInfo(f));
	}

	string getLocalIp() { return sock->getLocalIp(); }

	GETSET(char, separator, Separator)
private:

	struct TaskData { 
		virtual ~TaskData() { };
	};
	struct ConnectInfo : public TaskData {
		ConnectInfo(string addr_, short port_, bool proxy_) : addr(addr_), port(port_), proxy(proxy_) { }
		string addr;
		short port;
		bool proxy;
	};
	struct SendFileInfo : public TaskData {
		SendFileInfo(InputStream* stream_) : stream(stream_) { }
		InputStream* stream;
	};

	BufferedSocket(char aSeparator) throw();

	// Dummy...
	BufferedSocket(const BufferedSocket&);
	BufferedSocket& operator=(const BufferedSocket&);

	virtual ~BufferedSocket() throw();

	CriticalSection cs;

	Semaphore taskSem;
	vector<pair<Tasks, TaskData*> > tasks;

	int mode;
	int64_t dataBytes;
	
	string line;
	vector<u_int8_t> inbuf;
	vector<u_int8_t> writeBuf;
	vector<u_int8_t> sendBuf;

	Socket* sock;

	virtual int run();

	void threadConnect(const string& aAddr, short aPort, bool proxy) throw(SocketException);
	void threadRead() throw(SocketException);
	void threadSendFile(InputStream* is) throw(Exception);
	void threadSendData();
	void threadDisconnect();
	
	void fail(const string& aError) {
		sock->disconnect();
		fire(BufferedSocketListener::Failed(), aError);
	}

	bool checkEvents();
	void checkSocket();

	/**
	 * Shut down the socket and delete itself...no variables must be referenced after
	 * calling threadShutDown, the thread function should exit as soon as possible
	 */
	void threadShutDown() {
		delete this;
	}
	
	void addTask(Tasks task, TaskData* data) {
		tasks.push_back(make_pair(task, data));
		taskSem.signal();
	}
};

#endif // !defined(BUFFERED_SOCKET_H)

/**
 * @file
 * $Id: BufferedSocket.h,v 1.71 2005/12/01 00:01:14 arnetheduck Exp $
 */
