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

#include "Util.h"
#include "StringTokenizer.h"

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

string Util::emptyString;

bool Util::away = false;
string Util::awayMsg;
const string Util::defaultMsg = "I'm away. I might answer later if you're lucky. <DC++ v" VERSIONSTRING ">";
char Util::upper[256];
char Util::lower[256];

void Util::initialize() {
	for(int i = 0; i < 256; ++i) {
		upper[i] = (char)toupper(i);
		lower[i] = (char)tolower(i);
	}
}

/**
 * Decodes a URL the best it can...
 * Default ports:
 * http:// -> port 80
 * dchub:// -> port 411
 */
void Util::decodeUrl(const string& url, string& aServer, short& aPort, string& aFile) {
	// First, check for a protocol: xxxx://
	string::size_type i = 0, j, k;
	
	aServer = emptyString;
	aFile = emptyString;

	if( (j=url.find("://", i)) != string::npos) {
		// Protocol found
		string protocol = url.substr(0, j);
		i = j + 3;

		if(protocol == "http") {
			aPort = 80;
		} else if(protocol == "dchub") {
			aPort = 411;
		}
	}

	if( (j=url.find('/', i)) != string::npos) {
		// We have a filename...
		aFile = url.substr(j);
	}

	if( (k=url.find(':', i)) != string::npos) {
		// Port
		if(k < j)
			aPort = (short)Util::toInt(url.substr(k+1, j-k-1));
	} else {
		k = j;
	}

	// Only the server should be left now...
	aServer = url.substr(i, k-i);
}


string Util::getLocalIp() {
	string tmp;
	
	char buf[256];
	gethostname(buf, 255);
	hostent* he = gethostbyname(buf);
	if(he == NULL || he->h_addr_list[0] == 0)
		return Util::emptyString;
	sockaddr_in dest;
	int i = 0;
	
	// We take the first ip as default, but if we can find a better one, use it instead...
	memcpy(&(dest.sin_addr), he->h_addr_list[i++], he->h_length);
	tmp = inet_ntoa(dest.sin_addr);
	if( strncmp(tmp.c_str(), "192", 3) == 0 || 
		strncmp(tmp.c_str(), "169", 3) == 0 || 
		strncmp(tmp.c_str(), "127", 3) == 0 || 
		strncmp(tmp.c_str(), "10", 2) == 0 ) {
		
		while(he->h_addr_list[i]) {
			memcpy(&(dest.sin_addr), he->h_addr_list[i], he->h_length);
			string tmp2 = inet_ntoa(dest.sin_addr);
			if(	strncmp(tmp2.c_str(), "192", 3) != 0 &&
				strncmp(tmp2.c_str(), "169", 3) != 0 &&
				strncmp(tmp2.c_str(), "127", 3) != 0 &&
				strncmp(tmp2.c_str(), "10", 2) != 0) {
				
				tmp = tmp2;
			}
			i++;
		}
	}
	return tmp;
}

/**
* This function takes a sting and a set of parameters and transforms them accoring to
* a simple formatting rule, similar to strftime. In the message, every parameter should be
* represented by %[name]. It will then be replaced by the corresponding item in 
* the params stringmap. After that, the string is passed through strftime with the current
* date/time and then finally written to the log file. If the parameter is not present at all,
* it is removed from the string completely...
*/
string Util::formatParams(const string& msg, StringMap& params) {
	string result = msg;

	string::size_type i, j, k;
	i = 0;
	while (( j = result.find("%[", i)) != string::npos) {
		if( (result.size() < j + 2) || ((k = result.find(']', j + 2)) == string::npos) ) {
			break;
		}
		string name = result.substr(j + 2, k - j - 2);
		StringMapIter smi = params.find(name);
		if(smi == params.end()) {
			result.erase(j, k-j);
			i = j;
		} else {
			result.replace(j, k-j + 1, smi->second);
			i = j + smi->second.size();
		}
	}

	int bufsize = result.size() + 64;
	char* buf = new char[bufsize];
	time_t now = time(NULL);

	while(!strftime(buf, bufsize-1, result.c_str(), localtime(&now))) {
		delete buf;
		bufsize+=64;
		buf = new char[bufsize];
	}

	result = buf;
	delete buf;
	return result;
}

/**
 * @file Util.cpp
 * $Id: Util.cpp,v 1.18 2002/05/23 21:48:23 arnetheduck Exp $
 */

