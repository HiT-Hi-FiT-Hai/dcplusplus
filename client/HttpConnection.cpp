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

HttpConnection::HttpConnection()
{

}

HttpConnection::~HttpConnection()
{

}

/**
 * Downloads a file and returns it as a string
 * @todo Windows dependency
 * @todo Report exceptions
 * @todo Abort download
 * @param aUrl Full URL of file
 * @return A string with the content, or empty if download failed
 */
string HttpConnection::DownloadTextFile(const string& aUrl) {
	HINTERNET hInet = InternetOpen("DC++", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	HINTERNET hFile = InternetOpenUrl(hInet, aUrl.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, NULL);
	
	string tmp;
	char buf[10240];
	DWORD bytesRead;;
	while(InternetReadFile(hFile, buf, 10240, &bytesRead)) {
		if(bytesRead == 0)
			break;

		tmp = tmp + string(buf, bytesRead);
	}

	InternetCloseHandle(hFile);
	InternetCloseHandle(hInet);

	return tmp;
}

/**
 * @file HttpConnection.cpp
 * $Id: HttpConnection.cpp,v 1.1 2001/11/21 17:33:20 arnetheduck Exp $
 * @if LOG
 * $Log: HttpConnection.cpp,v $
 * Revision 1.1  2001/11/21 17:33:20  arnetheduck
 * Initial revision
 *
 * @endif
 */

