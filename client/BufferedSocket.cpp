/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

static const int BUFSIZE = 4096;

void BufferedSocket::accept(const ServerSocket& aSocket) {
	Socket::accept(aSocket);

	dcdebug("Socket accepted\n");
	startReader();
}


DWORD WINAPI BufferedSocket::writer(void* p) {
	BufferedSocket* bs = (BufferedSocket*)p;
	BYTE buf[4096];
	DWORD len;

	HANDLE h = bs->writerEvent;
	HANDLE file = bs->file;

	if(!bs->isConnected()) {
		bs->fireError("Not connected");
		return 0x11;

	}

	while(WaitForSingleObject(h, 0) == WAIT_TIMEOUT) {
		if(ReadFile(file, buf, 4096, &len, NULL)) {
			if(len == 0) {
				bs->fireTransmitDone();
				return 0x10;
			}
			try {
				bs->write((char*)buf, len);
				bs->fireBytesSent(len);
			} catch(SocketException e) {
				dcdebug("BufferedSocket::Writer caught: %s\n", e.getError().c_str());
				bs->fireError(e.getError());
				return 0x12;
			}
		} else {
			bs->fireError("Error reading file");
			return 0x13;
		}
	}

	// Hm, to fire or not to fire, that is the question...
//	fireError("File not finished");
	return 0x14;
	
}

DWORD WINAPI BufferedSocket::reader(void* p) {
	ATLASSERT(p);
	
	BufferedSocket* bs = (BufferedSocket*) p;
	
	HANDLE h[2];
	
	h[0] = bs->readerEvent;
	
	try {
		if(!bs->isConnected()) {
			bs->create();
			h[1] = bs->getEvent();

			bs->Socket::connect(bs->server, bs->port);
			if(WaitForMultipleObjects(2, h, FALSE, 10000) != WAIT_OBJECT_0 + 1) {
				// Either timeout or window stopped...don't care which really...
				bs->disconnect();
				bs->fireError("Connection Timeout.");
				return 0x01;
			}
				
			bs->fireConnected();
		} else {
			h[1] = bs->getEvent();
		}
	} catch (SocketException e) {
		dcdebug("BufferedSocket::Reader caught: %s\n", e.getError().c_str());
		bs->disconnect();
		bs->fireError(e.getError());
		return 0x02;
	}

	string line = "";
	BYTE buf[BUFSIZE];
	
	while(WaitForMultipleObjects(2, h, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) {
		try {
			//dcdebug("Available bytes: %d\n", bs->getAvailable());
			int i = bs->read(buf, BUFSIZE);
			if(i == -1) {
				// EWOULDBLOCK, no data recived, most probably a read event...
				continue;
			} else if(i == 0) {
				// This socket has been closed...
				bs->disconnect();
				bs->fireError("Disconnected.");
				return 0x03;
			}
			int bufpos = 0;
			string l;

			if(bs->mode == MODE_LINE) {
				// We might as well create the string, at least one part will be sent as a string...
				l = string((char*)buf, i);
			}
			
			while(i) {
				if(bs->mode == MODE_LINE) {
					string::size_type pos;
					if( (pos = l.find(bs->separator)) != string::npos) {
						bufpos += sizeof(bs->separator) + pos;
						if(!line.empty()) {
							bs->fireLine(line + l.substr(0, pos));
							line = "";
						} else {
							bs->fireLine(l.substr(0, pos));
						}
						l = l.substr(pos+1);
						i-=(pos+1);
					} else {
						line += l;
						i = 0;
					}
				} else if(bs->mode == MODE_DATA) {
					if(bs->dataBytes == -1) {
						bs->fireData(buf+bufpos, i);
						i = 0;
					} else {
						int high = bs->dataBytes < i ? bs->dataBytes - i : i;
						bs->fireData(buf+bufpos, high);
						i-=high;
						
						bs->dataBytes -= high;
						if(bs->dataBytes == 0) {
							bs->fireModeChange(MODE_LINE);
							bs->mode = MODE_LINE;
						}
					}
				} else if(bs->mode == MODE_DGRAM) {
					bs->fireData(buf, i);
					i = 0;
				}
			}
		} catch(SocketException e) {
			dcdebug("BufferedSocket::Reader caught(2): %s\n", e.getError().c_str());
			// Ouch...
			bs->disconnect();
			bs->fireError(e.getError());
			bs->readerThread = NULL;
			return 0x04;
		}
	}
	return 0;
}

/**
 * @file BufferedSocket.cpp
 * $Id: BufferedSocket.cpp,v 1.14 2001/12/11 01:10:29 arnetheduck Exp $
 * @if LOG
 * $Log: BufferedSocket.cpp,v $
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