/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#include "PrivateFrame.h"
#include "SearchFrm.h"
#include "LineDlg.h"

#include "../client/Util.h"
#include "../client/StringTokenizer.h"
#include "../client/ShareManager.h"
#include "../client/ClientManager.h"
#include "../client/TimerManager.h"
#include "../client/HubManager.h"
#include "../client/ResourceManager.h"
#include "../client/QueueManager.h"
#include "../client/UploadManager.h"
#include "../client/HashManager.h"
#include "../client/LogManager.h"
#include "HubFrame.h"
#include "MagnetDlg.h"

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
TStringList WinUtil::lastDirs;
HWND WinUtil::mainWnd = NULL;
HWND WinUtil::mdiClient = NULL;
FlatTabCtrl* WinUtil::tabCtrl = NULL;
HHOOK WinUtil::hook = NULL;
tstring WinUtil::tth;
StringPairList WinUtil::initialDirs;
DWORD WinUtil::helpCookie = 0;

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

void UserInfoBase::matchQueue() {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
	} catch(const Exception&) {
	}
}
void UserInfoBase::getList() {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} catch(const Exception&) {
	}
}
void UserInfoBase::addFav() {
	HubManager::getInstance()->addFavoriteUser(user);
}
void UserInfoBase::pm() {
	PrivateFrame::openWindow(user);
}
void UserInfoBase::grant() {
	UploadManager::getInstance()->reserveSlot(user);
}
void UserInfoBase::removeAll() {
	QueueManager::getInstance()->removeSources(user, QueueItem::Source::FLAG_REMOVED);
}

bool WinUtil::getVersionInfo(OSVERSIONINFOEX& ver) {
	memset(&ver, 0, sizeof(OSVERSIONINFOEX));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if(!GetVersionEx((OSVERSIONINFO*)&ver)) {
		ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if(!GetVersionEx((OSVERSIONINFO*)&ver)) {
			return false;
		}
	}
	return true;
}

static LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
	if(code == HC_ACTION) {
		if(wParam == VK_CONTROL && LOWORD(lParam) == 1) {
			if(lParam & 0x80000000) {
				WinUtil::tabCtrl->endSwitch();
			} else {
				WinUtil::tabCtrl->startSwitch();
			}
		}
	}
	return CallNextHookEx(WinUtil::hook, code, wParam, lParam);
}


