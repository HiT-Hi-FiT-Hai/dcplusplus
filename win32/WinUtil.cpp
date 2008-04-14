/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#include "WinUtil.h"

#include "resource.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/ShareManager.h>
#include <dcpp/ClientManager.h>
#include <dcpp/HashManager.h>
#include <dcpp/LogManager.h>
#include <dcpp/QueueManager.h>
#include <dcpp/StringTokenizer.h>
#include <dcpp/version.h>
#include <dcpp/File.h>
#include <dcpp/UserCommand.h>

#include "LineDlg.h"
#include "MagnetDlg.h"
#include "HubFrame.h"
#include "SearchFrame.h"
#include "MainWindow.h"

tstring WinUtil::tth;
dwt::BrushPtr WinUtil::bgBrush;
COLORREF WinUtil::textColor = 0;
COLORREF WinUtil::bgColor = 0;
dwt::FontPtr WinUtil::font;
dwt::FontPtr WinUtil::monoFont;
dwt::ImageListPtr WinUtil::fileImages;
dwt::ImageListPtr WinUtil::userImages;
int WinUtil::fileImageCount;
int WinUtil::dirIconIndex;
int WinUtil::dirMaskedIndex;
TStringList WinUtil::lastDirs;
MainWindow* WinUtil::mainWindow = 0;
bool WinUtil::urlDcADCRegistered = false;
bool WinUtil::urlMagnetRegistered = false;
WinUtil::ImageMap WinUtil::fileIndexes;
DWORD WinUtil::helpCookie = 0;

const dwt::Button::Seed WinUtil::Seeds::button;
const ComboBox::Seed WinUtil::Seeds::comboBoxStatic;
const ComboBox::Seed WinUtil::Seeds::comboBoxEdit;
const dwt::WidgetMenu::Seed WinUtil::Seeds::menu;
const dwt::Table::Seed WinUtil::Seeds::Table;
const TextBox::Seed WinUtil::Seeds::textBox;
const dwt::Tree::Seed WinUtil::Seeds::treeView;

