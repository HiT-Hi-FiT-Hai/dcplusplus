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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "BufferedSocket.h"

#include "TimerManager.h"
#include "CryptoManager.h"

#include "File.h"

#define SMALL_BUFFER_SIZE 1024

// Polling is used for tasks...should be fixed...
#define POLL_TIMEOUT 250

/**
 * Send a chunk of a file
 * @return True if file is finished, false if there's more data to send
 */
bool BufferedSocket::threadSendFile() {
	u_int32_t len;
	dcassert(file != NULL);

	try {
		if(compress) {
			if(comp == NULL) {
				comp = new ZCompressor(*file, size);
			}
			u_int32_t s = (u_int32_t)min(size, (int64_t) (BOOLSETTING(SMALL_SEND_BUFFER) ? SMALL_BUFFER_SIZE : inbufSize));
			u_int32_t bytes;
			while(true) {
				{
					Lock l(cs);
					if(!tasks.empty())
						return false;
				}
				
				if(wait(0, WAIT_READ) & WAIT_READ)
					return false;

				u_int32_t br = 0;
				bytes = comp->compress(inbuf, s, br);
				if(bytes == (u_int32_t)-1) {
					// Finished!
					delete comp;
					comp = NULL;
					dcassert(size == 0);
					fire(BufferedSocketListener::TRANSMIT_DONE);
					return true;
				} else {
					if(bytes != 0){
						Socket::write((char*) inbuf, bytes);
					}
					if(br > 0) {
						fire(BufferedSocketListener::BYTES_SENT, br, bytes);
						size -= br;
						dcassert(size >= 0);
					}
				}
			}
		} else {
			while(size > 0) {
				{
					Lock l(cs);
					if(!tasks.empty())
						return false;
				}

				if(wait(0, WAIT_READ) & WAIT_READ)
					return false;

				dcassert(inbufSize >= SMALL_BUFFER_SIZE);
				u_int32_t s = (u_int32_t)min(size, (int64_t) (BOOLSETTING(SMALL_SEND_BUFFER) ? SMALL_BUFFER_SIZE : inbufSize));

				if( (len = file->read(inbuf, s)) == 0) {
					// Premature EOF?
					dcdebug("BufferedSocket::threadSendFile Read returned 0!!!");
					disconnect();
					return true;
				}
				Socket::write((char*)inbuf, len);
				fire(BufferedSocketListener::BYTES_SENT, len, len);
				size -= len;
			}
		}
	} catch(const Exception& e) {
		if(comp) {
			delete comp;
			comp = NULL;
		}
		fail(e.getError());
		return true;
	}
	fire(BufferedSocketListener::TRANSMIT_DONE);
	return true;
}

bool BufferedSocket::fillBuffer(char* buf, int bufLen, u_int32_t timeout /* = 0 */) throw(SocketException) {
	dcassert(buf != NULL);
	dcassert(bufLen > 0);
	
	int bytesIn = 0;
	int start = GET_TICK();

	while(bytesIn < bufLen) {
		while(!wait(POLL_TIMEOUT, WAIT_READ)) {
			{
				Lock l(cs);
				if(!tasks.empty()) {
					// We don't want to handle tasks here...
					Socket::disconnect();
					return false;
				}
			}
			if((timeout != 0) && ((start + timeout) < GET_TICK())) {
				// Connection timeout
				throw SocketException(STRING(CONNECTION_TIMEOUT));
			}
		}
		dcassert(bufLen > bytesIn);
		int x = Socket::read(buf + bytesIn, bufLen - bytesIn);
		if(x <= 0) {
			// ???
			throw SocketException(STRING(CONNECTION_CLOSED));
		}
		bytesIn += x;
	}
	
	return true;
}

void BufferedSocket::threadConnect() {
	dcdebug("threadConnect()\n");

	fire(BufferedSocketListener::CONNECTING);

	u_int32_t start = GET_TICK();
	string s;
	short p;
	{
		Lock l(cs);

		s=address;
		p=port;
	}

	try {
		setBlocking(false);
		Socket::create();

		if( !getNoproxy() && SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_SOCKS5) {
			if(!BOOLSETTING(SOCKS_RESOLVE)) {
				s = resolve(s);
			}
			Socket::connect(SETTING(SOCKS_SERVER), (short)SETTING(SOCKS_PORT));
		} else {
			Socket::connect(s, p);
		}
		
		while(!wait(POLL_TIMEOUT, WAIT_CONNECT)) {
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
				fail(STRING(CONNECTION_TIMEOUT));
				return;
			}
		}

		// Hm, let's see if we're socksified...
		if( !getNoproxy() && SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_SOCKS5) {
			if(SETTING(SOCKS_USER).empty() && SETTING(SOCKS_PASSWORD).empty()) {
				// No username and pw, easier...=)
				char connStr[3];
				connStr[0] = 5;			// SOCKSv5
				connStr[1] = 1;			// 1 method
				connStr[2] = 0;			// Method 0: No auth...

				Socket::write(connStr, 3);
				
				if(!fillBuffer(connStr, 2, 30000))
					return;
				
				if(connStr[1] != 0) {
					fail(STRING(SOCKS_NEEDS_AUTH));
					return;
				}				
			} else {
				// We try the username and password auth type (no, we don't support gssapi)
				u_int8_t ulen = (u_int8_t)(SETTING(SOCKS_USER).length() & 0xff);
				u_int8_t plen = (u_int8_t)(SETTING(SOCKS_PASSWORD).length() & 0xff);
				AutoArray<u_int8_t> connStr(3 + ulen + plen);

				connStr[0] = 5;			// SOCKSv5
				connStr[1] = 1;			// 1 method
				connStr[2] = 2;			// Method 2: Name/Password...
				Socket::write((char*)(u_int8_t*)connStr, 3);
				if(!fillBuffer((char*)(u_int8_t*)connStr, 2, 30000))
					return;
				if(connStr[1] != 2) {
					fail(STRING(SOCKS_AUTH_UNSUPPORTED));
					return;
				}
				// Now we send the username / pw...
				connStr[0] = 1;
				connStr[1] = ulen;
				strncpy((char*)(u_int8_t*)connStr + 2, SETTING(SOCKS_USER).c_str(), ulen);
				connStr[2 + ulen] = plen;
				strncpy((char*)(u_int8_t*)connStr + 3 + ulen, SETTING(SOCKS_PASSWORD).c_str(), plen);
				Socket::write((char*)(u_int8_t*)connStr, 3 + plen + ulen);

				if(!fillBuffer((char*)(u_int8_t*)connStr, 2, 30000)) {
					return;
				}

				if(connStr[1] != 0) {
					fail(STRING(SOCKS_AUTH_FAILED));
					return;
				}
				
			}

			// Alrite, let's get on with it...
			AutoArray<u_int8_t> connStr(10 + s.length());
			int connLen;
			connStr[0] = 5;			// SOCKSv5
			connStr[1] = 1;			// Connect
			connStr[2] = 0;			// Reserved
			
			if(BOOLSETTING(SOCKS_RESOLVE)) {
				u_int8_t slen =(u_int8_t)(s.length() & 0xff);
				connStr[3] = 3;		// Address type: domain name
				connStr[4] = slen;
				strncpy((char*)(u_int8_t*)connStr + 5, s.c_str(), slen);
				*((u_int16_t*)(&connStr[5 + slen])) = htons(p);
				connLen = 7 + slen;
			} else {
				connStr[3] = 1;		// Address type: IPv4;
				*((long*)(&connStr[4])) = inet_addr(s.c_str());
				*((u_int16_t*)(&connStr[8])) = htons(p);	
				connLen = 10;
			}
			
			Socket::write((char*)(u_int8_t*)connStr, connLen);
			// We assume we'll get a ipv4 address back...therefore, 10 bytes...if not, things
			// will break, but hey...noone's perfect (and I'm tired...)...
			if(!fillBuffer((char*)(u_int8_t*)connStr, 10, 30000)) {
				return;
			}

			if(connStr[0] != 5 || connStr[1] != 0) {
				fail(STRING(SOCKS_FAILED));
				return;
			}

			// Yihaa!
		}

		// We're connected! Clear the buffers...
		for(int k = 0; k < BUFFERS; k++) {
			outbufPos[k] = 0;
		}
		line.clear();
		setBlocking(true);

		fire(BufferedSocketListener::CONNECTED);
	} catch(const SocketException& e) {
		if(!getNoproxy() && SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_SOCKS5) {
			fail("Socks5: " + e.getError());
		} else {
			fail(e.getError());
		}
		return;
	}
}	

void BufferedSocket::threadRead() {
	try {
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
		while(i > 0) {
			if(mode == MODE_LINE) {
				string::size_type pos;

				l = string((char*)inbuf + bufpos, i);

				if( (pos = l.find(separator)) != string::npos) {
					if(!line.empty()) {
						fire(BufferedSocketListener::LINE, line + l.substr(0, pos));
						line.clear();
					} else {
						fire(BufferedSocketListener::LINE, l.substr(0, pos));
					}
					i-=(pos + sizeof(separator));
					bufpos += (pos + sizeof(separator));
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
						mode = MODE_LINE;
						fire(BufferedSocketListener::MODE_CHANGE, MODE_LINE);
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

void BufferedSocket::write(const char* aBuf, int aLen) throw() {
	{
		Lock l(cs);
		int newSize = outbufSize[curBuf];
		
		while(newSize < (aLen + outbufPos[curBuf])) {
			newSize *= 2;
		}

		if(newSize > outbufSize[curBuf]) {
			// Need to grow...
			dcdebug("Growing outbuf[%d] to %d bytes\n", curBuf, newSize);
			u_int8_t* tmp = new u_int8_t[newSize];
			memcpy(tmp, outbuf[curBuf], outbufPos[curBuf]);
			delete[] outbuf[curBuf];
			outbuf[curBuf] = tmp;
			outbufSize[curBuf] = newSize;
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
int BufferedSocket::run() {
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
				case SHUTDOWN: threadShutDown(); return 0;
				case DISCONNECT: if(isConnected()) fail(STRING(DISCONNECTED)); break;
				case SEND_FILE: if(isConnected()) sendingFile = true; break;
				case SEND_DATA: dcassert(!sendingFile); if(isConnected()) threadSendData(); break;
				case CONNECT: sendingFile = false; threadConnect(); break;
				case ACCEPTED: break;

				default: dcassert("BufferedSocket::threadRun: Unknown command received" == NULL);
				}
			}

			// Now check if there's any activity on the socket
			if(isConnected()) {
				int waitFor = wait(POLL_TIMEOUT, sendingFile ? WAIT_READ | WAIT_WRITE : WAIT_READ);
				if(waitFor & WAIT_WRITE) {
					dcassert(sendingFile);
					if(threadSendFile())
						sendingFile = false;
				}
				if(waitFor & WAIT_READ) {
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
 * $Id: BufferedSocket.cpp,v 1.59 2003/11/21 17:00:53 arnetheduck Exp $
 */
