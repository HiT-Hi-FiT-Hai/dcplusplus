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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "BufferedSocket.h"

#include "ResourceManager.h"
#include "TimerManager.h"
#include "SettingsManager.h"

#include "Streams.h"
#include "SSLSocket.h"

// Polling is used for tasks...should be fixed...
#define POLL_TIMEOUT 250

BufferedSocket::BufferedSocket(char aSeparator) throw() : 
separator(aSeparator), mode(MODE_LINE), 
dataBytes(0), rollback(0), failed(false), inbuf(SETTING(SOCKET_IN_BUFFER)), sock(0), disconnecting(false)
{
}

BufferedSocket::~BufferedSocket() throw() {
	delete sock;
}

void BufferedSocket::accept(const Socket& srv, bool secure) throw(SocketException, ThreadException) {
	dcassert(!sock);

	sock = secure ? SSLSocketFactory::getInstance()->getClientSocket() : new Socket;

	sock->accept(srv);
	sock->setSocketOpt(SO_RCVBUF, SETTING(SOCKET_IN_BUFFER));
	sock->setSocketOpt(SO_SNDBUF, SETTING(SOCKET_OUT_BUFFER));
	sock->setBlocking(false);

	try {
		start();
	} catch(...) {
		delete sock;
		sock = 0;
		throw;
	}

	Lock l(cs);
	addTask(ACCEPTED, 0);
}

void BufferedSocket::connect(const string& aAddress, short aPort, bool secure, bool proxy) throw(SocketException, ThreadException) {
	dcassert(!sock);

	sock = secure ? SSLSocketFactory::getInstance()->getClientSocket() : new Socket;

	sock->create();
	sock->setSocketOpt(SO_RCVBUF, SETTING(SOCKET_IN_BUFFER));
	sock->setSocketOpt(SO_SNDBUF, SETTING(SOCKET_OUT_BUFFER));
	sock->setBlocking(false);

	try {
		start();
	} catch(...) {
		delete sock;
		sock = 0;
		throw;
	}

	Lock l(cs);
	addTask(CONNECT, new ConnectInfo(aAddress, aPort, proxy && (SETTING(OUTGOING_CONNECTIONS) == SettingsManager::OUTGOING_SOCKS5)));
}

#define CONNECT_TIMEOUT 30000
void BufferedSocket::threadConnect(const string& aAddr, short aPort, bool proxy) throw(SocketException) {
	dcdebug("threadConnect %s:%d\n", aAddr.c_str(), (int)aPort);
	dcassert(sock);
	if(!sock)
		return;
	fire(BufferedSocketListener::Connecting());

	u_int32_t startTime = GET_TICK();
	if(proxy) {
		sock->socksConnect(aAddr, aPort, CONNECT_TIMEOUT);
	} else {
		sock->connect(aAddr, aPort);
	}

	while(sock->wait(POLL_TIMEOUT, Socket::WAIT_CONNECT) != Socket::WAIT_CONNECT) {
		if(disconnecting)
			return;

		if((startTime + 30000) < GET_TICK()) {
			throw SocketException(STRING(CONNECTION_TIMEOUT));
		}
	}

	fire(BufferedSocketListener::Connected());
}

void BufferedSocket::threadRead() throw(SocketException) {
	dcassert(sock);
	if(!sock)
		return;
	int left = sock->read(&inbuf[0], (int)inbuf.size());
	if(left == -1) {
		// EWOULDBLOCK, no data received...
		return;
	} else if(left == 0) {
		// This socket has been closed...
		throw SocketException(STRING(CONNECTION_CLOSED));
	}

	int bufpos = 0;
	string l;
	while(left > 0) {
		if(mode == MODE_LINE) {
			// Special to autodetect nmdc connections...
			if(separator == 0) {
				if(inbuf[0] == '$') {
					separator = '|';
				} else {
					separator = '\n';
				}
			}
			string::size_type pos = 0;

			l = string((char*)&inbuf[0] + bufpos, left);

			if((pos = l.find(separator)) != string::npos) {
				if(!line.empty()) {
					fire(BufferedSocketListener::Line(), line + l.substr(0, pos));
					line.clear();
				} else {
					fire(BufferedSocketListener::Line(), l.substr(0, pos));
				}
				left -= (pos + sizeof(separator));
				bufpos += (pos + sizeof(separator));
			} else {
				line += l;
				left = 0;
			}
		} else if(mode == MODE_DATA) {
			if(dataBytes == -1) {
				fire(BufferedSocketListener::Data(), &inbuf[bufpos], left);
				bufpos += (left - rollback);
				left = rollback;
				rollback = 0;
			} else {
				int high = (int)min(dataBytes, (int64_t)left);
				fire(BufferedSocketListener::Data(), &inbuf[bufpos], high);
				bufpos += high;
				left -= high;

				dataBytes -= high;
				if(dataBytes == 0) {
					mode = MODE_LINE;
					fire(BufferedSocketListener::ModeChange());
				}
			}
		}
	}
}