void WinUtil::init(HWND hWnd) {
	mainWnd = hWnd;

	mainMenu.CreateMenu();

	CMenuHandle file;
	file.CreatePopupMenu();

	file.AppendMenu(MF_STRING, IDC_OPEN_FILE_LIST, CTSTRING(MENU_OPEN_FILE_LIST));
	file.AppendMenu(MF_STRING, IDC_REFRESH_FILE_LIST, CTSTRING(MENU_REFRESH_FILE_LIST));
	file.AppendMenu(MF_STRING, IDC_OPEN_DOWNLOADS, CTSTRING(MENU_OPEN_DOWNLOADS_DIR));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, ID_FILE_QUICK_CONNECT, CTSTRING(MENU_QUICK_CONNECT));
	file.AppendMenu(MF_STRING, IDC_FOLLOW, CTSTRING(MENU_FOLLOW_REDIRECT));
	file.AppendMenu(MF_STRING, ID_FILE_RECONNECT, CTSTRING(MENU_RECONNECT));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, ID_FILE_SETTINGS, CTSTRING(MENU_SETTINGS));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, ID_APP_EXIT, CTSTRING(MENU_EXIT));

	mainMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)file, CTSTRING(MENU_FILE));

	CMenuHandle view;
	view.CreatePopupMenu();

	view.AppendMenu(MF_STRING, ID_FILE_CONNECT, CTSTRING(MENU_PUBLIC_HUBS));
	view.AppendMenu(MF_STRING, IDC_QUEUE, CTSTRING(MENU_DOWNLOAD_QUEUE));
	view.AppendMenu(MF_STRING, IDC_FINISHED, CTSTRING(FINISHED_DOWNLOADS));
	view.AppendMenu(MF_STRING, IDC_FINISHED_UL, CTSTRING(FINISHED_UPLOADS));
	view.AppendMenu(MF_STRING, IDC_FAVORITES, CTSTRING(MENU_FAVORITE_HUBS));
	view.AppendMenu(MF_STRING, IDC_FAVUSERS, CTSTRING(MENU_FAVORITE_USERS));
	view.AppendMenu(MF_STRING, ID_FILE_SEARCH, CTSTRING(MENU_SEARCH));
	view.AppendMenu(MF_STRING, IDC_FILE_ADL_SEARCH, CTSTRING(MENU_ADL_SEARCH));
	view.AppendMenu(MF_STRING, IDC_SEARCH_SPY, CTSTRING(MENU_SEARCH_SPY));
	view.AppendMenu(MF_STRING, IDC_NET_STATS, CTSTRING(MENU_NETWORK_STATISTICS));
	view.AppendMenu(MF_STRING, IDC_NOTEPAD, CTSTRING(MENU_NOTEPAD));
	view.AppendMenu(MF_STRING, IDC_HASH_PROGRESS, CTSTRING(MENU_HASH_PROGRESS));
	view.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	view.AppendMenu(MF_STRING, ID_VIEW_TOOLBAR, CTSTRING(MENU_TOOLBAR));
	view.AppendMenu(MF_STRING, ID_VIEW_STATUS_BAR, CTSTRING(MENU_STATUS_BAR));
	view.AppendMenu(MF_STRING, ID_VIEW_TRANSFER_VIEW, CTSTRING(MENU_TRANSFER_VIEW));

	mainMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)view, CTSTRING(MENU_VIEW));

	CMenuHandle window;
	window.CreatePopupMenu();

	window.AppendMenu(MF_STRING, ID_WINDOW_CASCADE, CTSTRING(MENU_CASCADE));
	window.AppendMenu(MF_STRING, ID_WINDOW_TILE_HORZ, CTSTRING(MENU_HORIZONTAL_TILE));
	window.AppendMenu(MF_STRING, ID_WINDOW_TILE_VERT, CTSTRING(MENU_VERTICAL_TILE));
	window.AppendMenu(MF_STRING, ID_WINDOW_ARRANGE, CTSTRING(MENU_ARRANGE));
	window.AppendMenu(MF_STRING, ID_WINDOW_MINIMIZE_ALL, CTSTRING(MENU_MINIMIZE_ALL));
	window.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	window.AppendMenu(MF_STRING, IDC_CLOSE_DISCONNECTED, CTSTRING(MENU_CLOSE_DISCONNECTED));

	mainMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)window, CTSTRING(MENU_WINDOW));

	CMenuHandle help;
	help.CreatePopupMenu();

	help.AppendMenu(MF_STRING, IDC_HELP_CONTENTS, CTSTRING(MENU_CONTENTS));
	help.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	help.AppendMenu(MF_STRING, IDC_HELP_README, CTSTRING(MENU_README));
	help.AppendMenu(MF_STRING, IDC_HELP_CHANGELOG, CTSTRING(MENU_CHANGELOG));
	help.AppendMenu(MF_STRING, ID_APP_ABOUT, CTSTRING(MENU_ABOUT));
	help.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	help.AppendMenu(MF_STRING, IDC_HELP_HOMEPAGE, CTSTRING(MENU_HOMEPAGE));
	help.AppendMenu(MF_STRING, IDC_HELP_DOWNLOADS, CTSTRING(MENU_HELP_DOWNLOADS));
	help.AppendMenu(MF_STRING, IDC_HELP_TRANSLATIONS, CTSTRING(MENU_HELP_TRANSLATIONS));
	help.AppendMenu(MF_STRING, IDC_HELP_FAQ, CTSTRING(MENU_FAQ));
	help.AppendMenu(MF_STRING, IDC_HELP_HELP_FORUM, CTSTRING(MENU_HELP_FORUM));
	help.AppendMenu(MF_STRING, IDC_HELP_DISCUSS, CTSTRING(MENU_DISCUSS));
	help.AppendMenu(MF_STRING, IDC_HELP_REQUEST_FEATURE, CTSTRING(MENU_REQUEST_FEATURE));
	help.AppendMenu(MF_STRING, IDC_HELP_REPORT_BUG, CTSTRING(MENU_REPORT_BUG));
	help.AppendMenu(MF_STRING, IDC_HELP_DONATE, CTSTRING(MENU_DONATE));

	mainMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)help, CTSTRING(MENU_HELP));

	if(BOOLSETTING(USE_SYSTEM_ICONS)) {
		SHFILEINFO fi;
		fileImages.Create(16, 16, ILC_COLOR32 | ILC_MASK, 16, 16);
		::SHGetFileInfo(_T("."), FILE_ATTRIBUTE_DIRECTORY, &fi, sizeof(fi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
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
	SettingsManager::getInstance()->setDefault(SettingsManager::TEXT_FONT, Text::fromT(encodeFont(lf)));
	decodeFont(Text::toT(SETTING(TEXT_FONT)), lf);
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

	if(BOOLSETTING(URL_HANDLER)) {
		registerDchubHandler();
	}
	registerMagnetHandler();

	hook = SetWindowsHookEx(WH_KEYBOARD, &KeyboardProc, NULL, GetCurrentThreadId());

	HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD)&helpCookie);
}