void WinUtil::init() {

	SettingsManager::getInstance()->setDefault(SettingsManager::BACKGROUND_COLOR, (int)(GetSysColor(COLOR_WINDOW)));
	SettingsManager::getInstance()->setDefault(SettingsManager::TEXT_COLOR, (int)(GetSysColor(COLOR_WINDOWTEXT)));

	textColor = SETTING(TEXT_COLOR);
	bgColor = SETTING(BACKGROUND_COLOR);
	bgBrush = dwt::BrushPtr(new dwt::Brush(bgColor));

	LOGFONT lf;
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
	SettingsManager::getInstance()->setDefault(SettingsManager::TEXT_FONT, Text::fromT(encodeFont(lf)));
	decodeFont(Text::toT(SETTING(TEXT_FONT)), lf);

	font = dwt::FontPtr(new dwt::Font(::CreateFontIndirect(&lf), true));
	monoFont = dwt::FontPtr(new dwt::Font((BOOLSETTING(USE_OEM_MONOFONT) ? dwt::OemFixedFont : dwt::AnsiFixedFont)));

	fileImages = dwt::ImageListPtr(new dwt::ImageList(16, 16, ILC_COLOR32 | ILC_MASK));

	dirIconIndex = fileImageCount++;
	dirMaskedIndex = fileImageCount++;
	
	if(BOOLSETTING(USE_SYSTEM_ICONS)) {
		SHFILEINFO fi;
		::SHGetFileInfo(_T("."), FILE_ATTRIBUTE_DIRECTORY, &fi, sizeof(fi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
		dwt::Icon tmp(fi.hIcon);
		fileImages->add(tmp);
		// @todo This one should be masked further for the incomplete folder thing
		fileImages->add(tmp);
	} else {
		dwt::Bitmap tmp(IDB_FOLDERS);
		fileImages->add(tmp, RGB(255, 0, 255));

		// Unknown file
		fileImageCount++;
	}

	{
		userImages = dwt::ImageListPtr(new dwt::ImageList(16, 16, ILC_COLOR32 | ILC_MASK));
		dwt::Bitmap tmp(IDB_USERS);
		userImages->add(tmp, RGB(255, 0, 255));
	}
	
	if(BOOLSETTING(URL_HANDLER)) {
		registerDchubHandler();
		registerADChubHandler();
		urlDcADCRegistered = true;
	}
	if(BOOLSETTING(MAGNET_REGISTER)) {
		registerMagnetHandler();
		urlMagnetRegistered = true;
	}
	
	// Const so that noone else will change them after they've been initialized
	dwt::Button::Seed& xbutton = const_cast<dwt::Button::Seed&>(Seeds::button);
	ComboBox::Seed& xcomboBoxEdit = const_cast<ComboBox::Seed&>(Seeds::comboBoxEdit);
	ComboBox::Seed& xcomboBoxStatic = const_cast<ComboBox::Seed&>(Seeds::comboBoxStatic);
	dwt::WidgetMenu::Seed& xmenu = const_cast<dwt::WidgetMenu::Seed&>(Seeds::menu);
	dwt::Table::Seed& xTable = const_cast<dwt::Table::Seed&>(Seeds::Table);
	TextBox::Seed& xtextBox = const_cast<TextBox::Seed&>(Seeds::textBox);
	dwt::Tree::Seed& xtreeView =  const_cast<dwt::Tree::Seed&>(Seeds::treeView);

	xbutton.font = font;

	xcomboBoxStatic.style |= CBS_DROPDOWNLIST;
	xcomboBoxStatic.font = font;

	xcomboBoxEdit.style |= CBS_DROPDOWN | CBS_AUTOHSCROLL;
	xcomboBoxEdit.font = font;

	if(BOOLSETTING(OWNER_DRAWN_MENUS)) {
		xmenu.colorInfo.colorImageBackground = RGB(255, 0, 255); // DC++ bitmaps use RGB(255, 0, 255) as their background (transparent) color
		xmenu.font = font;
	} else
		xmenu.ownerDrawn = false;

	xTable.style |= WS_HSCROLL | WS_VSCROLL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS;
	xTable.exStyle = WS_EX_CLIENTEDGE;
	xTable.lvStyle = LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER;
	xTable.font = font;

	xtextBox.exStyle = WS_EX_CLIENTEDGE;
	xtextBox.font = font;

	xtreeView.style |= TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP;
	xtreeView.exStyle = WS_EX_CLIENTEDGE;
	xtreeView.font = font;

	::HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD)&helpCookie);
}

void WinUtil::uninit() {
	::HtmlHelp(NULL, NULL, HH_UNINITIALIZE, helpCookie);
}

tstring WinUtil::encodeFont(LOGFONT const& font)
{
	tstring res(font.lfFaceName);
	res += _T(',');
	res += Text::toT(Util::toString(font.lfHeight));
	res += _T(',');
	res += Text::toT(Util::toString(font.lfWeight));
	res += _T(',');
	res += Text::toT(Util::toString(font.lfItalic));
	return res;
}