void BufferedSocket::threadSendFile(InputStream* file) throw(Exception) {
	dcassert(sock);
	if(!sock)
		return;
	dcassert(file != NULL);
	vector<u_int8_t> buf;

	while(true) {
		buf.resize(SETTING(SOCKET_OUT_BUFFER));
		size_t bytesRead = buf.size();
		size_t actual = file->read(&buf[0], bytesRead);
		if(actual == 0) {
			fire(BufferedSocketListener::TransmitDone());
			return;
		}
		buf.resize(actual);

		while(!buf.empty()) {
			if(disconnecting)
				return;

			int w = sock->wait(POLL_TIMEOUT, Socket::WAIT_WRITE | Socket::WAIT_READ);
			if(w & Socket::WAIT_READ) {
				threadRead();
			}
			if(w & Socket::WAIT_WRITE) {
				int written = sock->write(&buf[0], buf.size());
				if(written > 0) {
					buf.erase(buf.begin(), buf.begin()+written);

					fire(BufferedSocketListener::BytesSent(), bytesRead, written);
					bytesRead = 0;		// Make sure we only report the bytes we actually read just once...
				}
			}
		}
	}
}

void BufferedSocket::write(const char* aBuf, size_t aLen) throw() {
	dcassert(sock);
	if(!sock)
		return;
	Lock l(cs);
	if(writeBuf.empty())
		addTask(SEND_DATA, 0);

	writeBuf.insert(writeBuf.end(), aBuf, aBuf+aLen);
}

void BufferedSocket::threadSendData() {
	dcassert(sock);
	if(!sock)
		return;
	{
		Lock l(cs);
		if(writeBuf.empty())
			return;

		writeBuf.swap(sendBuf);
	}

	size_t left = sendBuf.size();
	size_t done = 0;
	while(left > 0) {
		if(disconnecting) {
			return;
		}

		int w = sock->wait(POLL_TIMEOUT, Socket::WAIT_READ | Socket::WAIT_WRITE);

		if(w & Socket::WAIT_READ) {
			threadRead();
		}

		if(w & Socket::WAIT_WRITE) {
			int n = sock->write(&sendBuf[done], left);
			if(n > 0) {
				left -= n;
				done += n;
			}
		}
	}
	sendBuf.clear();
}

bool BufferedSocket::checkEvents() {
	while(isConnected() ? taskSem.wait(0) : taskSem.wait()) {
		pair<Tasks, TaskData*> p;
		{
			Lock l(cs);
			dcassert(tasks.size() > 0);
			p = tasks.front();
			tasks.erase(tasks.begin());
		}
		if(failed && p.first != SHUTDOWN) {
			dcdebug("BufferedSocket: New commands when already failed\n");
			fail(STRING(DISCONNECTED));
			delete p.second;
			continue;
		}

		switch(p.first) {
			case SEND_DATA:
				threadSendData(); break;
			case SEND_FILE: 
				threadSendFile(((SendFileInfo*)p.second)->stream); break;
			case CONNECT: 
				{
					ConnectInfo* ci = (ConnectInfo*)p.second;
					threadConnect(ci->addr, ci->port, ci->proxy); 
					break;
				}
			case DISCONNECT: 
				if(isConnected()) 
					fail(STRING(DISCONNECTED)); 
				break;
			case SHUTDOWN: 
				return false;
		}

		delete p.second;
	}
	return true;
}

void BufferedSocket::checkSocket() {
	dcassert(sock);
	if(!sock)
		return;

	int waitFor = sock->wait(POLL_TIMEOUT, Socket::WAIT_READ);

	if(waitFor & Socket::WAIT_READ) {
		threadRead();
	}
}

/**
 * Main task dispatcher for the buffered socket abstraction.
 * @todo Fix the polling...
 */
int BufferedSocket::run() {
	while(true) {
		try {
			if(!checkEvents())
				break;
			checkSocket();
		} catch(const Exception& e) {
			fail(e.getError());
		}
	}
	delete this;
	return 0;
}

void BufferedSocket::shutdown() { 
	if(sock) {
		Lock l(cs); 
		disconnecting = true; 
		addTask(SHUTDOWN, 0); 
	} else {
		// Socket thread not running yet, disconnect...
		delete this;
	}

}

/**
 * @file
 * $Id: BufferedSocket.cpp,v 1.96 2006/01/21 10:38:01 arnetheduck Exp $
 */
