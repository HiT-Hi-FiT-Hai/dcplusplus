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
	Socket::accept(aSocket);

	dcdebug("Socket accepted\n");
	startReader();
}

/**
 * Write a file to the socket, stops reading from the socket until the transfer's finished.
 * @return True if everythings ok and the thread should continue reading, false on error
 */
bool BufferedSocket::writer(BufferedSocket* bs) {
	DWORD len;

	HANDLE h = bs->readerEvent;
	File* file = bs->file;
	dcassert(file != NULL);
	
	if(!bs->isConnected()) {
		bs->fire(BufferedSocketListener::FAILED, "Not connected");
		return false;
	}

	while(WaitForSingleObject(h, 0) == WAIT_TIMEOUT) {
		try {
			if( (len = file->read(bs->inbuf, bs->inbufSize)) == 0) {
				bs->fire(BufferedSocketListener::TRANSMIT_DONE);
				return true;
			}
			bs->write((char*)bs->inbuf, len);
			bs->fire(BufferedSocketListener::BYTES_SENT, len);
		} catch(Exception e) {
			dcdebug("BufferedSocket::Writer caught: %s\n", e.getError().c_str());
			bs->fire(BufferedSocketListener::FAILED, e.getError());
			return false;
		}
	}
	// Hm, to fire or not to fire, that is the question...
//	fire(BufferedSocketListener::FAILED, "File not finished");
	return false;
}

DWORD WINAPI BufferedSocket::reader(void* p) {
	ATLASSERT(p);
	
	BufferedSocket* bs = (BufferedSocket*) p;
	
	HANDLE h[3];
	
	h[0] = bs->readerEvent;
	h[1] = bs->writerEvent;

	try {
		if(!bs->isConnected()) {
			bs->create();
			h[2] = bs->getEvent();

			bs->Socket::connect(bs->server, bs->port);
			switch(WaitForMultipleObjects(3, h, FALSE, 10000)) {
			case WAIT_TIMEOUT:
				bs->disconnect();
				bs->fire(BufferedSocketListener::FAILED, "Connection Timeout");
				return 0x01;
			case WAIT_OBJECT_0:
				bs->disconnect();
				dcdebug("BufferedSocket::reader stopped while connecting\n");
				return 0x00;
			case WAIT_OBJECT_0 + 1:
				// Ignore it?
				dcdebug("BufferedSocket::reader Writer event while connecting!\n");
				bs->disconnect();
				bs->fire(BufferedSocketListener::FAILED, "Not connected");
				return 0x02;
			case WAIT_OBJECT_0 + 2:
				// We're connected!
				break;
			case WAIT_FAILED:
				bs->disconnect();
				bs->fire(BufferedSocketListener::FAILED, "Connection failed");
				dcdebug("BufferedSocket::reader Wait failed (0x%x)\n", GetLastError());
				return 0x03;
			default:
				bs->disconnect();
				bs->fire(BufferedSocketListener::FAILED, "Connection failed");
				dcdebug("BufferedSocket::reader Unknown Wait response (0x%x)\n", GetLastError());
				return 0x04;
			}
				
			bs->fire(BufferedSocketListener::CONNECTED);
		} else {
			h[2] = bs->getEvent();
		}
	} catch (SocketException e) {
		dcdebug("BufferedSocket::Reader caught: %s\n", e.getError().c_str());
		bs->disconnect();
		bs->fire(BufferedSocketListener::FAILED, e.getError());
		return 0x05;
	}

	string line;
	if(bs->inbuf == NULL) {
		bs->inbuf = new BYTE[bs->inbufSize];
	}

	while(true) {
		switch( WaitForMultipleObjects(3, h, FALSE, INFINITE) ) {
		case WAIT_OBJECT_0:				// readerEvent, time to stop
			dcdebug("BufferedSocket::reader stopped\n");
			return 0;
		case WAIT_OBJECT_0 + 1:			// writerEvent, send the file
			dcdebug("BufferedSocket::reader Writer event\n");
			if(!writer(bs)) {
				return 0x06;
			}
			break;
		case WAIT_OBJECT_0 + 2:

			try {
				//dcdebug("Available bytes: %d\n", bs->getAvailable());
				if(bs->getAvailable() > bs->inbufSize) {
					// We grow the buffer according to how much is actually available...
					delete bs->inbuf;
					bs->inbufSize *= 2;
					bs->inbuf = new BYTE[bs->inbufSize];
					dcdebug("BufferedSocket::reader: Grown %p's buffer to %d\n", bs, bs->inbufSize);
				}
				int i = bs->read(bs->inbuf, bs->inbufSize);
				if(i == -1) {
					// EWOULDBLOCK, no data recived, most probably a read event...
					continue;
				} else if(i == 0) {
					// This socket has been closed...
					bs->disconnect();
					bs->fire(BufferedSocketListener::FAILED, "Disconnected");
					return 0x07;
				}
				int bufpos = 0;
				string l;

				if(bs->mode == MODE_LINE) {
					// We might as well create the string, at least one part will be sent as a string...
					l = string((char*)bs->inbuf, i);
				}
				
				while(i) {
					if(bs->mode == MODE_LINE) {
						string::size_type pos;
						if( (pos = l.find(bs->separator)) != string::npos) {
							bufpos += sizeof(bs->separator) + pos;
							if(!line.empty()) {
								bs->fire(BufferedSocketListener::LINE, line + l.substr(0, pos));
								line = "";
							} else {
								bs->fire(BufferedSocketListener::LINE, l.substr(0, pos));
							}
							l = l.substr(pos+1);
							i-=(pos+1);
						} else {
							line += l;
							i = 0;
						}
					} else if(bs->mode == MODE_DATA) {
						if(bs->dataBytes == -1) {
							bs->fire(BufferedSocketListener::DATA, bs->inbuf+bufpos, i);
							i = 0;
						} else {
							int high = bs->dataBytes < i ? bs->dataBytes - i : i;
							bs->fire(BufferedSocketListener::DATA, bs->inbuf+bufpos, high);
							i-=high;
							
							bs->dataBytes -= high;
							if(bs->dataBytes == 0) {
								bs->fire(BufferedSocketListener::MODE_CHANGE, MODE_LINE);
								bs->mode = MODE_LINE;
							}
						}
					} else if(bs->mode == MODE_DGRAM) {
						bs->fire(BufferedSocketListener::DATA, bs->inbuf, i);
						i = 0;
					}
				}
			} catch(SocketException e) {
				dcdebug("BufferedSocket::Reader caught(2): %s\n", e.getError().c_str());
				// Ouch...
				bs->disconnect();
				bs->fire(BufferedSocketListener::FAILED, e.getError());
				return 0x08;
			}
			break;
		case WAIT_FAILED:
			// Duuhhh???
			dcdebug("BufferedSocket::reader Wait failed (%x)\n", GetLastError());
			bs->disconnect();
			bs->fire(BufferedSocketListener::FAILED, "Disconnected");
			return 0x09;
		default:
			dcassert(0);
		}
	}
	return 0;
}

/**
 * @file BufferedSocket.cpp
 * $Id: BufferedSocket.cpp,v 1.24 2002/01/20 22:54:45 arnetheduck Exp $
 * @if LOG
 * $Log: BufferedSocket.cpp,v $
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