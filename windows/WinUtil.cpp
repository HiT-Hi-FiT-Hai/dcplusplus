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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "WinUtil.h"
#include "SearchFrm.h"
#include "LineDlg.h"

#include "../client/Util.h"
#include "../client/StringTokenizer.h"
#include "../client/ShareManager.h"
#include "../client/ClientManager.h"
#include "../client/TimerManager.h"
#include "../client/HubManager.h"

WinUtil::ImageMap WinUtil::fileIndexes;
HBRUSH WinUtil::bgBrush = NULL;
COLORREF WinUtil::textColor = 0;
COLORREF WinUtil::bgColor = 0;
HFONT WinUtil::font = NULL;
int WinUtil::fontHeight = 0;
HFONT WinUtil::boldFont = NULL;
CMenu WinUtil::mainMenu;
CImageList WinUtil::fileImages;
int WinUtil::dirIconIndex = 0;
StringList WinUtil::lastDirs;
string WinUtil::lastKick;
string WinUtil::lastRedirect;
string WinUtil::lastServer;
HWND WinUtil::mainWnd = NULL;
FlatTabCtrl* WinUtil::tabCtrl = NULL;

void WinUtil::decodeFont(const string& setting, LOGFONT &dest) {
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

int CALLBACK WinUtil::browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM /*lp*/, LPARAM pData) {
	switch(uMsg) {
	case BFFM_INITIALIZED: 
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
	}
	return 0;
}

bool WinUtil::browseDirectory(string& target, HWND owner /* = NULL */) {
	char buf[MAX_PATH];
	BROWSEINFO bi;
	LPMALLOC ma;
	
	ZeroMemory(&bi, sizeof(bi));
	
	bi.hwndOwner = owner;
	bi.pszDisplayName = buf;
	bi.lpszTitle = CSTRING(CHOOSE_FOLDER);
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

bool WinUtil::browseFile(string& target, HWND owner /* = NULL */, bool save /* = true */, const string& initialDir /* = Util::emptyString */) {
	char buf[MAX_PATH];
	OPENFILENAME ofn;       // common dialog box structure
	
	memcpy(buf, target.c_str(), target.length() + 1);
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFile = buf;

	if(!initialDir.empty()) {
		ofn.lpstrInitialDir = initialDir.c_str();
	}
	ofn.nMaxFile = sizeof(buf);
	ofn.Flags = save ? 0: OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	
	// Display the Open dialog box. 
	if ( (save ? GetSaveFileName(&ofn) : GetOpenFileName(&ofn) ) ==TRUE) {
		target = ofn.lpstrFile;
		return true;
	}
	return false;
}

void WinUtil::buildMenu() {
	mainMenu.CreateMenu();
	
	CMenuHandle file;
	file.CreatePopupMenu();
	
	file.AppendMenu(MF_STRING, IDC_QUEUE, CSTRING(MENU_FILE_DOWNLOAD_QUEUE));
	file.AppendMenu(MF_STRING, IDC_FINISHED, CSTRING(FINISHED_DOWNLOADS));
	file.AppendMenu(MF_STRING, IDC_FINISHED_UL, CSTRING(FINISHED_UPLOADS));
	file.AppendMenu(MF_STRING, ID_FILE_CONNECT, CSTRING(MENU_FILE_PUBLIC_HUBS));
	file.AppendMenu(MF_STRING, IDC_FAVORITES, CSTRING(MENU_FILE_FAVORITE_HUBS));
	file.AppendMenu(MF_STRING, IDC_FAVUSERS, CSTRING(MENU_FILE_FAVORITE_USERS));
	file.AppendMenu(MF_STRING, ID_FILE_SEARCH, CSTRING(MENU_FILE_SEARCH));
	file.AppendMenu(MF_STRING, IDC_FILE_ADL_SEARCH, CSTRING(MENU_FILE_ADL_SEARCH));
	file.AppendMenu(MF_STRING, IDC_SEARCH_SPY, CSTRING(MENU_FILE_SEARCH_SPY));
	file.AppendMenu(MF_STRING, IDC_NOTEPAD, CSTRING(MENU_FILE_NOTEPAD));
	file.AppendMenu(MF_STRING, IDC_OPEN_FILE_LIST, CSTRING(MENU_FILE_OPEN_FILE_LIST));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, IDC_FOLLOW, CSTRING(MENU_FILE_FOLLOW_REDIRECT));
	file.AppendMenu(MF_STRING, ID_FILE_RECONNECT, CSTRING(MENU_FILE_RECONNECT));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, ID_FILE_SETTINGS, CSTRING(MENU_FILE_SETTINGS));
	file.AppendMenu(MF_STRING, IDC_IMPORT_QUEUE, CSTRING(MENU_FILE_IMPORT_QUEUE));
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
	window.AppendMenu(MF_STRING, ID_WINDOW_MINIMIZE_ALL, CSTRING(MENU_WINDOW_MINIMIZE_ALL));
	window.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	window.AppendMenu(MF_STRING, IDC_CLOSE_DISCONNECTED, CSTRING(MENU_WINDOW_CLOSE_DISCONNECTED));

	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)window, CSTRING(MENU_WINDOW));
	
	CMenuHandle help;
	help.CreatePopupMenu();
	
	help.AppendMenu(MF_STRING, IDC_HELP_README, CSTRING(MENU_HELP_README));
	help.AppendMenu(MF_STRING, IDC_HELP_CHANGELOG, CSTRING(MENU_HELP_CHANGELOG));
	help.AppendMenu(MF_STRING, ID_APP_ABOUT, CSTRING(MENU_HELP_ABOUT));
	help.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	help.AppendMenu(MF_STRING, IDC_HELP_HOMEPAGE, CSTRING(MENU_HELP_HOMEPAGE));
	help.AppendMenu(MF_STRING, IDC_HELP_DOWNLOADS, CSTRING(MENU_HELP_DOWNLOADS));
	help.AppendMenu(MF_STRING, IDC_HELP_FAQ, CSTRING(MENU_HELP_FAQ));
	help.AppendMenu(MF_STRING, IDC_HELP_HELP_FORUM, CSTRING(MENU_HELP_HELP_FORUM));
	help.AppendMenu(MF_STRING, IDC_HELP_DISCUSS, CSTRING(MENU_HELP_DISCUSS));
	help.AppendMenu(MF_STRING, IDC_HELP_REQUEST_FEATURE, CSTRING(MENU_HELP_REQUEST_FEATURE));
	help.AppendMenu(MF_STRING, IDC_HELP_REPORT_BUG, CSTRING(MENU_HELP_REPORT_BUG));
	help.AppendMenu(MF_STRING, IDC_HELP_DONATE, CSTRING(MENU_HELP_DONATE));

	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)help, CSTRING(MENU_HELP));
	
}

