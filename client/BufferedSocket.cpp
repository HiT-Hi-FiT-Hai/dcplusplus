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
dataBytes(0), inbuf(SETTING(SOCKET_IN_BUFFER)), sock(0)
{
}

BufferedSocket::~BufferedSocket() throw() {
	delete sock;
}

void BufferedSocket::accept(const Socket& srv, bool secure) throw(SocketException) {
	dcassert(!sock);
	if(secure) {
		sock = SSLSocketFactory::getInstance()->getServerSocket();
	} else {
		sock = new Socket;
	}
	sock->accept(srv);
	addTask(ACCEPTED, 0);

	try {
		start();
	} catch(const ThreadException& e) {
		throw SocketException(e.getError());
	}
}

void BufferedSocket::connect(const string& aAddress, short aPort, bool secure, bool proxy) throw(SocketException) {
	dcassert(!sock);

	if(secure) {
		sock = SSLSocketFactory::getInstance()->getClientSocket();
	} else {
		sock = new Socket;
	}

	addTask(CONNECT, new ConnectInfo(aAddress, aPort, proxy));

	try {
		start();
	} catch(const ThreadException& e) {
		throw SocketException(e.getError());
	}
}

/**
 * Send a chunk of a file
 * @return True if file is finished, false if there's more data to send
 */
bool BufferedSocket::threadSendFile(InputStream* file) {
	dcassert(file != NULL);
	try {
		vector<u_int8_t> buf;

		for(;;) {
			size_t bytesRead = 0;

			if(buf.empty()) {
				buf.resize(SETTING(SOCKET_OUT_BUFFER));
				bytesRead = buf.size();
				size_t actual = file->read(&buf[0], bytesRead);
				if(actual == 0) {
					fire(BufferedSocketListener::TransmitDone());
					return true;
				}
				buf.resize(actual);
			}

			while(!buf.empty()) {
				int written = sock->write(&buf[0], buf.size());
				if(written == -1) {
					if(sock->wait(1000, Socket::WAIT_WRITE | Socket::WAIT_READ) == Socket::WAIT_READ) {
						// This could be a socket close notification...or strange data coming in while sending file...better check.
						threadRead();
					}
				} else {
					fire(BufferedSocketListener::BytesSent(), bytesRead, written);
					buf.erase(buf.begin(), buf.begin()+written);
				}
				bytesRead = 0;
			}
		}
	} catch(const Exception& e) {
		fail(e.getError());
		return true;
	}
}

void BufferedSocket::threadConnect(const string& aAddr, short aPort, bool proxy) {
	dcdebug("threadConnect()\n");

	fire(BufferedSocketListener::Connecting());

	try {
		sock->create();
		sock->setBlocking(false);

		u_int32_t startTime = GET_TICK();
		if(proxy) {
			sock->socksConnect(aAddr, aPort);
		} else {
			sock->connect(aAddr, aPort);
		}

		
		while(!sock->wait(POLL_TIMEOUT, Socket::WAIT_CONNECT)) {
			{
				Lock l(cs);
				if(!tasks.empty()) {
					// We don't want to handle tasks here...
					sock->disconnect();
					return;
				}
			}
			if((startTime + 30000) < GET_TICK()) {
				// Connection timeout
				fail(STRING(CONNECTION_TIMEOUT));
				return;
			}
		}

		fire(BufferedSocketListener::Connected());
	} catch(const SocketException& e) {
		fail(e.getError());
	}
}	

void BufferedSocket::threadRead() {
	try {
		int left = sock->read(&inbuf[0], (int)inbuf.size());
		if(left == -1) {
			// EWOULDBLOCK, no data recived...
			return;
		} else if(left == 0) {
			// This socket has been closed...
			disconnect();
			return;
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
					bufpos += left;
					left = 0;
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
	} catch(const SocketException& e) {
		dcdebug("BufferedSocket::threadRead caught: %s\n", e.getError().c_str());
		// Ouch...
		fail(e.getError());
		return;
	}
}

void BufferedSocket::write(const char* aBuf, size_t aLen) throw() {
	{
		Lock l(cs);
		writeBuf.insert(writeBuf.end(), aBuf, aBuf+aLen);
		addTask(SEND_DATA, 0);
	}
}

void BufferedSocket::threadSendData() {
	if(sendBuf.empty()) {
		Lock l(cs);
		if(writeBuf.empty())
			return;
	}

	if(sendBuf.empty())
		return;

	int n = sock->write(&sendBuf[0], sendBuf.size());
	if(n == -1)
		return;
	sendBuf.erase(sendBuf.begin(), sendBuf.begin() + n);
}

/**
 * Main task dispatcher for the buffered socket abstraction.
 * @todo Fix the polling...
 */
int BufferedSocket::run() {
	while(true) {
		try {
			while(isConnected() ? taskSem.wait(0) : taskSem.wait()) {
				pair<Tasks, TaskData*> p;
				{
					Lock l(cs);
					dcassert(tasks.size() > 0);
					p = tasks.front();
					tasks.erase(tasks.begin());
				}

				switch(p.first) {
				case SHUTDOWN: 
					threadShutDown(); 
					return 0;
				case DISCONNECT: 
					if(isConnected()) 
						fail(STRING(DISCONNECTED)); 
					break;
				case SEND_FILE: 
					threadSendFile(((SendFileInfo*)p.second)->stream); break;
				case CONNECT: 
					{
						ConnectInfo* ci = (ConnectInfo*)p.second;
						threadConnect(ci->addr, ci->port, ci->proxy); 
						break;
					}
				case ACCEPTED: 
					break;

				default: dcassert("BufferedSocket::threadRun: Unknown command received" == NULL);
				}
				
				delete p.second;
			}

			// Now check if there's any activity on the socket
			if(isConnected()) {
				bool hasData = !sendBuf.empty();
				if(!hasData) {
					Lock l(cs);
					hasData = !writeBuf.empty();
				}

				int waitFor = sock->wait(POLL_TIMEOUT, hasData ? Socket::WAIT_READ | Socket::WAIT_WRITE : Socket::WAIT_READ);
				
				if(waitFor & Socket::WAIT_WRITE) {
					threadSendData();
				}

				if(waitFor & Socket::WAIT_READ) {
					threadRead();
				}
			}
		} catch(const SocketException& e) {
			fail(e.getError());
		}
	}
	return 0;
}

/**
 * @file
 * $Id: BufferedSocket.cpp,v 1.84 2005/11/27 19:19:20 arnetheduck Exp $
 */
