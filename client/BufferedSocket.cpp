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

static const int BUFSIZE = 1024;

void BufferedSocket::accept(const ServerSocket& aSocket) {
	Socket::accept(aSocket);
	startReader();
}


DWORD WINAPI BufferedSocket::reader(void* p) {
	ATLASSERT(p);
	
	BufferedSocket* bs = (BufferedSocket*) p;
	
	HANDLE h[2];
	
	h[0] = bs->stopEvent;
	
	try {
		if(!bs->isConnected())
			bs->Socket::connect(bs->server, bs->port);
		h[1] = bs->getReadEvent();
	} catch (SocketException e) {
		bs->fireError(e.getError());
		return 0;
	}

	string line = "";
	BYTE* buf = new BYTE[BUFSIZE];

	bs->fireConnected();
	
	while(WaitForMultipleObjects(2, h, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) {
		try {
			dcdebug("Available bytes: %d\n", bs->getAvailable());
			int i = bs->read(buf, BUFSIZE);
			string l;
			switch(bs->mode) {
			case MODE_LINE:
				l = string((char*)buf, i);
				line = line + l;
				while(line.find(bs->separator) != string::npos) {
					
					string tmp = line.substr(0, line.find('|'));
					line = line.substr(line.find('|')+1);
					
					bs->fireLine(tmp);
				}
				break;
			case MODE_DATA:
				if((bs->dataBytes - i)<0) {
					bs->fireData(buf, bs->dataBytes);
					i = i - bs->dataBytes;
					line = line + string((char*)buf+bs->dataBytes, i);
					while(line.find(bs->separator) != string::npos) {
						
						string tmp = line.substr(0, line.find('|'));
						line = line.substr(line.find('|')+1);
						
						bs->fireLine(tmp);
					}

					bs->dataBytes = 0;
					bs->mode = MODE_LINE;
					bs->fireModeChange(bs->mode);
				} else {
					bs->fireData(buf, i);
					bs->dataBytes-=i;
					dcassert(bs->dataBytes >= 0);
					if(bs->dataBytes == 0) {
						bs->mode = MODE_LINE;
						bs->fireModeChange(bs->mode);
					}
				}
				break;
			default:
				dcassert(0);
			}
		} catch(SocketException e) {
			bs->fireError(e.getError());
			return 0;
		}
	}
	return 0;
}

/**
 * @file BufferedSocket.cpp
 * $Id: BufferedSocket.cpp,v 1.2 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: BufferedSocket.cpp,v $
 * Revision 1.2  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.1  2001/11/24 10:39:00  arnetheduck
 * New BufferedSocket creates reader threads and reports inbound data through a listener.
 *
 * @endif
 */