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

#include "Util.h"
#include "StringTokenizer.h"
#include "ResourceManager.h"

string Util::emptyString;
HBRUSH Util::bgBrush = NULL;
COLORREF Util::textColor = 0;
COLORREF Util::bgColor = 0;
HFONT Util::font;
CMenu Util::mainMenu;

bool Util::away = false;
string Util::awayMsg;
const string Util::defaultMsg = "I'm away. I might answer later if you're lucky. <DC++ v" VERSIONSTRING ">";

/**
 * Decodes a URL the best it can...
 * Default ports:
 * http:// -> port 80
 * dchub:// -> port 411
 */

void Util::decodeUrl(const string& url, string& aServer, short& aPort, string& aFile) {
	// First, check for a protocol: xxxx://
	string::size_type i = 0, j, k;
	
	aServer = "";
	aFile = "";

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

void Util::decodeFont(const string& setting, LOGFONT &dest) {
	StringTokenizer st(setting, ',');
	StringList &sl = st.getTokens();
	
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(dest), &dest);
	string face;
	if(sl.size() == 4)
	{
		face = sl[0];
		dest.lfHeight = Util::toInt(sl[1]);
		dest.lfWeight = Util::toInt(sl[2]);
		dest.lfItalic = (BYTE)Util::toInt(sl[3]);
	}

	if(!face.empty()) {
		::ZeroMemory(dest.lfFaceName, LF_FACESIZE);
		strcpy(dest.lfFaceName, face.c_str());
	}
}

string Util::getLocalIp() {
	string tmp;
	
	char buf[256];
	gethostname(buf, 256);
	hostent* he = gethostbyname(buf);
	if(he == NULL || he->h_addr_list[0] == 0)
		return "";
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

int CALLBACK Util::browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM /*lp*/, LPARAM pData) {
	switch(uMsg) {
	case BFFM_INITIALIZED: 
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
	}
	return 0;
}

bool Util::browseDirectory(string& target, HWND owner /* = NULL */) {
	char buf[MAX_PATH];
	BROWSEINFO bi;
	LPMALLOC ma;
	
	ZeroMemory(&bi, sizeof(bi));
	
	bi.hwndOwner = owner;
	bi.pszDisplayName = buf;
	bi.lpszTitle = "Choose folder";
	bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lParam = (LPARAM)target.c_str();
	bi.lpfn = &browseCallbackProc;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if(pidl != NULL) {
		SHGetPathFromIDList(pidl, buf);
		target = buf;
		
		if(target.size() > 0 && target[target.size()-1] != '\\')
			target+='\\';
		
		if(SHGetMalloc(&ma) != E_FAIL) {
			ma->Free(pidl);
			ma->Release();
		}
		return true;
	}
	return false;
}

bool Util::browseFile(string& target, HWND owner /* = NULL */, bool save /* = true */) {
	char buf[MAX_PATH];
	OPENFILENAME ofn;       // common dialog box structure
	
	memcpy(buf, target.c_str(), target.length() + 1);
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = sizeof(buf);
	ofn.Flags = OFN_PATHMUSTEXIST;
	
	// Display the Open dialog box. 
	if ( (save ? GetSaveFileName(&ofn) : GetOpenFileName(&ofn) ) ==TRUE) {
		target = ofn.lpstrFile;
		return true;
	}
	return false;
}