void WinUtil::uninit() {
	HtmlHelp(NULL, NULL, HH_UNINITIALIZE, helpCookie);

	fileImages.Destroy();
	userImages.Destroy();
	::DeleteObject(font);
	::DeleteObject(boldFont);
	::DeleteObject(bgBrush);
	::DeleteObject(monoFont);

	mainMenu.DestroyMenu();

	UnhookWindowsHookEx(hook);

}

void WinUtil::decodeFont(const tstring& setting, LOGFONT &dest) {
	StringTokenizer<tstring> st(setting, _T(','));
	TStringList &sl = st.getTokens();
	
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(dest), &dest);
	tstring face;
	if(sl.size() == 4)
	{
		face = sl[0];
		dest.lfHeight = Util::toInt(Text::fromT(sl[1]));
		dest.lfWeight = Util::toInt(Text::fromT(sl[2]));
		dest.lfItalic = (BYTE)Util::toInt(Text::fromT(sl[3]));
	}
	
	if(!face.empty()) {
		::ZeroMemory(dest.lfFaceName, LF_FACESIZE);
		_tcscpy(dest.lfFaceName, face.c_str());
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

bool WinUtil::browseDirectory(tstring& target, HWND owner /* = NULL */) {
	TCHAR buf[MAX_PATH];
	BROWSEINFO bi;
	LPMALLOC ma;
	
	ZeroMemory(&bi, sizeof(bi));
	
	bi.hwndOwner = owner;
	bi.pszDisplayName = buf;
	bi.lpszTitle = CTSTRING(CHOOSE_FOLDER);
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	bi.lParam = (LPARAM)target.c_str();
	bi.lpfn = &browseCallbackProc;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if(pidl != NULL) {
		SHGetPathFromIDList(pidl, buf);
		target = buf;
		
		if(target.size() > 0 && target[target.size()-1] != L'\\')
			target+=L'\\';
		
		if(SHGetMalloc(&ma) != E_FAIL) {
			ma->Free(pidl);
			ma->Release();
		}
		return true;
	}
	return false;
}

bool WinUtil::browseFile(tstring& target, HWND owner /* = NULL */, bool save /* = true */, const tstring& initialDir /* = Util::emptyString */, const TCHAR* types /* = NULL */, const TCHAR* defExt /* = NULL */) {
	TCHAR buf[MAX_PATH];
	OPENFILENAME ofn;       // common dialog box structure
	target = Text::toT(Util::validateFileName(Text::fromT(target)));
	_tcscpy(buf, target.c_str());
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

tstring WinUtil::encodeFont(LOGFONT const& font)
{
	tstring res(font.lfFaceName);
	res += L',';
	res += Text::toT(Util::toString(font.lfHeight));
	res += L',';
	res += Text::toT(Util::toString(font.lfWeight));
	res += L',';
	res += Text::toT(Util::toString(font.lfItalic));
	return res;
}


void WinUtil::setClipboard(const tstring& str) {
	if(!::OpenClipboard(mainWnd)) {
		return;
	}

	EmptyClipboard();

	// Allocate a global memory object for the text. 
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (str.size() + 1) * sizeof(TCHAR)); 
	if (hglbCopy == NULL) { 
		CloseClipboard(); 
		return; 
	} 

	// Lock the handle and copy the text to the buffer. 
	TCHAR* lptstrCopy = (TCHAR*)GlobalLock(hglbCopy); 
	_tcscpy(lptstrCopy, str.c_str());
	GlobalUnlock(hglbCopy); 

	// Place the handle on the clipboard. 
	SetClipboardData(CF_TEXT, hglbCopy); 
	CloseClipboard();
}

void WinUtil::splitTokens(int* array, const string& tokens, int maxItems /* = -1 */) throw() {
	StringTokenizer<string> t(tokens, _T(','));
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
			dlg.title = Text::toT(uc.getName());
			dlg.description = Text::toT(name);
			dlg.line = Text::toT(sm["line:" + name]);
			if(dlg.DoModal(parent) == IDOK) {
				sm["line:" + name] = Text::fromT(dlg.line);
				done[name] = Text::fromT(dlg.line);
			} else {
				return false;
			}
		}
		i = j + 1;
	}
	return true;
}

