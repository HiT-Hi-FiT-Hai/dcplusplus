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

#include "HttpConnection.h"

/**
 * Downloads a file and returns it as a string
 * @todo Windows dependency
 * @todo Report exceptions
 * @todo Abort download
 * @param aUrl Full URL of file
 * @return A string with the content, or empty if download failed
 */
void HttpConnection::downloadFile(const string& aUrl) {

	Util::decodeUrl(aUrl, server, port, file);

	if(file.empty())
		file = "/";

	if(port == 0)
		port = 80;
	

	socket.connect(server, port);
}

void HttpConnection::onConnected() {
	socket.write("GET " + file + " HTTP/1.1\r\n");
	socket.write("User-Agent: DC++\r\n");
	socket.write("Host: " + server + "\r\n");
	socket.write("Cache-Control: no-cache\r\n\r\n");
}

void HttpConnection::onLine(const string& aLine) {
	if(!ok) {
		if(aLine.find("200") == string::npos) {
			socket.disconnect();
			fire(HttpConnectionListener::FAILED, this, "File Not Available");
		}
		ok = true;
	} else if(aLine == "\x0d") {
		socket.setDataMode(size);
	} else if(aLine.find("Content-Length") != string::npos) {
		size = atoi(aLine.substr(16, aLine.length() - 17).c_str());
	}
}

/**
 * @file HttpConnection.cpp
 * $Id: HttpConnection.cpp,v 1.4 2002/01/11 14:52:57 arnetheduck Exp $
 * @if LOG
 * $Log: HttpConnection.cpp,v $
 * Revision 1.4  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.3  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.2  2001/12/07 20:03:06  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */

