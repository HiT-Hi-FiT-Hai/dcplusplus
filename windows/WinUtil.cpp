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
#include "../client/ResourceManager.h"

WinUtil::ImageMap WinUtil::fileIndexes;
int WinUtil::fileImageCount;
HBRUSH WinUtil::bgBrush = NULL;
COLORREF WinUtil::textColor = 0;
COLORREF WinUtil::bgColor = 0;
HFONT WinUtil::font = NULL;
int WinUtil::fontHeight = 0;
HFONT WinUtil::boldFont = NULL;
HFONT WinUtil::systemFont = NULL;
HFONT WinUtil::monoFont = NULL;
CMenu WinUtil::mainMenu;
CImageList WinUtil::fileImages;
CImageList WinUtil::userImages;
int WinUtil::dirIconIndex = 0;
StringList WinUtil::lastDirs;
HWND WinUtil::mainWnd = NULL;
HWND WinUtil::mdiClient = NULL;
FlatTabCtrl* WinUtil::tabCtrl = NULL;

HLSCOLOR RGB2HLS (COLORREF rgb) {
	unsigned char minval = min(GetRValue(rgb), min(GetGValue(rgb), GetBValue(rgb)));
	unsigned char maxval = max(GetRValue(rgb), max(GetGValue(rgb), GetBValue(rgb)));
	float mdiff  = float(maxval) - float(minval);
	float msum   = float(maxval) + float(minval);

	float luminance = msum / 510.0f;
	float saturation = 0.0f;
	float hue = 0.0f; 

	if ( maxval != minval ) { 
		float rnorm = (maxval - GetRValue(rgb)  ) / mdiff;      
		float gnorm = (maxval - GetGValue(rgb)) / mdiff;
		float bnorm = (maxval - GetBValue(rgb) ) / mdiff;   

		saturation = (luminance <= 0.5f) ? (mdiff / msum) : (mdiff / (510.0f - msum));

		if (GetRValue(rgb) == maxval) hue = 60.0f * (6.0f + bnorm - gnorm);
		if (GetGValue(rgb) == maxval) hue = 60.0f * (2.0f + rnorm - bnorm);
		if (GetBValue(rgb) == maxval) hue = 60.0f * (4.0f + gnorm - rnorm);
		if (hue > 360.0f) hue = hue - 360.0f;
	}
	return HLS ((hue*255)/360, luminance*255, saturation*255);
}

static BYTE _ToRGB (float rm1, float rm2, float rh) {
	if      (rh > 360.0f) rh -= 360.0f;
	else if (rh <   0.0f) rh += 360.0f;

	if      (rh <  60.0f) rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;   
	else if (rh < 180.0f) rm1 = rm2;
	else if (rh < 240.0f) rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;      

	return (BYTE)(rm1 * 255);
}

COLORREF HLS2RGB (HLSCOLOR hls) {
	float hue        = ((int)HLS_H(hls)*360)/255.0f;
	float luminance  = HLS_L(hls)/255.0f;
	float saturation = HLS_S(hls)/255.0f;

	if ( saturation == 0.0f ) {
		return RGB (HLS_L(hls), HLS_L(hls), HLS_L(hls));
	}
	float rm1, rm2;

	if ( luminance <= 0.5f ) rm2 = luminance + luminance * saturation;  
	else                     rm2 = luminance + saturation - luminance * saturation;
	rm1 = 2.0f * luminance - rm2;   
	BYTE red   = _ToRGB (rm1, rm2, hue + 120.0f);   
	BYTE green = _ToRGB (rm1, rm2, hue);
	BYTE blue  = _ToRGB (rm1, rm2, hue - 120.0f);

	return RGB (red, green, blue);
}

COLORREF HLS_TRANSFORM (COLORREF rgb, int percent_L, int percent_S) {
	HLSCOLOR hls = RGB2HLS (rgb);
	BYTE h = HLS_H(hls);
	BYTE l = HLS_L(hls);
	BYTE s = HLS_S(hls);

	if ( percent_L > 0 ) {
		l = BYTE(l + ((255 - l) * percent_L) / 100);
	} else if ( percent_L < 0 )	{
		l = BYTE((l * (100+percent_L)) / 100);
	}
	if ( percent_S > 0 ) {
		s = BYTE(s + ((255 - s) * percent_S) / 100);
	} else if ( percent_S < 0 ) {
		s = BYTE((s * (100+percent_S)) / 100);
	}
	return HLS2RGB (HLS(h, l, s));
}

void WinUtil::init(HWND hWnd) {
	mainWnd = hWnd;

	mainMenu.CreateMenu();

	CMenuHandle file;
	file.CreatePopupMenu();

	file.AppendMenu(MF_STRING, IDC_OPEN_FILE_LIST, CSTRING(MENU_OPEN_FILE_LIST));
	file.AppendMenu(MF_STRING, IDC_REFRESH_FILE_LIST, CSTRING(MENU_REFRESH_FILE_LIST));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, IDC_FOLLOW, CSTRING(MENU_FOLLOW_REDIRECT));
	file.AppendMenu(MF_STRING, ID_FILE_RECONNECT, CSTRING(MENU_RECONNECT));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, IDC_IMPORT_QUEUE, CSTRING(MENU_IMPORT_QUEUE));
	file.AppendMenu(MF_STRING, ID_FILE_SETTINGS, CSTRING(MENU_SETTINGS));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, ID_APP_EXIT, CSTRING(MENU_EXIT));

	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)file, CSTRING(MENU_FILE));

	CMenuHandle view;
	view.CreatePopupMenu();

	view.AppendMenu(MF_STRING, ID_FILE_CONNECT, CSTRING(MENU_PUBLIC_HUBS));
	view.AppendMenu(MF_STRING, IDC_QUEUE, CSTRING(MENU_DOWNLOAD_QUEUE));
	view.AppendMenu(MF_STRING, IDC_FINISHED, CSTRING(FINISHED_DOWNLOADS));
	view.AppendMenu(MF_STRING, IDC_FINISHED_UL, CSTRING(FINISHED_UPLOADS));
	view.AppendMenu(MF_STRING, IDC_FAVORITES, CSTRING(MENU_FAVORITE_HUBS));
	view.AppendMenu(MF_STRING, IDC_FAVUSERS, CSTRING(MENU_FAVORITE_USERS));
	view.AppendMenu(MF_STRING, ID_FILE_SEARCH, CSTRING(MENU_SEARCH));
	view.AppendMenu(MF_STRING, IDC_FILE_ADL_SEARCH, CSTRING(MENU_ADL_SEARCH));
	view.AppendMenu(MF_STRING, IDC_SEARCH_SPY, CSTRING(MENU_SEARCH_SPY));
	view.AppendMenu(MF_STRING, IDC_NET_STATS, CSTRING(MENU_NETWORK_STATISTICS));
	view.AppendMenu(MF_STRING, IDC_NOTEPAD, CSTRING(MENU_NOTEPAD));
	view.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	view.AppendMenu(MF_STRING, ID_VIEW_TOOLBAR, CSTRING(MENU_TOOLBAR));
	view.AppendMenu(MF_STRING, ID_VIEW_STATUS_BAR, CSTRING(MENU_STATUS_BAR));

	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)view, CSTRING(MENU_VIEW));

	CMenuHandle window;
	window.CreatePopupMenu();

	window.AppendMenu(MF_STRING, ID_WINDOW_CASCADE, CSTRING(MENU_CASCADE));
	window.AppendMenu(MF_STRING, ID_WINDOW_TILE_HORZ, CSTRING(MENU_HORIZONTAL_TILE));
	window.AppendMenu(MF_STRING, ID_WINDOW_TILE_VERT, CSTRING(MENU_VERTICAL_TILE));
	window.AppendMenu(MF_STRING, ID_WINDOW_ARRANGE, CSTRING(MENU_ARRANGE));
	window.AppendMenu(MF_STRING, ID_WINDOW_MINIMIZE_ALL, CSTRING(MENU_MINIMIZE_ALL));
	window.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	window.AppendMenu(MF_STRING, IDC_CLOSE_DISCONNECTED, CSTRING(MENU_CLOSE_DISCONNECTED));

	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)window, CSTRING(MENU_WINDOW));

	CMenuHandle help;
	help.CreatePopupMenu();

	help.AppendMenu(MF_STRING, IDC_HELP_README, CSTRING(MENU_README));
	help.AppendMenu(MF_STRING, IDC_HELP_CHANGELOG, CSTRING(MENU_CHANGELOG));
	help.AppendMenu(MF_STRING, ID_APP_ABOUT, CSTRING(MENU_ABOUT));
	help.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	help.AppendMenu(MF_STRING, IDC_HELP_HOMEPAGE, CSTRING(MENU_HOMEPAGE));
	help.AppendMenu(MF_STRING, IDC_HELP_DOWNLOADS, CSTRING(MENU_DOWNLOADS));
	help.AppendMenu(MF_STRING, IDC_HELP_FAQ, CSTRING(MENU_FAQ));
	help.AppendMenu(MF_STRING, IDC_HELP_HELP_FORUM, CSTRING(MENU_HELP_FORUM));
	help.AppendMenu(MF_STRING, IDC_HELP_DISCUSS, CSTRING(MENU_DISCUSS));
	help.AppendMenu(MF_STRING, IDC_HELP_REQUEST_FEATURE, CSTRING(MENU_REQUEST_FEATURE));
	help.AppendMenu(MF_STRING, IDC_HELP_REPORT_BUG, CSTRING(MENU_REPORT_BUG));
	help.AppendMenu(MF_STRING, IDC_HELP_DONATE, CSTRING(MENU_DONATE));

	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)help, CSTRING(MENU_HELP));

	if(BOOLSETTING(USE_SYSTEM_ICONS)) {
		SHFILEINFO fi;
		fileImages.Create(16, 16, ILC_COLOR8, 16, 16);
		fileImages.SetBkColor(SETTING(BACKGROUND_COLOR));
		::SHGetFileInfo(".", FILE_ATTRIBUTE_DIRECTORY, &fi, sizeof(fi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
		fileImages.AddIcon(fi.hIcon);
		::DestroyIcon(fi.hIcon);
		dirIconIndex = fileImageCount++;	
	} else {
		fileImages.CreateFromImage(IDB_FOLDERS, 16, 3, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
		dirIconIndex = 0;
	}

	userImages.CreateFromImage(IDB_USERS, 16, 8, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);

	LOGFONT lf, lf2;
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
	SettingsManager::getInstance()->setDefault(SettingsManager::TEXT_FONT, encodeFont(lf));
	decodeFont(SETTING(TEXT_FONT), lf);
	::GetObject((HFONT)GetStockObject(ANSI_FIXED_FONT), sizeof(lf2), &lf2);
	
	lf2.lfHeight = lf.lfHeight;
	lf2.lfWeight = lf.lfWeight;
	lf2.lfItalic = lf.lfItalic;

	bgBrush = CreateSolidBrush(SETTING(BACKGROUND_COLOR));
	textColor = SETTING(TEXT_COLOR);
	bgColor = SETTING(BACKGROUND_COLOR);
	font = ::CreateFontIndirect(&lf);
	fontHeight = WinUtil::getTextHeight(mainWnd, font);
	lf.lfWeight = FW_BOLD;
	boldFont = ::CreateFontIndirect(&lf);
	systemFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	monoFont = (HFONT)::GetStockObject(BOOLSETTING(USE_OEM_MONOFONT)?OEM_FIXED_FONT:ANSI_FIXED_FONT);
}

void WinUtil::uninit() {
	fileImages.Destroy();
	userImages.Destroy();
	::DeleteObject(font);
	::DeleteObject(boldFont);
	::DeleteObject(bgBrush);
	::DeleteObject(monoFont);

	mainMenu.DestroyMenu();

}

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

bool WinUtil::browseFile(string& target, HWND owner /* = NULL */, bool save /* = true */, const string& initialDir /* = Util::emptyString */, const char* types /* = NULL */, const char* defExt /* = NULL */) {
	char buf[MAX_PATH];
	OPENFILENAME ofn;       // common dialog box structure
	target = Util::validateFileName(target);
	memcpy(buf, target.c_str(), target.length() + 1);
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFile = buf;
	ofn.lpstrFilter = types;
	ofn.lpstrDefExt = defExt;
	ofn.nFilterIndex = 1;

	if(!initialDir.empty()) {
		ofn.lpstrInitialDir = initialDir.c_str();
	}
	ofn.nMaxFile = sizeof(buf);
	ofn.Flags = (save ? 0: OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
	
	// Display the Open dialog box. 
	if ( (save ? GetSaveFileName(&ofn) : GetOpenFileName(&ofn) ) ==TRUE) {
		target = ofn.lpstrFile;
		return true;
	}
	return false;
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
	StringMap done;

	while( (i = uc.getCommand().find("%[line:", i)) != string::npos) {
		i += 7;
		string::size_type j = uc.getCommand().find(']', i);
		if(j == string::npos)
			break;

		string name = uc.getCommand().substr(i, j-i);
		if(done.find(name) == done.end()) {
			LineDlg dlg;
			dlg.title = uc.getName();
			dlg.description = name;
			dlg.line = sm["line:" + name];
			if(dlg.DoModal(parent) == IDOK) {
				sm["line:" + name] = dlg.line;
				done[name] = dlg.line;
			} else {
				return false;
			}
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

string WinUtil::commands = "/refresh, /slots #, /search <string>, /dc++, /away <msg>, /back, /g <searchstring>, /imdb <imdbquery>";

bool WinUtil::checkCommand(string& cmd, string& param, string& message, string& status) {
	string::size_type i = cmd.find(' ');
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
			SearchFrame::openWindow(param);
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
	} else if(Util::stricmp(cmd.c_str(), "g") == 0) {
		if(param.empty()) {
			status = STRING(SPECIFY_SEARCH_STRING);
		} else {
			WinUtil::openLink("http://www.google.com/search?q="+param);
		}
	} else if(Util::stricmp(cmd.c_str(), "imdb") == 0) {
		if(param.empty()) {
			status = STRING(SPECIFY_SEARCH_STRING);
		} else {
			WinUtil::openLink("http://www.imdb.com/find?q="+param);
		}
	} else {
		return false;
	}

	return true;
}

void WinUtil::openLink(const string& url) {
	CRegKey key;
	char buf[MAX_PATH];
	ULONG len = MAX_PATH;
	if(key.Open(HKEY_CLASSES_ROOT, "http\\shell\\open\\command", KEY_READ) == ERROR_SUCCESS) {
		if(key.QueryStringValue(NULL, buf, &len) == ERROR_SUCCESS) {
			string cmd(buf, len);
			
			if(!cmd.empty()) {
				string::size_type start,end;
				if(cmd[0] == '"') {
					start = 1;
					end = cmd.find('"');
				} else {
					start = 0;
					end = cmd.find(' ');
				}
				if(end == string::npos)
					end = cmd.length();

				string param;
				if(end < cmd.length()) {
					param = cmd.substr(end + 1);
				}
				cmd = cmd.substr(start, end-start);
				string::size_type i = param.find("%1");
				if(i != string::npos) {
					param.replace(i, 2, url);
				} else {
					param = " " + url;
				}

				if((int)::ShellExecute(NULL, NULL, cmd.c_str(), param.c_str(), NULL, SW_SHOWNORMAL) > 32) {
					return;
				}
			}
		}
	}

	::ShellExecute(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOWNORMAL);
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

int WinUtil::getIconIndex(const string& aFileName) {
	if(BOOLSETTING(USE_SYSTEM_ICONS)) {
		SHFILEINFO fi;
		string x = Util::getFileName(aFileName);
		string::size_type i = x.rfind('.');
		if(i != string::npos) {
			x = x.substr(i);
			ImageIter j = fileIndexes.find(x);
			if(j != fileIndexes.end())
				return j->second;
		}

		::SHGetFileInfo(x.c_str(), FILE_ATTRIBUTE_NORMAL, &fi, sizeof(fi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
		fileImages.AddIcon(fi.hIcon);
		::DestroyIcon(fi.hIcon);

		fileIndexes[x] = fileImageCount++;
		return fileImageCount - 1;
	} else {
		return 2;
	}
}
/**
 * @file
 * $Id: WinUtil.cpp,v 1.31 2003/11/10 22:42:12 arnetheduck Exp $
 */