#define LINE2 _T("-- http://dcplusplus.sourceforge.net  <DC++ ") _T(VERSIONSTRING) _T(">")
TCHAR *msgs[] = { _T("\r\n-- I'm a happy dc++ user. You could be happy too.\r\n") LINE2,
_T("\r\n-- Neo-...what? Nope...never heard of it...\r\n") LINE2,
_T("\r\n-- Evolution of species: Ape --> Man\r\n-- Evolution of science: \"The Earth is Flat\" --> \"The Earth is Round\"\r\n-- Evolution of sharing: NMDC --> DC++\r\n") LINE2,
_T("\r\n-- I share, therefore I am.\r\n") LINE2,
_T("\r\n-- I came, I searched, I found...\r\n") LINE2,
_T("\r\n-- I came, I shared, I sent...\r\n") LINE2,
_T("\r\n-- I can set away mode, can't you?\r\n") LINE2,
_T("\r\n-- I don't have to see any ads, do you?\r\n") LINE2,
_T("\r\n-- I don't have to see those annoying kick messages, do you?\r\n") LINE2,
_T("\r\n-- I can resume my files to a different filename, can you?\r\n") LINE2,
_T("\r\n-- I can share huge amounts of files, can you?\r\n") LINE2,
_T("\r\n-- My client doesn't spam the chat with useless debug messages, does yours?\r\n") LINE2,
_T("\r\n-- I can add multiple users to the same download and have the client connect to another automatically when one goes offline, can you?\r\n") LINE2,
_T("\r\n-- These addies are pretty annoying, aren't they? Get revenge by sending them yourself!\r\n") LINE2,
_T("\r\n-- My client supports TTH hashes, does yours?\r\n") LINE2,
_T("\r\n-- My client supports XML file lists, does yours?\r\n") LINE2
};

#define MSGS 16

tstring WinUtil::commands = _T("/refresh, /slots #, /search <string>, /dc++, /away <msg>, /back, /g <searchstring>, /imdb <imdbquery>, /rebuild");

bool WinUtil::checkCommand(tstring& cmd, tstring& param, tstring& message, tstring& status) {
	string::size_type i = cmd.find(' ');
	if(i != string::npos) {
		param = cmd.substr(i+1);
		cmd = cmd.substr(1, i - 1);
	} else {
		cmd = cmd.substr(1);
	}

	if(Util::stricmp(cmd.c_str(), _T("refresh"))==0) {
		try {
			ShareManager::getInstance()->setDirty();
			ShareManager::getInstance()->refresh(true);
		} catch(const ShareException& e) {
			status = Text::toT(e.getError());
		}
	} else if(Util::stricmp(cmd.c_str(), _T("slots"))==0) {
		int j = Util::toInt(Text::fromT(param));
		if(j > 0) {
			SettingsManager::getInstance()->set(SettingsManager::SLOTS, j);
			status = TSTRING(SLOTS_SET);
			ClientManager::getInstance()->infoUpdated();
		} else {
			status = TSTRING(INVALID_NUMBER_OF_SLOTS);
		}
	} else if(Util::stricmp(cmd.c_str(), _T("search")) == 0) {
		if(!param.empty()) {
			SearchFrame::openWindow(param);
		} else {
			status = TSTRING(SPECIFY_SEARCH_STRING);
		}
	} else if(Util::stricmp(cmd.c_str(), _T("dc++")) == 0) {
		message = msgs[GET_TICK() % MSGS];
	} else if(Util::stricmp(cmd.c_str(), _T("away")) == 0) {
		if(Util::getAway() && param.empty()) {
			Util::setAway(false);
			status = TSTRING(AWAY_MODE_OFF);
		} else {
			Util::setAway(true);
			Util::setAwayMessage(Text::fromT(param));
			status = TSTRING(AWAY_MODE_ON) + Text::toT(Util::getAwayMessage());
		}
	} else if(Util::stricmp(cmd.c_str(), _T("back")) == 0) {
		Util::setAway(false);
		status = TSTRING(AWAY_MODE_OFF);
	} else if(Util::stricmp(cmd.c_str(), _T("g")) == 0) {
		if(param.empty()) {
			status = TSTRING(SPECIFY_SEARCH_STRING);
		} else {
			WinUtil::openLink(_T("http://www.google.com/search?q=") + Text::toT(Util::encodeURI(Text::fromT(param))));
		}
	} else if(Util::stricmp(cmd.c_str(), _T("imdb")) == 0) {
		if(param.empty()) {
			status = TSTRING(SPECIFY_SEARCH_STRING);
		} else {
			WinUtil::openLink(_T("http://www.imdb.com/find?q=") + Text::toT(Util::encodeURI(Text::fromT(param))));
		}
	} else if(Util::stricmp(cmd.c_str(), _T("rebuild")) == 0) {
		HashManager::getInstance()->rebuild();
	} else {
		return false;
	}

	return true;
}