void WinUtil::splitTokens(int* array, const string& tokens, int maxItems /* = -1 */) throw() {
	StringTokenizer t(tokens, ',');
	StringList& l = t.getTokens();
	if(maxItems == -1)
		maxItems = l.size();
	
	int k = 0;
	for(StringList::const_iterator i = l.begin(); i != l.end() && k < maxItems; ++i, ++k) {
		array[k] = Util::toInt(*i);
	}
}

bool WinUtil::getUCParams(HWND parent, const UserCommand& uc, StringMap& sm) throw() {
	string::size_type i = 0;

	while( (i = uc.getCommand().find("%[line:", i)) != string::npos) {
		i += 7;
		string::size_type j = uc.getCommand().find(']', i);
		if(j == string::npos)
			break;

		string name = uc.getCommand().substr(i, j-i);
		LineDlg dlg;
		dlg.title = uc.getName();
		dlg.description = name;
		dlg.line = sm["line:" + name];
		if(dlg.DoModal(parent) == IDOK) {
			sm["line:" + name] = dlg.line;
		} else {
			return false;
		}
		i = j + 1;
	}
	return true;
}

#define LINE2 "-- http://dcplusplus.sourceforge.net  <DC++ " VERSIONSTRING ">"
char *msgs[] = { "\r\n-- I'm a happy dc++ user. You could be happy too.\r\n" LINE2,
"\r\n-- Neo-...what? Nope...never heard of it...\r\n" LINE2,
"\r\n-- Evolution of species: Ape --> Man\r\n-- Evolution of science: \"The Earth is Flat\" --> \"The Earth is Round\"\r\n-- Evolution of sharing: NMDC --> DC++\r\n" LINE2,
"\r\n-- I share, therefore I am.\r\n" LINE2,
"\r\n-- I came, I searched, I found...\r\n" LINE2,
"\r\n-- I came, I shared, I sent...\r\n" LINE2,
"\r\n-- I can set away mode, can't you?\r\n" LINE2,
"\r\n-- I don't have to see any ads, do you?\r\n" LINE2,
"\r\n-- I don't have to see those annoying kick messages, do you?\r\n" LINE2,
"\r\n-- I can resume my files to a different filename, can you?\r\n" LINE2,
"\r\n-- I can share huge amounts of files, can you?\r\n" LINE2,
"\r\n-- My client doesn't spam the chat with useless debug messages, does yours?\r\n" LINE2,
"\r\n-- I can add multiple users to the same download and have the client connect to another automatically when one goes offline, can you?\r\n" LINE2,
"\r\n-- These addies are pretty annoying, aren't they? Get revenge by sending them yourself!\r\n" LINE2
};

#define MSGS 14

string WinUtil::commands = "/refresh, /slots #, /search <string>, /dc++, /away <msg>, /back";

bool WinUtil::checkCommand(HWND mdiClient, string& cmd, string& param, string& message, string& status) {
	int i = cmd.find(' ');
	if(i != string::npos) {
		param = cmd.substr(i+1);
		cmd = cmd.substr(1, i - 1);
	} else {
		cmd = cmd.substr(1);
	}

	if(Util::stricmp(cmd.c_str(), "refresh")==0) {
		try {
			ShareManager::getInstance()->setDirty();
			ShareManager::getInstance()->refresh(true);
			status = STRING(FILE_LIST_REFRESHED);
		} catch(const ShareException& e) {
			status = e.getError();
		}
	} else if(Util::stricmp(cmd.c_str(), "slots")==0) {
		int j = Util::toInt(param);
		if(j > 0) {
			SettingsManager::getInstance()->set(SettingsManager::SLOTS, j);
			status = STRING(SLOTS_SET);
			ClientManager::getInstance()->infoUpdated();
		} else {
			status = STRING(INVALID_NUMBER_OF_SLOTS);
		}
	} else if(Util::stricmp(cmd.c_str(), "search") == 0) {
		if(!param.empty()) {
			SearchFrame* pChild = new SearchFrame();
			pChild->setTab(tabCtrl);
			pChild->setInitial(param, 0, SearchManager::SIZE_ATLEAST);
			pChild->CreateEx(mdiClient);
		} else {
			status = STRING(SPECIFY_SEARCH_STRING);
		}
	} else if(Util::stricmp(cmd.c_str(), "dc++") == 0) {
		message = msgs[GET_TICK() % MSGS];
	} else if(Util::stricmp(cmd.c_str(), "away") == 0) {
		if(Util::getAway() && param.empty()) {
			Util::setAway(false);
			status = STRING(AWAY_MODE_OFF);
		} else {
			Util::setAway(true);
			Util::setAwayMessage(param);
			status = STRING(AWAY_MODE_ON) + Util::getAwayMessage();
		}
	} else if(Util::stricmp(cmd.c_str(), "back") == 0) {
		Util::setAway(false);
		status = STRING(AWAY_MODE_OFF);
	} else {
		return false;
	}

	return true;
}

void WinUtil::saveHeaderOrder(CListViewCtrl& ctrl, SettingsManager::StrSetting order, 
							  SettingsManager::StrSetting widths, int n, 
							  int* indexes, int* sizes) throw() {
	string tmp;

	ctrl.GetColumnOrderArray(n, indexes);
	int i;
	for(i = 0; i < n; ++i) {
		tmp += Util::toString(indexes[i]);
		tmp += ',';
	}
	tmp.erase(tmp.size()-1, 1);
	SettingsManager::getInstance()->set(order, tmp);
	tmp.clear();
	for(i = 0; i < n; ++i) {
		sizes[i] = ctrl.GetColumnWidth(i);
		tmp += Util::toString(sizes[i]);
		tmp += ',';
	}
	tmp.erase(tmp.size()-1, 1);
	SettingsManager::getInstance()->set(widths, tmp);
}

/**
 * @file
 * $Id: WinUtil.cpp,v 1.15 2003/05/14 09:17:57 arnetheduck Exp $
 */