void Util::buildMenu() {
	mainMenu.CreateMenu();
	
	CMenuHandle file;
	file.CreatePopupMenu();
	
	file.AppendMenu(MF_STRING, IDC_QUEUE, CSTRING(MENU_FILE_DOWNLOAD_QUEUE));
	file.AppendMenu(MF_STRING, ID_FILE_CONNECT, CSTRING(MENU_FILE_PUBLIC_HUBS));
	file.AppendMenu(MF_STRING, IDC_FAVORITES, CSTRING(MENU_FILE_FAVORITE_HUBS));
	file.AppendMenu(MF_STRING, IDC_FAVUSERS, CSTRING(MENU_FILE_FAVORITE_USERS));
	file.AppendMenu(MF_STRING, ID_FILE_SEARCH, CSTRING(MENU_FILE_SEARCH));
	file.AppendMenu(MF_STRING, IDC_NOTEPAD, CSTRING(MENU_FILE_NOTEPAD));
	file.AppendMenu(MF_STRING, IDC_SEARCH_SPY, CSTRING(MENU_FILE_SEARCH_SPY));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, IDC_FOLLOW, CSTRING(MENU_FILE_FOLLOW_REDIRECT));
	file.AppendMenu(MF_STRING, ID_FILE_RECONNECT, CSTRING(MENU_FILE_RECONNECT));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, ID_FILE_SETTINGS, CSTRING(MENU_FILE_SETTINGS));
	file.AppendMenu(MF_STRING, IDC_IMPORT_QUEUE, CSTRING(IMPORT_QUEUE));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, ID_APP_EXIT, CSTRING(MENU_FILE_EXIT));
	
	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)file, CSTRING(MENU_FILE));
	
	CMenuHandle view;
	view.CreatePopupMenu();
	
	view.AppendMenu(MF_STRING, ID_VIEW_TOOLBAR, CSTRING(MENU_VIEW_TOOLBAR));
	view.AppendMenu(MF_STRING, ID_VIEW_STATUS_BAR, CSTRING(MENU_VIEW_STATUS_BAR));
	
	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)view, CSTRING(MENU_VIEW));
	
	CMenuHandle window;
	window.CreatePopupMenu();
	
	window.AppendMenu(MF_STRING, ID_WINDOW_CASCADE, CSTRING(MENU_WINDOW_CASCADE));
	window.AppendMenu(MF_STRING, ID_WINDOW_TILE_HORZ, CSTRING(MENU_WINDOW_TILE));
	window.AppendMenu(MF_STRING, ID_WINDOW_ARRANGE, CSTRING(MENU_WINDOW_ARRANGE));
	
	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)window, CSTRING(MENU_WINDOW));
	
	CMenuHandle help;
	help.CreatePopupMenu();
	
	help.AppendMenu(MF_STRING, ID_APP_ABOUT, CSTRING(MENU_HELP_ABOUT));
	help.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	help.AppendMenu(MF_STRING, IDC_HELP_HOMEPAGE, CSTRING(MENU_HELP_HOMEPAGE));
	help.AppendMenu(MF_STRING, IDC_HELP_DOWNLOADS, CSTRING(MENU_HELP_DOWNLOADS));
	help.AppendMenu(MF_STRING, IDC_HELP_FAQ, CSTRING(MENU_HELP_FAQ));
	help.AppendMenu(MF_STRING, IDC_HELP_HELP_FORUM, CSTRING(MENU_HELP_HELP_FORUM));
	help.AppendMenu(MF_STRING, IDC_HELP_DISCUSS, CSTRING(MENU_HELP_DISCUSS));
	help.AppendMenu(MF_STRING, IDC_HELP_REQUEST_FEATURE, CSTRING(MENU_HELP_REQUEST_FEATURE));
	help.AppendMenu(MF_STRING, IDC_HELP_REPORT_BUG, CSTRING(MENU_HELP_REPORT_BUG));
	
	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)help, CSTRING(MENU_HELP));
	
}

/**
 * @file Util.cpp
 * $Id: Util.cpp,v 1.11 2002/04/03 23:20:35 arnetheduck Exp $
 * @if LOG
 * $Log: Util.cpp,v $
 * Revision 1.11  2002/04/03 23:20:35  arnetheduck
 * ...
 *
 * Revision 1.10  2002/03/13 20:35:26  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.9  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.8  2002/03/07 19:07:52  arnetheduck
 * Minor fixes + started code review
 *
 * Revision 1.7  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.6  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.5  2002/01/26 16:34:01  arnetheduck
 * Colors dialog added, as well as some other options
 *
 * Revision 1.4  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
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

