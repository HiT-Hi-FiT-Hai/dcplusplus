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

#include "stdinc.h"
#include "DCPlusPlus.h"
#include "TimerManager.h"

#include "BufferedSocket.h"
#include "File.h"

BufferedSocket* BufferedSocket::accept(const ServerSocket& aSocket, char sep /* = '\n' */, BufferedSocketListener* l /* = NULL */) throw(SocketException) {
	BufferedSocket* b = getSocket(sep);
	if(l != NULL) {
		b->addListener(l);
	}

	try {
		b->Socket::accept(aSocket);
	} catch(...) {
		putSocket(b);
		throw;
	}
	return b;
}

/**
 * Send a chunk of a file
 * @return True if file is finished, false if there's more data to send
 */
bool BufferedSocket::threadSendFile() {
	u_int32_t len;
	dcassert(file != NULL);

	while(size > 0) {
		{
			Lock l(cs);
			if(!tasks.empty())
				return false;
		}
		int waitFor = WAIT_READ;
		if(wait(0, waitFor))
			return false;
		u_int32_t s = (u_int32_t)min((int64_t)inbufSize, size);
		if( (len = file->read(inbuf, s)) == 0) {
			// We don't want this to happen really...disconnect!
			dcdebug("BufferedSocket::threadSendFile Read returned 0!!!");
			disconnect();
			return false;
		}
		Socket::write((char*)inbuf, len);
		fire(BufferedSocketListener::BYTES_SENT, len);
		size -= len;
	}

	fire(BufferedSocketListener::TRANSMIT_DONE);
	return true;
}

void BufferedSocket::threadConnect() {

	u_int32_t start = GET_TICK();
	string s;
	short p;
	{
		Lock l(cs);

		s=server;
		p=port;
	}

	try {
		setBlocking(false);
		Socket::create();
		Socket::connect(s, p);
		
		int waitFor = WAIT_CONNECT;

		while(!wait(200, waitFor)) {
			{
				Lock l(cs);
				if(!tasks.empty()) {
					// We don't want to handle tasks here...
					Socket::disconnect();
					return;
				}
			}
			if((start + 30000) < GET_TICK()) {
				// Connection timeout
				Socket::disconnect();
				fire(BufferedSocketListener::FAILED, STRING(CONNECTION_TIMEOUT));
				return;
			}
		}

		// We're connected! Clear the buffers...
		for(int k = 0; k < BUFFERS; k++) {
			outbufPos[k] = 0;
		}
		// Update the default local ip...this one should in any case be better than 
		// whatever we might have guessed from the beginning...
		SettingsManager::getInstance()->setDefault(SettingsManager::SERVER, getLocalIp());
		fire(BufferedSocketListener::CONNECTED);

		setBlocking(true);
	} catch(SocketException e) {
		Socket::disconnect();
		fire(BufferedSocketListener::FAILED, e.getError());
		return;
	}
}	

void BufferedSocket::threadRead() {
	try {
		int waitFor = WAIT_READ;
		while(wait(0, waitFor)) {
			int i = read(inbuf, inbufSize);
			if(i == -1) {
				// EWOULDBLOCK, no data recived...
				return;
			} else if(i == 0) {
				// This socket has been closed...
				disconnect();
				return;
			}

			int bufpos = 0;
			string l;

			while(i) {
				if(mode == MODE_LINE) {
					string::size_type pos;

					l = string((char*)inbuf + bufpos, i);

					if( (pos = l.find(separator)) != string::npos) {
						bufpos += sizeof(separator) + pos;
						if(!line.empty()) {
							fire(BufferedSocketListener::LINE, line + l.substr(0, pos));
							line = Util::emptyString;
						} else {
							fire(BufferedSocketListener::LINE, l.substr(0, pos));
						}
						l = l.substr(pos+1);
						i-=(pos+1);
					} else {
						line += l;
						i = 0;
					}
				} else if(mode == MODE_DATA) {
					if(dataBytes == -1) {
						fire(BufferedSocketListener::DATA, inbuf+bufpos, i);
						bufpos+=i;
						i = 0;
					} else {
						int high = (int)min(dataBytes, (int64_t)i);
						fire(BufferedSocketListener::DATA, inbuf+bufpos, high);
						bufpos += high;
						i-=high;

						dataBytes -= high;
						if(dataBytes == 0) {
							fire(BufferedSocketListener::MODE_CHANGE, MODE_LINE);
							mode = MODE_LINE;
						}
					}
				}
			}
		}
	} catch(SocketException e) {
		dcdebug("BufferedSocket::threadRead caught: %s\n", e.getError().c_str());
		// Ouch...
		fail(e.getError());
		return;
	}
}

void BufferedSocket::write(const char* aBuf, int aLen) throw() {

	{
		Lock l(cs);
		
		while(outbufSize[curBuf] < (aLen + outbufPos[curBuf])) {
			// Need to grow...
			outbufSize[curBuf]*=2;
			dcdebug("Growing outbuf[%d] to %d bytes\n", curBuf, outbufSize[curBuf]);
			u_int8_t* tmp = new u_int8_t[outbufSize[curBuf]];
			memcpy(tmp, outbuf[curBuf], outbufPos[curBuf]);
			delete[] outbuf[curBuf];
			outbuf[curBuf] = tmp;
		}

		memcpy(outbuf[curBuf] + outbufPos[curBuf], aBuf, aLen);
		outbufPos[curBuf] += aLen;
		addTask(SEND_DATA);
	}
}

void BufferedSocket::threadSendData() {
	int myBuf;

	{
		Lock l(cs);
		myBuf = curBuf;
		curBuf = (curBuf + 1) % BUFFERS;
	}

	if(outbufPos[myBuf] == 0)
		return;

	Socket::write((char*)outbuf[myBuf], outbufPos[myBuf]);
	outbufPos[myBuf] = 0;
}

/**
 * Main task dispatcher for the buffered socket abstraction.
 * @todo Fix the polling...
 */
void BufferedSocket::threadRun() {
	bool sendingFile = false;

	while(true) {
		try {

			while(isConnected() ? taskSem.wait(0) : taskSem.wait()) {
				Tasks t;
				{
					Lock l(cs);
					dcassert(tasks.size() > 0);
					t = tasks.front();
					tasks.erase(tasks.begin());
				}

				switch(t) {
				case SHUTDOWN: threadShutDown(); return;
				case DISCONNECT:
					if(isConnected()) {
						Socket::disconnect(); 
						fire(BufferedSocketListener::FAILED, STRING(DISCONNECTED));
					}
					break;

				case SEND_FILE: if(isConnected()) sendingFile = true; break;
				case SEND_DATA: dcassert(!sendingFile); if(isConnected()) threadSendData(); break;
				case CONNECT: threadConnect(); sendingFile = false; break;

				default: dcassert("BufferedSocket::threadRun: Unknown command received" == NULL);
				}
			}
			// Now check if there's any activity on the socket

			if(isConnected()) {
				int waitFor = sendingFile ? WAIT_READ | WAIT_WRITE : WAIT_READ;
				if(wait(200, waitFor)) {
					if(waitFor & WAIT_WRITE) {
						dcassert(sendingFile);
						if(threadSendFile())
							sendingFile = false;
					}
					if(waitFor & WAIT_READ) {
						threadRead();
					}
				}
			}
		} catch(SocketException e) {
			fail(e.getError());
		}
	}
}

/**
 * @file BufferedSocket.cpp
 * $Id: BufferedSocket.cpp,v 1.42 2002/06/01 19:38:28 arnetheduck Exp $
 */