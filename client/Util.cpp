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

#include "Util.h"

string Util::emptyString;

/**
 * Decodes a URL the best it can...
 * Recognised protocols:
 * http:// -> port 80
 * dchub:// -> port 411
 *
 * Any unknown fields are set to 0 or ""
 */

void Util::decodeUrl(const string& aUrl, string& aServer, short& aPort, string& aFile) {
	// First, check for a protocol: xxxx://
	string::size_type i;
	string url = aUrl;

	aServer = "";
	aPort = 0;
	aFile = "";

	if( (i=url.find("://")) != string::npos) {
		// Protocol found
		string protocol = aUrl.substr(0, i);
		url = url.substr(i+3);
		if(protocol == "http") {
			aPort = 80;
		} else if(protocol == "dchub") {
			aPort = 411;
		}
	}

	if( (i=url.find('/')) != string::npos) {
		// We have a filename...
		aFile = url.substr(i);
		url = url.substr(0, i);
	}

	if( (i=url.find(':')) != string::npos) {
		// Port
		aPort = atoi(aUrl.substr(i+1).c_str());
		url = url.substr(0, i);
	}

	// Only the server should be left now...
	aServer = url;
}



/**
 * @file Util.cpp
 * $Id: Util.cpp,v 1.3 2002/01/13 22:50:48 arnetheduck Exp $
 * @if LOG
 * $Log: Util.cpp,v $
 * Revision 1.3  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.2  2001/12/07 20:03:28  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.1  2001/12/02 11:18:10  arnetheduck
 * Added transfer totals and speed...
 *
 * @endif
 */