void WinUtil::bitziLink(TTHValue* aHash) {
	// to use this free service by bitzi, we must not hammer or request information from bitzi
	// except when the user requests it (a mass lookup isn't acceptable), and (if we ever fetch
	// this data within DC++, we must identify the client/mod in the user agent, so abuse can be 
	// tracked down and the code can be fixed
	if(aHash != NULL) {
		openLink(_T("http://bitzi.com/lookup/tree:tiger:") + Text::toT(aHash->toBase32()));
	}
}

 void WinUtil::copyMagnet(TTHValue* aHash, const tstring& aFile) {
	if(aHash != NULL && !aFile.empty()) {
		setClipboard(_T("magnet:?xt=urn:tree:tiger:") + Text::toT(aHash->toBase32()) + _T("&dn=") + Text::toT(Util::encodeURI(Text::fromT(aFile))));
	}
}

 void WinUtil::searchHash(TTHValue* aHash) {
	 if(aHash != NULL) {
		 SearchFrame::openWindow(Text::toT(aHash->toBase32()), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_HASH);
	 }
 }

 void WinUtil::registerDchubHandler() {
	HKEY hk;
	TCHAR Buf[512];
	tstring app = _T("\"") + Text::toT(Util::getAppName()) + _T("\" %1");
	Buf[0] = 0;

	if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("dchub\\Shell\\Open\\Command"), 0, KEY_WRITE | KEY_READ, &hk) == ERROR_SUCCESS) {
		DWORD bufLen = sizeof(Buf);
		DWORD type;
		::RegQueryValueEx(hk, NULL, 0, &type, (LPBYTE)Buf, &bufLen);
		::RegCloseKey(hk);
	}

	if(Util::stricmp(app.c_str(), Buf) != 0) {
		::RegCreateKey(HKEY_CLASSES_ROOT, _T("dchub"), &hk);
		TCHAR* tmp = _T("URL:Direct Connect Protocol");
		::RegSetValueEx(hk, NULL, 0, REG_SZ, (LPBYTE)tmp, _tcslen(tmp) + 1);
		::RegSetValueEx(hk, _T("URL Protocol"), 0, REG_SZ, (LPBYTE)_T(""), sizeof(TCHAR));
		::RegCloseKey(hk);

		::RegCreateKey(HKEY_CLASSES_ROOT, _T("dchub\\Shell\\Open\\Command"), &hk);
		::RegSetValueEx(hk, _T(""), 0, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length() + 1));
		::RegCloseKey(hk);

		::RegCreateKey(HKEY_CLASSES_ROOT, _T("dchub\\DefaultIcon"), &hk);
		app = Text::toT(Util::getAppName());
		::RegSetValueEx(hk, _T(""), 0, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length() + 1));
		::RegCloseKey(hk);
	}
}

 void WinUtil::registerMagnetHandler() {
	HKEY hk;
	TCHAR buf[512];
	tstring openCmd, magnetLoc, magnetExe;
	buf[0] = 0;
	bool haveMagnet = true;

	// what command is set up to handle magnets right now?
	if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("magnet\\shell\\open\\command"), 0, KEY_READ, &hk) == ERROR_SUCCESS) {
		DWORD bufLen = sizeof(TCHAR) * sizeof(buf);
		::RegQueryValueEx(hk, NULL, NULL, NULL, (LPBYTE)buf, &bufLen);
		::RegCloseKey(hk);
	}
	openCmd = buf;
	buf[0] = 0;
	// read the location of magnet.exe
	if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Magnet"), NULL, KEY_READ, &hk) == ERROR_SUCCESS) {
		DWORD bufLen = sizeof(buf) * sizeof(TCHAR);
		::RegQueryValueEx(hk, _T("Location"), NULL, NULL, (LPBYTE)buf, &bufLen);
		::RegCloseKey(hk);
	}
	magnetLoc = buf;
	string::size_type i;
	if (magnetLoc[0]==_T('"') && string::npos != (i = magnetLoc.find(_T('"'), 1))) {
		magnetExe = magnetLoc.substr(1, i-1);
	}
	// check for the existence of magnet.exe
	if(File::getSize(Text::fromT(magnetExe)) == -1) {
		magnetExe = Text::toT(Util::getAppPath() + "magnet.exe");
		if(File::getSize(Text::fromT(magnetExe)) == -1) {
			// gracefully fall back to registering DC++ to handle magnets
			magnetExe = Text::toT(Util::getAppName());
			haveMagnet = false;
		} else {
			// set Magnet\Location
			::RegCreateKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Magnet"), &hk);
			::RegSetValueEx(hk, _T("Location"), NULL, REG_SZ, (LPBYTE)magnetExe.c_str(), sizeof(TCHAR) * (magnetExe.length()+1));
			::RegCloseKey(hk);
		}
		magnetLoc = _T('"') + magnetExe + _T('"');
	}
	// (re)register the handler if magnet.exe isn't the default, or if DC++ is handling it
	if(BOOLSETTING(MAGNET_REGISTER) && (Util::strnicmp(openCmd, magnetLoc, magnetLoc.size()) != 0 || !haveMagnet)) {
		SHDeleteKey(HKEY_CLASSES_ROOT, _T("magnet"));
		::RegCreateKey(HKEY_CLASSES_ROOT, _T("magnet"), &hk);
		::RegSetValueEx(hk, NULL, NULL, REG_SZ, (LPBYTE)CTSTRING(MAGNET_SHELL_DESC), sizeof(TCHAR)*(TSTRING(MAGNET_SHELL_DESC).length()+1));
		::RegSetValueEx(hk, _T("URL Protocol"), NULL, REG_SZ, NULL, NULL);
		::RegCloseKey(hk);
		::RegCreateKey(HKEY_CLASSES_ROOT, _T("magnet\\DefaultIcon"), &hk);
		::RegSetValueEx(hk, NULL, NULL, REG_SZ, (LPBYTE)magnetLoc.c_str(), magnetLoc.length()+1);
		::RegCloseKey(hk);
		magnetLoc += _T(" %1");
		::RegCreateKey(HKEY_CLASSES_ROOT, _T("magnet\\shell\\open\\command"), &hk);
		::RegSetValueEx(hk, NULL, NULL, REG_SZ, (LPBYTE)magnetLoc.c_str(), magnetLoc.length()+1);
		::RegCloseKey(hk);
	}
	// magnet-handler specific code
	// clean out the DC++ tree first
	SHDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Magnet\\Handlers\\DC++"));
	// add DC++ to magnet-handler's list of applications
	::RegCreateKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Magnet\\Handlers\\DC++"), &hk);
	::RegSetValueEx(hk, NULL, NULL, REG_SZ, (LPBYTE)CTSTRING(MAGNET_HANDLER_ROOT), sizeof(TCHAR) * (TSTRING(MAGNET_HANDLER_ROOT).size()+1));
	::RegSetValueEx(hk, _T("Description"), NULL, REG_SZ, (LPBYTE)CTSTRING(MAGNET_HANDLER_DESC), sizeof(TCHAR) * (STRING(MAGNET_HANDLER_DESC).size()+1));
	// set ShellExecute
	tstring app = Text::toT("\"" + Util::getAppName() + "\" %URL");
	::RegSetValueEx(hk, _T("ShellExecute"), NULL, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length()+1));
	// set DefaultIcon
	app = Text::toT('"' + Util::getAppName() + '"');
	::RegSetValueEx(hk, _T("DefaultIcon"), NULL, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR)*(app.length()+1));
	::RegCloseKey(hk);

	// These two types contain a tth root, and are in common use.  The other two are variations picked up
	// from Shareaza's source, which come second hand from Gordon Mohr.  -GargoyleMT
	// Reference: http://forums.shareaza.com/showthread.php?threadid=23731
	// Note: the three part hash types require magnethandler >= 1.0.0.3
	DWORD nothing = 0;
	::RegCreateKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Magnet\\Handlers\\DC++\\Type"), &hk);
	::RegSetValueEx(hk, _T("urn:bitprint"), NULL, REG_DWORD, (LPBYTE)&nothing, sizeof(nothing));
	::RegSetValueEx(hk, _T("urn:tree:tiger"), NULL, REG_DWORD, (LPBYTE)&nothing, sizeof(nothing));
	::RegSetValueEx(hk, _T("urn:tree:tiger/"), NULL, REG_DWORD, (LPBYTE)&nothing, sizeof(nothing));
	::RegSetValueEx(hk, _T("urn:tree:tiger/1024"), NULL, REG_DWORD, (LPBYTE)&nothing, sizeof(nothing));
	::RegCloseKey(hk);
}