std::string WinUtil::toString(const std::vector<int>& tokens) {
	std::string ret;
	for(std::vector<int>::const_iterator i = tokens.begin(); i != tokens.end(); ++i) {
		ret += Util::toString(*i) + ',';
	}
	if(!ret.empty())
		ret.erase(ret.size()-1);
	return ret;
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

#define LINE2 _T("-- http://dcplusplus.sourceforge.net <DC++ ") _T(VERSIONSTRING) _T(">")
const TCHAR *msgs[] = { _T("\r\n-- I'm a happy dc++ user. You could be happy too.\r\n") LINE2,
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

tstring WinUtil::commands = _T("/refresh, /me <msg>, /slots #, /search <string>, /dc++, /away <msg>, /back, /g <searchstring>, /imdb <imdbquery>, /u <url>, /rebuild");

bool WinUtil::checkCommand(tstring& cmd, tstring& param, tstring& message, tstring& status, bool& thirdPerson) {
	string::size_type i = cmd.find(' ');
	if(i != string::npos) {
		param = cmd.substr(i+1);
		cmd = cmd.substr(1, i - 1);
	} else {
		cmd = cmd.substr(1);
	}

	if(Util::stricmp(cmd.c_str(), _T("log")) == 0) {
		if(Util::stricmp(param.c_str(), _T("system")) == 0) {
			WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + "system.log")));
		} else if(Util::stricmp(param.c_str(), _T("downloads")) == 0) {
			WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatTime(SETTING(LOG_FILE_DOWNLOAD), time(NULL)))));
		} else if(Util::stricmp(param.c_str(), _T("uploads")) == 0) {
			WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatTime(SETTING(LOG_FILE_UPLOAD), time(NULL)))));
		} else {
			return false;
		}
	} else if(Util::stricmp(cmd.c_str(), _T("me")) == 0) {
		message = param;
		thirdPerson = true;
	} else if(Util::stricmp(cmd.c_str(), _T("refresh"))==0) {
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
			status = T_("Slots set");
			ClientManager::getInstance()->infoUpdated();
		} else {
			status = T_("Invalid number of slots");
		}
	} else if(Util::stricmp(cmd.c_str(), _T("search")) == 0) {
		if(!param.empty()) {
			SearchFrame::openWindow(mainWindow->getTabView(), param);
		} else {
			status = T_("Specify a search string");
		}
	} else if(Util::stricmp(cmd.c_str(), _T("dc++")) == 0) {
		message = msgs[GET_TICK() % MSGS];
	} else if(Util::stricmp(cmd.c_str(), _T("away")) == 0) {
		if(Util::getAway() && param.empty()) {
			Util::setAway(false);
			Util::setManualAway(false);
			status = T_("Away mode off");
		} else {
			Util::setAway(true);
			Util::setManualAway(true);
			Util::setAwayMessage(Text::fromT(param));
			status = str(TF_("Away mode on: %1%") % Text::toT(Util::getAwayMessage()));
		}
	} else if(Util::stricmp(cmd.c_str(), _T("back")) == 0) {
		Util::setAway(false);
		Util::setManualAway(false);
		status = T_("Away mode off");
	} else if(Util::stricmp(cmd.c_str(), _T("g")) == 0) {
		if(param.empty()) {
			status = T_("Specify a search string");
		} else {
			WinUtil::openLink(_T("http://www.google.com/search?q=") + Text::toT(Util::encodeURI(Text::fromT(param))));
		}
	} else if(Util::stricmp(cmd.c_str(), _T("imdb")) == 0) {
		if(param.empty()) {
			status = T_("Specify a search string");
		} else {
			WinUtil::openLink(_T("http://www.imdb.com/find?q=") + Text::toT(Util::encodeURI(Text::fromT(param))));
		}
	} else if(Util::stricmp(cmd.c_str(), _T("u")) == 0) {
		if (param.empty()) {
			status = T_("Specify a URL");
		} else {
			WinUtil::openLink(Text::toT(Util::encodeURI(Text::fromT(param))));
		}
	} else if(Util::stricmp(cmd.c_str(), _T("rebuild")) == 0) {
		HashManager::getInstance()->rebuild();
	} else {
		return false;
	}

	return true;
}

