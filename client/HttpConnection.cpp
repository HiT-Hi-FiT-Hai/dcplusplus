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
	dcassert(Util::findSubString(aUrl, "http://") == 0);
	
	if(SETTING(HTTP_PROXY).empty()) {
		Util::decodeUrl(aUrl, server, port, file);
		if(file.empty())
			file = "/";
	} else {
		Util::decodeUrl(SETTING(HTTP_PROXY), server, port, file);
		file = aUrl;
	}

	if(port == 0)
		port = 80;
	
	if(!socket) {
		socket = BufferedSocket::getSocket();
		socket->addListener(this);
	}
	socket->connect(server, port);
}

void HttpConnection::onConnected() {
	dcassert(socket);
	socket->write("GET " + file + " HTTP/1.1\r\n");
	socket->write("User-Agent: DC++ v" VERSIONSTRING "\r\n");
	socket->write("Host: " + server + "\r\n");
	socket->write("Cache-Control: no-cache\r\n\r\n");
}

void HttpConnection::onLine(const string& aLine) {
	if(!ok) {
		if(aLine.find("200") == string::npos) {
			socket->disconnect();
			fire(HttpConnectionListener::FAILED, this, aLine);
		}
		ok = true;
	} else if(aLine == "\x0d") {
		socket->setDataMode(size);
	} else if(aLine.find("Content-Length") != string::npos) {
		size = Util::toInt(aLine.substr(16, aLine.length() - 17));
	}
}

/**
 * @file HttpConnection.cpp
 * $Id: HttpConnection.cpp,v 1.10 2002/04/13 12:57:22 arnetheduck Exp $
 */