void WinUtil::openLink(const tstring& url) {
	CRegKey key;
	TCHAR regbuf[MAX_PATH];
	ULONG len = MAX_PATH;
	tstring x;

	tstring::size_type i = url.find(_T("://"));
	if(i != string::npos) {
		x = url.substr(0, i);
	} else {
		x = _T("http");
	}
	x += _T("\\shell\\open\\command");
	if(key.Open(HKEY_CLASSES_ROOT, x.c_str(), KEY_READ) == ERROR_SUCCESS) {
		if(key.QueryStringValue(NULL, regbuf, &len) == ERROR_SUCCESS) {
			/*
			 * Various values (for http handlers):
			 *  C:\PROGRA~1\MOZILL~1\FIREFOX.EXE -url "%1"
			 *  "C:\Program Files\Internet Explorer\iexplore.exe" -nohome
			 *  "C:\Apps\Opera7\opera.exe"
			 *  C:\PROGRAMY\MOZILLA\MOZILLA.EXE -url "%1"
			 *  C:\PROGRA~1\NETSCAPE\NETSCAPE\NETSCP.EXE -url "%1"
			 */
			tstring cmd(regbuf); // otherwise you consistently get two trailing nulls
			
			if(cmd.length() > 1) {
				string::size_type start,end;
				if(cmd[0] == '"') {
					start = 1;
					end = cmd.find('"', 1);
				} else {
					start = 0;
					end = cmd.find(' ', 1);
				}
				if(end == string::npos)
					end = cmd.length();

				tstring cmdLine(cmd);
				cmd = cmd.substr(start, end-start);
				size_t arg_pos;
				if((arg_pos = cmdLine.find(_T("%1"))) != string::npos) {
					cmdLine.replace(arg_pos, 2, url);
				} else {
					cmdLine.append(_T(" \"") + url + _T('\"'));
				}

				STARTUPINFO si = { sizeof(si), 0 };
				PROCESS_INFORMATION pi = { 0 };
				AutoArray<TCHAR> buf(cmdLine.length() + 1);
				_tcscpy(buf, cmdLine.c_str());
				if(::CreateProcess(cmd.c_str(), buf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
					::CloseHandle(pi.hThread);
					::CloseHandle(pi.hProcess);
					return;
				}
			}
		}
	}

	::ShellExecute(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void WinUtil::parseDchubUrl(const tstring& aUrl) {
	string server, file;
	short port = 411;
	Util::decodeUrl(Text::fromT(aUrl), server, port, file);
	if(!server.empty()) {
		HubFrame::openWindow(Text::toT(server + ":" + Util::toString(port)));
	}
	if(!file.empty()) {
		try {
			QueueManager::getInstance()->addList(ClientManager::getInstance()->getUser(file), QueueItem::FLAG_CLIENT_VIEW);
		} catch(const Exception&) {
			// ...
		}
	}
}

void WinUtil::parseMagnetUri(const tstring& aUrl, bool /*aOverride*/) {
	// official types that are of interest to us
	//  xt = exact topic
	//  xs = exact substitute
	//  as = acceptable substitute
	//  dn = display name
	if (Util::strnicmp(aUrl.c_str(), _T("magnet:?"), 8) == 0) {
		LogManager::getInstance()->message(STRING(MAGNET_DLG_TITLE) + ": " + Text::fromT(aUrl));
		StringTokenizer<tstring> mag(aUrl.substr(8), _T('&'));
		typedef map<tstring, tstring> MagMap;
		MagMap hashes;
		tstring fname, fhash, type, param;
		for(TStringList::iterator idx = mag.getTokens().begin(); idx != mag.getTokens().end(); ++idx) {
			// break into pairs
			string::size_type pos = idx->find(_T('='));
			if(pos != string::npos) {
				type = Text::toT(Text::toLower(Util::encodeURI(Text::fromT(idx->substr(0, pos)), true)));
				param = Text::toT(Util::encodeURI(Text::fromT(idx->substr(pos+1)), true));
			} else {
				type = Text::toT(Util::encodeURI(Text::fromT(*idx), true));
				param.clear();
			}
			// extract what is of value
			if(param.length() == 85 && Util::strnicmp(param.c_str(), _T("urn:bitprint:"), 13) == 0) {
				hashes[type] = param.substr(46);
			} else if(param.length() == 54 && Util::strnicmp(param.c_str(), _T("urn:tree:tiger:"), 15) == 0) {
				hashes[type] = param.substr(15);
			} else if(param.length() == 55 && Util::strnicmp(param.c_str(), _T("urn:tree:tiger/:"), 16) == 0) {
				hashes[type] = param.substr(16);
			} else if(param.length() == 59 && Util::strnicmp(param.c_str(), _T("urn:tree:tiger/1024:"), 20) == 0) {
				hashes[type] = param.substr(20);
			} else if(type.length() == 2 && Util::strnicmp(type.c_str(), _T("dn"), 2) == 0) {
				fname = param;
			}
		}
		// pick the most authoritative hash out of all of them.
		if(hashes.find(_T("as")) != hashes.end()) {
			fhash = hashes[_T("as")];
		}
		if(hashes.find(_T("xs")) != hashes.end()) {
			fhash = hashes[_T("xs")];
		}
		if(hashes.find(_T("xt")) != hashes.end()) {
			fhash = hashes[_T("xt")];
		}
		if(!fhash.empty()){
			// ok, we have a hash, and maybe a filename.
			//if(!BOOLSETTING(MAGNET_ASK)) {
			//	switch(SETTING(MAGNET_ACTION)) {
			//		case SettingsManager::MAGNET_AUTO_DOWNLOAD:
			//			break;
			//		case SettingsManager::MAGNET_AUTO_SEARCH:
			//			SearchFrame::openWindow(fhash, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_HASH);
			//			break;
			//	};
			//} else {
			// use aOverride to force the display of the dialog.  used for auto-updating
				CMagnetDlg dlg(fhash, fname);
				dlg.DoModal(mainWnd);
			//}
		} else {
			MessageBox(mainWnd, CTSTRING(MAGNET_DLG_TEXT_BAD), CTSTRING(MAGNET_DLG_TITLE), MB_OK | MB_ICONEXCLAMATION);
		}
	}
}

int WinUtil::textUnderCursor(POINT p, CEdit& ctrl, tstring& x) {
	
	int i = ctrl.CharFromPos(p);
	int line = ctrl.LineFromChar(i);
	int c = LOWORD(i) - ctrl.LineIndex(line);
	int len = ctrl.LineLength(i) + 1;
	if(len < 3) {
		return 0;
	}

	TCHAR* buf = new TCHAR[len];
	ctrl.GetLine(line, buf, len);
	x = tstring(buf, len-1);
	delete[] buf;

	string::size_type start = x.find_last_of(_T(" <\t\r\n"), c);
	if(start == string::npos)
		start = 0;
	else
		start++;

	return start;
}

bool WinUtil::parseDBLClick(const tstring& aString, string::size_type start, string::size_type end) {
	if( (Util::strnicmp(aString.c_str() + start, _T("http://"), 7) == 0) || 
		(Util::strnicmp(aString.c_str() + start, _T("www."), 4) == 0) ||
		(Util::strnicmp(aString.c_str() + start, _T("ftp://"), 6) == 0) ||
		(Util::strnicmp(aString.c_str() + start, _T("irc://"), 6) == 0) ||
		(Util::strnicmp(aString.c_str() + start, _T("https://"), 8) == 0) )	
	{

		openLink(aString.substr(start, end-start));
		return true;
	} else if(Util::strnicmp(aString.c_str() + start, _T("dchub://"), 8) == 0) {
		parseDchubUrl(aString.substr(start, end-start));
		return true;
	} else if(Util::strnicmp(aString.c_str() + start, _T("magnet:?"), 8) == 0) {
		parseMagnetUri(aString.substr(start, end-start));
		return true;
	}
	return false;
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

int WinUtil::getIconIndex(const tstring& aFileName) {
	if(BOOLSETTING(USE_SYSTEM_ICONS)) {
		SHFILEINFO fi;
		string x = Text::toLower(Util::getFileExt(Text::fromT(aFileName)));
		if(!x.empty()) {
			ImageIter j = fileIndexes.find(x);
			if(j != fileIndexes.end())
				return j->second;
		}
		tstring fn = Text::toT(Text::toLower(Util::getFileName(Text::fromT(aFileName))));
		::SHGetFileInfo(fn.c_str(), FILE_ATTRIBUTE_NORMAL, &fi, sizeof(fi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
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
 * $Id: WinUtil.cpp,v 1.61 2004/09/27 12:02:44 arnetheduck Exp $
 */