void WinUtil::openFile(const tstring& file) {
	::ShellExecute(NULL, NULL, file.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void WinUtil::openFolder(const tstring& file) {
	if (File::getSize(Text::fromT(file)) != -1)
		::ShellExecute(NULL, NULL, Text::toT("explorer.exe").c_str(), Text::toT("/e, /select, \"" + (Text::fromT(file)) + "\"").c_str(), NULL, SW_SHOWNORMAL);
	else
		::ShellExecute(NULL, NULL, Text::toT("explorer.exe").c_str(), Text::toT("/e, \"" + Util::getFilePath(Text::fromT(file)) + "\"").c_str(), NULL, SW_SHOWNORMAL);
}

tstring WinUtil::getNicks(const CID& cid) throw() {
	return Text::toT(Util::toString(ClientManager::getInstance()->getNicks(cid)));
}
tstring WinUtil::getNicks(const UserPtr& u) { 
	return getNicks(u->getCID()); 
}

pair<tstring, bool> WinUtil::getHubNames(const CID& cid) throw() {
	StringList hubs = ClientManager::getInstance()->getHubNames(cid);
	if(hubs.empty()) {
		return make_pair(T_("Offline"), false);
	} else {
		return make_pair(Text::toT(Util::toString(hubs)), true);
	}
}

pair<tstring, bool> WinUtil::getHubNames(const UserPtr& u) { 
	return getHubNames(u->getCID()); 
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
		if(!fi.hIcon) {
			return 2;
		}
		try {
			dwt::Icon tmp(fi.hIcon);
			fileImages->add(tmp);

			fileIndexes[x] = fileImageCount++;
			return fileImageCount - 1;
		} catch(const dwt::xCeption&) {
			return 2;
		}
	} else {
		return 2;
	}
}
void WinUtil::addHashItems(const dwt::WidgetMenu::ObjectType& menu, const TTHValue& tth, const tstring& filename) {
	menu->appendItem(IDC_SEARCH_ALTERNATES, T_("Search for alternates"), std::tr1::bind(&WinUtil::searchHash, tth));
	menu->appendItem(IDC_BITZI_LOOKUP, T_("Lookup TTH at Bitzi.com"), std::tr1::bind(WinUtil::bitziLink, tth));
	menu->appendItem(IDC_COPY_MAGNET, T_("Copy magnet link to clipboard"), std::tr1::bind(&WinUtil::copyMagnet, tth, filename));
}

void WinUtil::bitziLink(const TTHValue& aHash) {
	// to use this free service by bitzi, we must not hammer or request information from bitzi
	// except when the user requests it (a mass lookup isn't acceptable), and (if we ever fetch
	// this data within DC++, we must identify the client/mod in the user agent, so abuse can be
	// tracked down and the code can be fixed
	openLink(_T("http://bitzi.com/lookup/tree:tiger:") + Text::toT(aHash.toBase32()));
}

void WinUtil::copyMagnet(const TTHValue& aHash, const tstring& aFile) {
	if(!aFile.empty()) {
		setClipboard(_T("magnet:?xt=urn:tree:tiger:") + Text::toT(aHash.toBase32()) + _T("&dn=") + Text::toT(Util::encodeURI(Text::fromT(aFile))));
	}
}

void WinUtil::searchHash(const TTHValue& aHash) {
	SearchFrame::openWindow(mainWindow->getTabView(), Text::toT(aHash.toBase32()), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
}

void WinUtil::addLastDir(const tstring& dir) {
	TStringIter i = find(lastDirs.begin(), lastDirs.end(), dir); 
	if(i != lastDirs.end()) {
		lastDirs.erase(i);
	}
	while(lastDirs.size() >= 10) {
		lastDirs.erase(lastDirs.begin());
	}
	lastDirs.push_back(dir);
}

bool WinUtil::browseSaveFile(dwt::SaveDialog dlg, tstring& file) {
	tstring ext = Util::getFileExt(file);
	tstring path = Util::getFilePath(file);
	
	if(!ext.empty()) {
		ext = ext.substr(1); // remove leading dot so default extension works when browsing for file
		dlg.addFilter(str(TF_("%1% files") % ext), _T("*.") + ext);
		dlg.setDefaultExtension(ext);
	}
	dlg.addFilter(T_("All files"), _T("*.*"));
	dlg.setInitialDirectory(path);
	
	return dlg.open(file);
}

bool WinUtil::browseFileList(dwt::LoadDialog dialog, tstring& file) {
	return dialog.addFilter(T_("File Lists"), _T("*.xml.bz2"))
		.addFilter(T_("All files"), _T("*.*"))
		.setInitialDirectory(Text::toT(Util::getListPath()))
		.open(file);
}

int WinUtil::getOsMajor() {
	OSVERSIONINFOEX ver;
	memset(&ver, 0, sizeof(OSVERSIONINFOEX));
	if(!GetVersionEx((OSVERSIONINFO*)&ver))
	{
		ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	}
	GetVersionEx((OSVERSIONINFO*)&ver);
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	return ver.dwMajorVersion;
}

int WinUtil::getOsMinor() {
	OSVERSIONINFOEX ver;
	memset(&ver, 0, sizeof(OSVERSIONINFOEX));
	if(!GetVersionEx((OSVERSIONINFO*)&ver))
	{
		ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	}
	GetVersionEx((OSVERSIONINFO*)&ver);
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	return ver.dwMinorVersion;
}

void WinUtil::setClipboard(const tstring& str) {
	if(!mainWindow)
		return;
	
	if(!::OpenClipboard(mainWindow->handle())) {
		return;
	}

	EmptyClipboard();

#ifdef UNICODE
	OSVERSIONINFOEX ver;
	if( WinUtil::getVersionInfo(ver) ) {
		if( ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
			string tmp = Text::wideToAcp(str);

			HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (tmp.size() + 1) * sizeof(char));
			if (hglbCopy == NULL) {
				CloseClipboard();
				return;
			}

			// Lock the handle and copy the text to the buffer.
			char* lptstrCopy = (char*)GlobalLock(hglbCopy);
			strcpy(lptstrCopy, tmp.c_str());
			GlobalUnlock(hglbCopy);

			SetClipboardData(CF_TEXT, hglbCopy);

			CloseClipboard();

			return;
		}
	}
#endif

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
#ifdef UNICODE
	SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
	SetClipboardData(CF_TEXT, hglbCopy);
#endif

	CloseClipboard();
}

bool WinUtil::getUCParams(dwt::Widget* parent, const UserCommand& uc, StringMap& sm) throw() {
	string::size_type i = 0;
	StringMap done;

	while( (i = uc.getCommand().find("%[line:", i)) != string::npos) {
		i += 7;
		string::size_type j = uc.getCommand().find(']', i);
		if(j == string::npos)
			break;

		string name = uc.getCommand().substr(i, j-i);
		if(done.find(name) == done.end()) {
			LineDlg dlg(parent, Text::toT(uc.getName()), Text::toT(name), Text::toT(sm["line:" + name]));
			if(dlg.run() == IDOK) {
				done[name] = sm["line:" + name] = Text::fromT(dlg.getLine());
			} else {
				return false;
			}
		}
		i = j + 1;
	}
	return true;
}

void WinUtil::help(HWND hWnd, unsigned id) {
	dcdebug("WinUtil::help; hWnd: %p; id: %u\n", hWnd, id);

	string path = Util::getDataPath() + "DCPlusPlus.chm";
	if(File::getSize(path) == -1)
		return;
	tstring helpFile = Text::toT(path);

	if(id >= IDH_CSHELP_BEGIN && id <= IDH_CSHELP_END) {
		// context-sensitive help; display a tooltip
		HH_POPUP popup = { 0 };
		popup.cbStruct = sizeof(HH_POPUP);
		popup.idString = id;

		RECT rect;
		::GetWindowRect(hWnd, &rect);
		popup.pt.x = (rect.left + rect.right) / 2;
		popup.pt.y = rect.top;

		popup.rcMargins.left = 5;
		popup.rcMargins.top = 5;
		popup.rcMargins.right = 5;
		popup.rcMargins.bottom = 5;

		::HtmlHelp(hWnd, helpFile.c_str(), HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD_PTR>(&popup));
	} else
		::HtmlHelp(hWnd, helpFile.c_str(), id ? HH_HELP_CONTEXT : HH_DISPLAY_TOC, id);
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

#ifdef PORT_ME

#define COMPILE_MULTIMON_STUBS 1
#include <MultiMon.h>

HLSCOLOR RGB2HLS (COLORREF rgb) {
	unsigned char minval = min(GetRValue(rgb), min(GetGValue(rgb), GetBValue(rgb)));
	unsigned char maxval = max(GetRValue(rgb), max(GetGValue(rgb), GetBValue(rgb)));
	float mdiff = float(maxval) - float(minval);
	float msum  = float(maxval) + float(minval);

	float luminance = msum / 510.0f;
	float saturation = 0.0f;
	float hue = 0.0f;

	if ( maxval != minval ) {
		float rnorm = (maxval - GetRValue(rgb) ) / mdiff;
		float gnorm = (maxval - GetGValue(rgb) ) / mdiff;
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
	if		(rh > 360.0f) rh -= 360.0f;
	else if (rh <   0.0f) rh += 360.0f;

	if		(rh <  60.0f) rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;
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
#endif

void WinUtil::registerDchubHandler() {
	HKEY hk;
	TCHAR Buf[512];
	tstring app = _T("\"") + Text::toT(getAppName()) + _T("\" %1");
	Buf[0] = 0;

	if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("dchub\\Shell\\Open\\Command"), 0, KEY_WRITE | KEY_READ, &hk) == ERROR_SUCCESS) {
		DWORD bufLen = sizeof(Buf);
		DWORD type;
		::RegQueryValueEx(hk, NULL, 0, &type, (LPBYTE)Buf, &bufLen);
		::RegCloseKey(hk);
	}

	if(Util::stricmp(app.c_str(), Buf) != 0) {
		if (::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("dchub"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL)) {
			LogManager::getInstance()->message(_("Error creating dchub registry key"));
			return;
		}

		TCHAR tmp[] = _T("URL:Direct Connect Protocol");
		::RegSetValueEx(hk, NULL, 0, REG_SZ, (LPBYTE)tmp, sizeof(TCHAR) * (_tcslen(tmp) + 1));
		::RegSetValueEx(hk, _T("URL Protocol"), 0, REG_SZ, (LPBYTE)_T(""), sizeof(TCHAR));
		::RegCloseKey(hk);

		::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("dchub\\Shell\\Open\\Command"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
		::RegSetValueEx(hk, _T(""), 0, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length() + 1));
		::RegCloseKey(hk);

		::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("dchub\\DefaultIcon"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
		app = Text::toT(getAppName());
		::RegSetValueEx(hk, _T(""), 0, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length() + 1));
		::RegCloseKey(hk);
	}
}

 void WinUtil::unRegisterDchubHandler() {
	SHDeleteKey(HKEY_CLASSES_ROOT, _T("dchub"));
 }

 void WinUtil::registerADChubHandler() {
	 HKEY hk;
	 TCHAR Buf[512];
	 tstring app = _T("\"") + Text::toT(getAppName()) + _T("\" %1");
	 Buf[0] = 0;

	 if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("adc\\Shell\\Open\\Command"), 0, KEY_WRITE | KEY_READ, &hk) == ERROR_SUCCESS) {
		 DWORD bufLen = sizeof(Buf);
		 DWORD type;
		 ::RegQueryValueEx(hk, NULL, 0, &type, (LPBYTE)Buf, &bufLen);
		 ::RegCloseKey(hk);
	 }

	 if(Util::stricmp(app.c_str(), Buf) != 0) {
		 if (::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("adc"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL)) {
			 LogManager::getInstance()->message(_("Error creating adc registry key"));
			 return;
		 }

		 TCHAR tmp[] = _T("URL:Direct Connect Protocol");
		 ::RegSetValueEx(hk, NULL, 0, REG_SZ, (LPBYTE)tmp, sizeof(TCHAR) * (_tcslen(tmp) + 1));
		 ::RegSetValueEx(hk, _T("URL Protocol"), 0, REG_SZ, (LPBYTE)_T(""), sizeof(TCHAR));
		 ::RegCloseKey(hk);

		 ::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("adc\\Shell\\Open\\Command"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
		 ::RegSetValueEx(hk, _T(""), 0, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length() + 1));
		 ::RegCloseKey(hk);

		 ::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("adc\\DefaultIcon"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
		 app = Text::toT(getAppName());
		 ::RegSetValueEx(hk, _T(""), 0, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length() + 1));
		 ::RegCloseKey(hk);
	 }
 }

 void WinUtil::unRegisterADChubHandler() {
	SHDeleteKey(HKEY_CLASSES_ROOT, _T("adc"));
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
		magnetExe = Text::toT(Util::getDataPath() + "magnet.exe");
		if(File::getSize(Text::fromT(magnetExe)) == -1) {
			// gracefully fall back to registering DC++ to handle magnets
			magnetExe = Text::toT(getAppName());
			haveMagnet = false;
		} else {
			// set Magnet\Location
			if (::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Magnet"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL)) {
				LogManager::getInstance()->message(_("Error creating magnet registry key"));
				return;
			}

			::RegSetValueEx(hk, _T("Location"), NULL, REG_SZ, (LPBYTE)magnetExe.c_str(), sizeof(TCHAR) * (magnetExe.length()+1));
			::RegCloseKey(hk);
		}
		magnetLoc = _T('"') + magnetExe + _T('"');
	}
	// (re)register the handler if magnet.exe isn't the default, or if DC++ is handling it
	if(BOOLSETTING(MAGNET_REGISTER) && (Util::strnicmp(openCmd, magnetLoc, magnetLoc.size()) != 0 || !haveMagnet)) {
		SHDeleteKey(HKEY_CLASSES_ROOT, _T("magnet"));
		if (::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("magnet"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL)) {
			LogManager::getInstance()->message(_("Error creating magnet registry key"));
			return;
		}
		::RegSetValueEx(hk, NULL, NULL, REG_SZ, (LPBYTE)CT_("URL:MAGNET URI"), sizeof(TCHAR)*(T_("URL:MAGNET URI").length()+1));
		::RegSetValueEx(hk, _T("URL Protocol"), NULL, REG_SZ, NULL, NULL);
		::RegCloseKey(hk);
		::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("magnet\\DefaultIcon"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
		::RegSetValueEx(hk, NULL, NULL, REG_SZ, (LPBYTE)magnetLoc.c_str(), sizeof(TCHAR)*(magnetLoc.length()+1));
		::RegCloseKey(hk);
		magnetLoc += _T(" %1");
		::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("magnet\\shell\\open\\command"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
		::RegSetValueEx(hk, NULL, NULL, REG_SZ, (LPBYTE)magnetLoc.c_str(), sizeof(TCHAR)*(magnetLoc.length()+1));
		::RegCloseKey(hk);
	}
	// magnet-handler specific code
	// clean out the DC++ tree first
	SHDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Magnet\\Handlers\\DC++"));
	// add DC++ to magnet-handler's list of applications
	::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Magnet\\Handlers\\DC++"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
	::RegSetValueEx(hk, NULL, NULL, REG_SZ, (LPBYTE)CT_("DC++"), sizeof(TCHAR) * (T_("DC++").size()+1));
	::RegSetValueEx(hk, _T("Description"), NULL, REG_SZ, (LPBYTE)CT_("Download files from the Direct Connect network"), sizeof(TCHAR) * (T_("Download files from the Direct Connect network").size()+1));
	// set ShellExecute
	tstring app = Text::toT("\"" + getAppName() + "\" %URL");
	::RegSetValueEx(hk, _T("ShellExecute"), NULL, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length()+1));
	// set DefaultIcon
	app = Text::toT('"' + getAppName() + '"');
	::RegSetValueEx(hk, _T("DefaultIcon"), NULL, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR)*(app.length()+1));
	::RegCloseKey(hk);

	// These two types contain a tth root, and are in common use.  The other two are variations picked up
	// from Shareaza's source, which come second hand from Gordon Mohr.  -GargoyleMT
	// Reference: http://forums.shareaza.com/showthread.php?threadid=23731
	// Note: the three part hash types require magnethandler >= 1.0.0.3
	DWORD nothing = 0;
	::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Magnet\\Handlers\\DC++\\Type"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
	::RegSetValueEx(hk, _T("urn:bitprint"), NULL, REG_DWORD, (LPBYTE)&nothing, sizeof(nothing));
	::RegSetValueEx(hk, _T("urn:tree:tiger"), NULL, REG_DWORD, (LPBYTE)&nothing, sizeof(nothing));
	::RegSetValueEx(hk, _T("urn:tree:tiger/"), NULL, REG_DWORD, (LPBYTE)&nothing, sizeof(nothing));
	::RegSetValueEx(hk, _T("urn:tree:tiger/1024"), NULL, REG_DWORD, (LPBYTE)&nothing, sizeof(nothing));
	::RegCloseKey(hk);
}

