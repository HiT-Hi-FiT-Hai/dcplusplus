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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "BufferedSocket.h"
#include "File.h"

void BufferedSocket::accept(const ServerSocket& aSocket) {
	if(isConnected()) {
		Socket::disconnect();
	}
	Socket::accept(aSocket);

	dcdebug("Socket accepted\n");
}

/**
 * Write a file to the socket, stops reading from the socket until the transfer's finished.
 * @return True if everythings ok and the thread should continue reading, false on error
 */
void BufferedSocket::threadSendFile() {
	DWORD len;
	dcassert(file != NULL);
	
	if(!isConnected()) {
		fire(BufferedSocketListener::FAILED, "Not connected");
		return;
	}
	{
		Lock l(cs);
		for(deque<Tasks>::iterator i = tasks.begin(); i != tasks.end(); ++i) {
			if(*i == SHUTDOWN) {
				return;
			} else if(*i == DISCONNECT){
				// Let threadrun handle it...
				return;
			} else {
				// Should never happen...
				dcassert("Bad tasks in BufferedSocket after SendFile" == NULL);
			}
		}
	}

	while(WaitForSingleObject(commandEvent, 0) == WAIT_TIMEOUT) {
		try {

			if( (len = file->read(inbuf, inbufSize)) == 0) {
				fire(BufferedSocketListener::TRANSMIT_DONE);
				return;
			}
			Socket::write((char*)inbuf, len);
			fire(BufferedSocketListener::BYTES_SENT, len);
		} catch(Exception e) {
			dcdebug("BufferedSocket::Writer caught: %s\n", e.getError().c_str());
			fire(BufferedSocketListener::FAILED, e.getError());
			return;
		}
	}
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

bool BufferedSocket::threadConnect() {
	HANDLE h[2];
	string s;
	short p;

	{
		Lock l(cs);

		// Check the task queue that we don't have a pending shutdown / disconnect...
		for(deque<Tasks>::iterator i = tasks.begin(); i != tasks.end(); ++i) {
			if(*i == SHUTDOWN) {
				return false;
			} else if(*i == DISCONNECT){
				// Let threadrun handle it...
				return false;
			} else {
				// Should never happen...
				dcassert("Bad tasks in BufferedSocket after SendFile" == NULL);
			} 
		}
		
		s = server;
		p = port;
	}

	try {
		Socket::create();
		h[0] = commandEvent;
		h[1] = getEvent();
		
		Socket::connect(s, p);

		switch(WaitForMultipleObjects(2, h, FALSE, 10000)) {
		case WAIT_TIMEOUT:
			Socket::disconnect();
			fire(BufferedSocketListener::FAILED, "Connection Timeout");
			return false;
		case WAIT_OBJECT_0:
			dcdebug("BufferedSocket::threadConnect: Received task\n");
			{
				Lock l(cs);
				
				// Check the task queue that we don't have a pending shutdown / disconnect...
				for(deque<Tasks>::iterator i = tasks.begin(); i != tasks.end(); ++i) {
					if(*i == SHUTDOWN) {
						return false;
					} else if(*i == DISCONNECT){
						// Let threadrun handle it...
						return false;
					} else {
						// Should never happen...
						dcassert("Bad tasks in BufferedSocket after SendFile" == NULL);
					} 
				}
				
			}
			return 0x00;
		case WAIT_OBJECT_0 + 1:
			// We're connected!
			fire(BufferedSocketListener::CONNECTED);
			return true;
		default: dcassert("BufferedSocket::threadRun: Unknown command received" == NULL);
		}
	} catch(SocketException e) {
		dcdebug("BufferedSocket::threadConnect caught: %s\n", e.getError().c_str());
		// Release all resources
		fail(e.getError());
	}

	// ?
	return false;
}	


void BufferedSocket::threadRead() {
	try {
		//dcdebug("Available bytes: %d\n", bs->getAvailable());
		while(getAvailable() > inbufSize) {
			// We grow the buffer according to how much is actually available...
			delete inbuf;
			inbufSize *= 2;
			inbuf = new BYTE[inbufSize];
			dcdebug("BufferedSocket::reader: Grown %p's buffer to %d\n", this, inbufSize);
		}
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
		
		if(mode == MODE_LINE) {
			// We might as well create the string, at least one part will be sent as a string...
			l = string((char*)inbuf, i);
		}
		
		while(i) {
			if(mode == MODE_LINE) {
				string::size_type pos;
				if( (pos = l.find(separator)) != string::npos) {
					bufpos += sizeof(separator) + pos;
					if(!line.empty()) {
						fire(BufferedSocketListener::LINE, line + l.substr(0, pos));
						line = "";
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
					i = 0;
				} else {
					int high = dataBytes < i ? dataBytes - i : i;
					fire(BufferedSocketListener::DATA, inbuf+bufpos, high);
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
	
	int pos = 0;
	try {
		BYTE* buf;
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

void BufferedSocket::write(const char* aBuf, int aLen) throw(SocketException) {
	int pos = 0;

	if(!isConnected()) {
		throw("Not connected");
	}
	{
		Lock l(cs);
		int mybuf = curBuf;
		
		while(outbufSize[mybuf] < (aLen + outbufPos[mybuf])) {
			// Need to grow...
			outbufSize[mybuf]*=2;
			dcdebug("Growing outbuf[%d] to %d bytes\n", mybuf, outbufSize[mybuf]);
			BYTE* tmp = new BYTE[outbufSize[mybuf]];
			memcpy(tmp, outbuf[mybuf], outbufPos[mybuf]);
			delete outbuf[mybuf];
			outbuf[mybuf] = tmp;
		}

		memcpy(outbuf[mybuf] + outbufPos[mybuf], aBuf, aLen);
		outbufPos[mybuf] += aLen;
		addTask(SEND_DATA);
	}
}

void BufferedSocket::threadRun() {
	
	HANDLE h[2];
	
	h[0] = commandEvent;

	int handles = 1;
	int i = 0;
	while(true) {
		handles = isConnected() ? 2 : 1;
		switch( (i = WaitForMultipleObjects(handles, h, FALSE, INFINITE))) {
		case WAIT_OBJECT_0:
			{
				while(true) {
					Tasks t;
					{
						Lock l(cs);
						if(tasks.size() == 0)
							break;
						t = tasks.front();
						tasks.pop_front();
					}
					switch(t) {
					case SHUTDOWN: threadShutDown(); return;
					case DISCONNECT:
						{
							Socket::disconnect(); 
							fire(BufferedSocketListener::FAILED, "Disconnected");
							handles = 1;
							
							for(int k = 0; k < BUFFERS; k++) {
								outbufPos[k] = 0;
							}
						}
						break;

					case SEND_FILE: threadSendFile(); break;
					case SEND_DATA: threadSendData(); break;
					case CONNECT:
						if(threadConnect()) {
							h[1] = getEvent();
						}
						break;
						
					case BIND:
						if(threadBind()) {
							h[1] = getEvent();
						}
						break;
						
					default: dcassert("BufferedSocket::threadRun: Unknown command received" == NULL);
					}
				}
			}
			break;
		case WAIT_OBJECT_0 + 1: threadRead(); break;
		default: dcassert("Bad return value from WaitForMultipleObjects" == NULL);

		}
	}
}

/**
 * @file BufferedSocket.cpp
 * $Id: BufferedSocket.cpp,v 1.25 2002/02/06 12:29:06 arnetheduck Exp $
 * @if LOG
 * $Log: BufferedSocket.cpp,v $
 * Revision 1.25  2002/02/06 12:29:06  arnetheduck
 * New Buffered socket handling with asynchronous sending (asynchronous everything really...)
 *
 * Revision 1.24  2002/01/20 22:54:45  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.23  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.22  2002/01/11 14:52:56  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.21  2002/01/06 21:55:20  arnetheduck
 * Some minor bugs fixed, but there remains one strange thing, the reconnect
 * button doesn't work...
 *
 * Revision 1.20  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.19  2001/12/21 23:52:30  arnetheduck
 * Last commit for five days
 *
 * Revision 1.18  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.17  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.16  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.15  2001/12/12 00:06:04  arnetheduck
 * Updated the public hub listings, fixed some minor transfer bugs, reworked the
 * sockets to use only one thread (instead of an extra thread for sending files),
 * and fixed a major bug in the client command decoding (still have to fix this
 * one for the userconnections...)
 *
 * Revision 1.14  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.13  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.12  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.11  2001/12/07 20:03:00  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.10  2001/12/05 19:40:13  arnetheduck
 * More bugfixes.
 *
 * Revision 1.9  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
 * Revision 1.8  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.7  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
 *
 * Revision 1.6  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.5  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.4  2001/11/29 19:10:54  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.3  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
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