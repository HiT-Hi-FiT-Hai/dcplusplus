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
 * Write a file to the socket, stops reading from the socket until the transfer's finished.
 * @return True if everythings ok and the thread should continue reading, false on error
 */
void BufferedSocket::threadSendFile() {
	u_int32_t len;
	dcassert(file != NULL);
	
	if(!isConnected()) {
		fire(BufferedSocketListener::FAILED, STRING(NOT_CONNECTED));
		return;
	}

	try{
		while(!taskSem.wait(0)) {

			if( (len = file->read(inbuf, inbufSize)) == 0) {
				fire(BufferedSocketListener::TRANSMIT_DONE);
				return;
			}
			Socket::write((char*)inbuf, len);
			fire(BufferedSocketListener::BYTES_SENT, len);
		}
	} catch(Exception e) {
		dcdebug("BufferedSocket::Writer caught: %s\n", e.getError().c_str());
		fire(BufferedSocketListener::FAILED, e.getError());
		return;
	}
		
	// Signal the task again since we don't handle it here...
	taskSem.signal();
#ifdef _DEBUG
	{
		Lock l(cs);
		
		// Check the task queue that we don't have a pending shutdown / disconnect...
		for(vector<Tasks>::iterator i = tasks.begin(); i != tasks.end(); ++i) {
			if(*i != SHUTDOWN && *i != DISCONNECT) {
				// Should never happen...
				dcdebug("Bad tasks in BufferedSocket in threadSendFile, %d\n", *i);
				dcassert(0);
			} 
		}
	}
#endif
	
}

bool BufferedSocket::threadBind() {
	try {
		Socket::create(TYPE_UDP);
		Socket::bind(port);
	} catch(SocketException e) {
		dcdebug("BufferedSocket::threadBind caught: %s\n", e.getError().c_str());
		fail(e.getError());
		return false;
	}
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

		while(!waitForConnect(200)) {
			if(taskSem.wait(0)) {
				// We don't want to handle tasks here, push it back and return...
				taskSem.signal();
				Socket::disconnect();
				return;
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
		while(waitForData(0)) {
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
				} else if(mode == MODE_DGRAM) {
					fire(BufferedSocketListener::DATA, inbuf, i);
					i = 0;
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

void BufferedSocket::threadSendData() {

	if(outbufPos[curBuf] == 0)
		return;
	
	try {
		u_int8_t* buf;
		int len;
		{
			Lock l(cs);
			buf = outbuf[curBuf];
			len = outbufPos[curBuf];
			outbufPos[curBuf] = 0;
			curBuf = (curBuf + 1) % BUFFERS;
		}
		
		Socket::write((char*)buf, len);
	} catch(SocketException e) {
		dcdebug("BufferedSocket::threadSendData caught: %s\n", e.getError().c_str());
		// Ouch...
		fail(e.getError());
	}
}

void BufferedSocket::write(const char* aBuf, int aLen) throw() {

	{
		Lock l(cs);
		int mybuf = curBuf;
		
		while(outbufSize[mybuf] < (aLen + outbufPos[mybuf])) {
			// Need to grow...
			outbufSize[mybuf]*=2;
			dcdebug("Growing outbuf[%d] to %d bytes\n", mybuf, outbufSize[mybuf]);
			u_int8_t* tmp = new u_int8_t[outbufSize[mybuf]];
			memcpy(tmp, outbuf[mybuf], outbufPos[mybuf]);
			delete[] outbuf[mybuf];
			outbuf[mybuf] = tmp;
		}

		memcpy(outbuf[mybuf] + outbufPos[mybuf], aBuf, aLen);
		outbufPos[mybuf] += aLen;
		addTask(SEND_DATA);
	}
}

/**
 * Main task dispatcher for the buffered socket abstraction.
 * @todo Fix the polling...
 */
void BufferedSocket::threadRun() {

	while(true) {
		while(isConnected() ? taskSem.wait(0) : taskSem.wait()) {
			Tasks t;
			{
				Lock l(cs);
				if(tasks.size() > 0) {
					t = tasks.front();
					tasks.erase(tasks.begin());
				} else {
					break;
				}
			}

			switch(t) {
			case SHUTDOWN: threadShutDown(); return;
			case DISCONNECT:
				{
					Socket::disconnect(); 
					fire(BufferedSocketListener::FAILED, STRING(DISCONNECTED));

					for(int k = 0; k < BUFFERS; k++) {
						outbufPos[k] = 0;
					}
				}
				break;

			case SEND_FILE: if(isConnected()) threadSendFile(); break;
			case SEND_DATA: if(isConnected()) threadSendData(); break;
			case CONNECT: threadConnect(); break;
			case BIND: threadBind(); break;

			default: dcassert("BufferedSocket::threadRun: Unknown command received" == NULL);
			}
		}
		// Now check if there's any activity on the socket
		try {
			if(isConnected() && waitForData(200)) {
				threadRead();
			}
		} catch(SocketException e) {
			fail(e.getError());
		}
	}
}

/**
 * @file BufferedSocket.cpp
 * $Id: BufferedSocket.cpp,v 1.39 2002/05/03 18:52:59 arnetheduck Exp $
 */