void WinUtil::unRegisterMagnetHandler() {
	SHDeleteKey(HKEY_CLASSES_ROOT, _T("magnet"));
	SHDeleteKey(HKEY_LOCAL_MACHINE, _T("magnet"));
}

void WinUtil::openLink(const tstring& url) {
#ifdef PORT_ME
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
				boost::scoped_ptr<TCHAR> buf(new TCHAR[cmdLine.length() + 1]);
				_tcscpy(&buf[0], cmdLine.c_str());
				if(::CreateProcess(cmd.c_str(), &buf[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
					::CloseHandle(pi.hThread);
					::CloseHandle(pi.hProcess);
					return;
				}
			}
		}
	}
#endif
	::ShellExecute(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

bool WinUtil::parseDBLClick(const tstring& aString) {
	if( (Util::strnicmp(aString.c_str(), _T("http://"), 7) == 0) ||
		(Util::strnicmp(aString.c_str(), _T("www."), 4) == 0) ||
		(Util::strnicmp(aString.c_str(), _T("ftp://"), 6) == 0) ||
		(Util::strnicmp(aString.c_str(), _T("irc://"), 6) == 0) ||
		(Util::strnicmp(aString.c_str(), _T("https://"), 8) == 0) ||
		(Util::strnicmp(aString.c_str(), _T("mailto:"), 7) == 0) )
	{

		openLink(aString);
		return true;
	} else if(Util::strnicmp(aString.c_str(), _T("dchub://"), 8) == 0) {
		parseDchubUrl(aString);
		return true;
	} else if(Util::strnicmp(aString.c_str(), _T("magnet:?"), 8) == 0) {
		parseMagnetUri(aString);
		return true;
	} else if(Util::strnicmp(aString.c_str(), _T("adc://"), 6) == 0) {
		parseADChubUrl(aString);
		return true;
	}
	return false;
}


void WinUtil::parseDchubUrl(const tstring& aUrl) {
	string server, file;
	uint16_t port = 411;
	Util::decodeUrl(Text::fromT(aUrl), server, port, file);
	if(!server.empty()) {
		HubFrame::openWindow(mainWindow->getTabView(), server + ":" + Util::toString(port));
	}
	if(!file.empty()) {
		if(file[0] == '/') // Remove any '/' in from of the file
			file = file.substr(1);
		try {
			UserPtr user = ClientManager::getInstance()->findLegacyUser(file);
			if(user)
				QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
			// @todo else report error
		} catch(const Exception&) {
			// ...
		}
	}
}

void WinUtil::parseADChubUrl(const tstring& aUrl) {
	string server, file;
	uint16_t port = 0; //make sure we get a port since adc doesn't have a standard one
	Util::decodeUrl(Text::fromT(aUrl), server, port, file);
	if(!server.empty() && port > 0) {
		HubFrame::openWindow(mainWindow->getTabView(), "adc://" + server + ":" + Util::toString(port));
	}
}

void WinUtil::parseMagnetUri(const tstring& aUrl, bool /*aOverride*/) {
	// official types that are of interest to us
	//  xt = exact topic
	//  xs = exact substitute
	//  as = acceptable substitute
	//  dn = display name
	if (Util::strnicmp(aUrl.c_str(), _T("magnet:?"), 8) == 0) {
		LogManager::getInstance()->message(str(F_("MAGNET Link detected: %1%") % Text::fromT(aUrl)));
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
				MagnetDlg(mainWindow, fhash, fname).run();
			//}
		} else {
			dwt::MessageBox(mainWindow).show(T_("A MAGNET link was given to DC++, but it didn't contain a valid file hash for use on the Direct Connect network.  No action will be taken."), T_("MAGNET Link detected"), dwt::MessageBox::BOX_OK, dwt::MessageBox::BOX_ICONEXCLAMATION);
		}
	}
}
#ifdef PORT_ME

double WinUtil::toBytes(TCHAR* aSize) {
	double bytes = _tstof(aSize);

	if (_tcsstr(aSize, CT_("PiB"))) {
		return bytes * 1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0;
	} else if (_tcsstr(aSize, CT_("TiB"))) {
		return bytes * 1024.0 * 1024.0 * 1024.0 * 1024.0;
	} else if (_tcsstr(aSize, CT_("GiB"))) {
		return bytes * 1024.0 * 1024.0 * 1024.0;
	} else if (_tcsstr(aSize, CT_("MiB"))) {
		return bytes * 1024.0 * 1024.0;
	} else if (_tcsstr(aSize, CT_("KiB"))) {
		return bytes * 1024.0;
	} else {
		return bytes;
	}
}
#endif
