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

#include "HttpConnection.h"

#include "SettingsManager.h"

/**
 * Downloads a file and returns it as a string
 * @todo Report exceptions
 * @todo Abort download
 * @param aUrl Full URL of file
 * @return A string with the content, or empty if download failed
 */
void HttpConnection::downloadFile(const string& aUrl) {
	dcassert(Util::findSubString(aUrl, "http://") == 0);
	currentUrl = aUrl;
	// reset all settings (as in constructor), moved here from onLine(302) because ok was not reset properly
	moved302 = false; 
	ok = false;
	size = -1;
	// set download type
	if(aUrl.substr(aUrl.size() - 4) == ".bz2") {
		fire(HttpConnectionListener::SET_DOWNLOAD_TYPE_BZIP2, this);
	} else {
		fire(HttpConnectionListener::SET_DOWNLOAD_TYPE_NORMAL, this);
	}

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
		socket = BufferedSocket::getSocket(0x0a);
	}
	socket->setNoproxy(true);
	socket->addListener(this);
	socket->connect(server, port);
}

void HttpConnection::onConnected() { 
	dcassert(socket); 
	socket->write("GET " + file + " HTTP/1.1\r\n"); 
	socket->write("User-Agent: " APPNAME " v" VERSIONSTRING "\r\n"); 

	string sRemoteServer = server; 
	if(!SETTING(HTTP_PROXY).empty()) 
	{ 
		string tfile;short tport; 
		Util::decodeUrl(file, sRemoteServer, tport, tfile); 
	} 
	socket->write("Host: " + sRemoteServer + "\r\n"); 
	socket->write("Connection: close\r\n");	// we'll only be doing one request
	socket->write("Cache-Control: no-cache\r\n\r\n"); 
} 

void HttpConnection::onLine(const string& aLine) {
	if(!ok) {
		if(aLine.find("200") == string::npos) {
			if(aLine.find("302") != string::npos){
				moved302 = true;
			} else {
				socket->removeListener(this);
				socket->disconnect();
				BufferedSocket::putSocket(socket);
				socket = NULL;
				fire(HttpConnectionListener::FAILED, this, aLine + " (" + currentUrl + ")");
				return;
			}
		}
		ok = true;
	} else if(moved302 && aLine.find("Location") != string::npos){
		dcassert(socket);
		socket->removeListener(this);
		socket->disconnect();
		BufferedSocket::putSocket(socket);
		socket = NULL;

		string location302 = aLine.substr(10, aLine.length() - 11);
		// make sure we can also handle redirects with relative paths
		if(Util::strnicmp(location302.c_str(), "http://", 7) != 0) {
			if(location302[0] == '/') {
				Util::decodeUrl(currentUrl, server, port, file);
				string tmp = "http://" + server;
				if(port != 80)
					tmp += ':' + Util::toString(port);
				location302 = tmp + location302;
			} else {
				string::size_type i = currentUrl.rfind('/');
				dcassert(i != string::npos);
				location302 = currentUrl.substr(0, i + 1) + location302;
			}
		}
		fire(HttpConnectionListener::REDIRECTED, this, location302);

		downloadFile(location302); 		
	} else if(aLine == "\x0d") {
		socket->setDataMode(size);
	} else if(Util::findSubString(aLine, "Content-Length") != string::npos) {
		size = Util::toInt(aLine.substr(16, aLine.length() - 17));
	} else if(Util::findSubString(aLine, "Content-Encoding") != string::npos) {
		if(aLine.substr(18, aLine.length() - 19) == "x-bzip2")
			fire(HttpConnectionListener::SET_DOWNLOAD_TYPE_BZIP2, this);            
	}
}

// BufferedSocketListener
void HttpConnection::onAction(BufferedSocketListener::Types type) throw() {
	switch(type) {
	case BufferedSocketListener::CONNECTED:
		onConnected(); break;
	default:
		break;
	}
}
void HttpConnection::onAction(BufferedSocketListener::Types type, const string& aLine) throw() {
	switch(type) {
	case BufferedSocketListener::LINE:
		onLine(aLine); break;
	case BufferedSocketListener::FAILED:
		socket->removeListener(this);
		BufferedSocket::putSocket(socket);
		socket = NULL;
		fire(HttpConnectionListener::FAILED, this, aLine + " (" + currentUrl + ")"); break;
	default:
		break;
	}
}
void HttpConnection::onAction(BufferedSocketListener::Types type, int /*mode*/) throw() {
	switch(type) {
	case BufferedSocketListener::MODE_CHANGE:
		socket->removeListener(this);
		socket->disconnect();
		BufferedSocket::putSocket(socket);
		socket = NULL;
		fire(HttpConnectionListener::COMPLETE, this, currentUrl); 
		break;
	default:
		dcasserta(0);
	}
}
void HttpConnection::onAction(BufferedSocketListener::Types type, const u_int8_t* aBuf, int aLen) throw() {
	switch(type) {
	case BufferedSocketListener::DATA:
		fire(HttpConnectionListener::DATA, this, aBuf, aLen); break;
	default:
		dcasserta(0);
	}
}

/**
 * @file
 * $Id: HttpConnection.cpp,v 1.20 2003/11/11 20:31:56 arnetheduck Exp $
 */

