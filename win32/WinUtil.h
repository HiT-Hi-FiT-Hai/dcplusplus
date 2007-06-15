/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_WIN32_WIN_UTIL_H
#define DCPLUSPLUS_WIN32_WIN_UTIL_H

#include <client/StringTokenizer.h>
#include <client/Util.h>
#include <client/forward.h>
#include <client/MerkleTree.h>

class UserInfoBase {
public:
	UserInfoBase(const UserPtr& u) : user(u) { }

	void getList() { };
	void browseList() { };
	void matchQueue() { };
	void pm() { };
	void grant() { };
	void addFav() { };
	void removeAll() { };

	UserPtr& getUser() { return user; }
	UserPtr user;
};

#ifdef PORT_ME
#include "../client/Util.h"
#include "../client/SettingsManager.h"
#include "../client/User.h"
#include "../client/MerkleTree.h"

// Some utilities for handling HLS colors, taken from Jean-Michel LE FOL's codeproject
// article on WTL OfficeXP Menus
typedef DWORD HLSCOLOR;
#define HLS(h,l,s) ((HLSCOLOR)(((BYTE)(h)|((WORD)((BYTE)(l))<<8))|(((DWORD)(BYTE)(s))<<16)))
#define HLS_H(hls) ((BYTE)(hls))
#define HLS_L(hls) ((BYTE)(((WORD)(hls)) >> 8))
#define HLS_S(hls) ((BYTE)((hls)>>16))

HLSCOLOR RGB2HLS (COLORREF rgb);
COLORREF HLS2RGB (HLSCOLOR hls);

COLORREF HLS_TRANSFORM (COLORREF rgb, int percent_L, int percent_S);

template<class T>
class UserInfoBaseHandler {
public:
	BEGIN_MSG_MAP(UserInfoBaseHandler)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_BROWSELIST, onBrowseList)
		COMMAND_ID_HANDLER(IDC_MATCH_QUEUE, onMatchQueue)
		COMMAND_ID_HANDLER(IDC_PRIVATEMESSAGE, onPrivateMessage)
		COMMAND_ID_HANDLER(IDC_ADD_TO_FAVORITES, onAddToFavorites)
		COMMAND_ID_HANDLER(IDC_GRANTSLOT, onGrantSlot)
		COMMAND_ID_HANDLER(IDC_REMOVEALL, onRemoveAll)
	END_MSG_MAP()

	LRESULT onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::matchQueue);
		return 0;
	}
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::getList);
		return 0;
	}
	LRESULT onBrowseList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::browseList);
		return 0;
	}
	LRESULT onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::addFav);
		return 0;
	}
	LRESULT onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::pm);
		return 0;
	}
	LRESULT onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::grant);
		return 0;
	}
	LRESULT onRemoveAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::removeAll);
		return 0;
	}

	struct ADCOnly {
		ADCOnly() : adcOnly(true) { }
		void operator()(UserInfoBase* ui) { if(ui->getUser()->isSet(User::NMDC)) adcOnly = false; }

		bool adcOnly;
	};
	void checkAdcItems(CMenu& menu) {

		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_STATE;
		if(((T*)this)->getUserList().forEachSelectedT(ADCOnly()).adcOnly) {
			menu.EnableMenuItem(IDC_BROWSELIST, MFS_ENABLED);
		} else {
			menu.EnableMenuItem(IDC_BROWSELIST, MFS_DISABLED);
		}
	}

	void appendUserItems(CMenu& menu) {
		menu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
		menu.AppendMenu(MF_STRING, IDC_BROWSELIST, CTSTRING(BROWSE_FILE_LIST));
		menu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
		menu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CTSTRING(SEND_PRIVATE_MESSAGE));
		menu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));
		menu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT));
		menu.AppendMenu(MF_STRING, IDC_CONNECT, CTSTRING(CONNECT_FAVUSER_HUB));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, IDC_REMOVEALL, CTSTRING(REMOVE_FROM_ALL));
	}
};

class FlatTabCtrl;
class UserCommand;

#endif

class WinUtil {
public:
	static tstring tth;

	static HBRUSH bgBrush;
	static COLORREF textColor;
	static COLORREF bgColor;
	static SmartWin::FontPtr font;
	static SmartWin::FontPtr monoFont;
	static tstring commands;
	static SmartWin::ImageListPtr fileImages;
	static SmartWin::ImageListPtr userImages;
	static int fileImageCount;
	static int dirIconIndex;
	static int dirMaskedIndex;
	static StringList lastDirs;

	static void init();
	static void uninit();

	static tstring encodeFont(LOGFONT const& font);
	static void decodeFont(const tstring& setting, LOGFONT &dest);

	/**
	 * Check if this is a common /-command.
	 * @param cmd The whole text string, will be updated to contain only the command.
	 * @param param Set to any parameters.
	 * @param message Message that should be sent to the chat.
	 * @param status Message that should be shown in the status line.
	 * @return True if the command was processed, false otherwise.
	 */
	static bool checkCommand(tstring& cmd, tstring& param, tstring& message, tstring& status);

	static void openFile(const tstring& file);
	static void openFolder(const tstring& file);

	template<typename T>
	static std::vector<int> splitTokens(const string& str, const T& defaults) {
		const size_t n = sizeof(defaults) / sizeof(defaults[0]);
		std::vector<int> ret(defaults, defaults + n);
		StringTokenizer<string> tokens(str, ',') ;
		const StringList& l = tokens.getTokens();
		for(size_t i = 0; i < std::min(ret.size(), l.size()); ++i) {
			ret[i] = Util::toInt(l[i]);
		}
		return ret;			
	}
	static std::string toString(const std::vector<int>& tokens);
	static void splitTokens(int* array, const string& tokens, int maxItems = -1) throw();
	
	static int getIconIndex(const tstring& aFileName);
	static int getDirIconIndex() { return dirIconIndex; }
	static int getDirMaskedIndex() { return dirMaskedIndex; }

	static bool isShift() { return (::GetKeyState(VK_SHIFT) & 0x8000) > 0; }
	static bool isAlt() { return (::GetKeyState(VK_MENU) & 0x8000) > 0; }
	static bool isCtrl() { return (::GetKeyState(VK_CONTROL) & 0x8000) > 0; }

	static tstring getNicks(const CID& cid) throw();
	static tstring getNicks(const UserPtr& u);
	/** @return Pair of hubnames as a string and a bool representing the user's online status */
	static pair<tstring, bool> getHubNames(const CID& cid) throw();
	static pair<tstring, bool> getHubNames(const UserPtr& u);

	// Hash related
	static void bitziLink(const TTHValue& /*aHash*/);
	static void copyMagnet(const TTHValue& /*aHash*/, const tstring& /*aFile*/);
	static void searchHash(const TTHValue& /*aHash*/);

	static tstring escapeMenu(tstring str);

	static void addLastDir(const string& dir);

	static bool browseFile(tstring& target, HWND owner = NULL, bool save = true, const tstring& initialDir = Util::emptyStringT, const TCHAR* types = NULL, const TCHAR* defExt = NULL);
	static bool browseDirectory(tstring& target, HWND owner = NULL);

#ifdef PORT_ME
	static CImageList userImages;

	typedef HASH_MAP<string, int> ImageMap;
	typedef ImageMap::iterator ImageIter;
	static ImageMap fileIndexes;
	static int fontHeight;
	static HFONT boldFont;
	static HFONT systemFont;
	static HFONT monoFont;
	static HWND mainWnd;
	static HWND mdiClient;
	static FlatTabCtrl* tabCtrl;
	static HHOOK hook;
	static DWORD helpCookie;

	static string getAppName() {
		TCHAR buf[MAX_PATH+1];
		DWORD x = GetModuleFileName(NULL, buf, MAX_PATH);
		return Text::fromT(tstring(buf, x));
	}

	static bool getVersionInfo(OSVERSIONINFOEX& ver);

	static int getTextWidth(const tstring& str, HWND hWnd) {
		HDC dc = ::GetDC(hWnd);
		int sz = getTextWidth(str, dc);
		::ReleaseDC(mainWnd, dc);
		return sz;
	}
	static int getTextWidth(const tstring& str, HDC dc) {
		SIZE sz = { 0, 0 };
		::GetTextExtentPoint32(dc, str.c_str(), str.length(), &sz);
		return sz.cx;
	}

	static void setClipboard(const tstring& str);

	static tstring getHelpFile() {
		return Text::toT(Util::getDataPath() + "DCPlusPlus.chm");
	}


	// URL related
	static void registerDchubHandler();
	static void registerADChubHandler();
	static void registerMagnetHandler();
	static void unRegisterDchubHandler();
	static void unRegisterADChubHandler();
	static void unRegisterMagnetHandler();
	static void parseDchubUrl(const tstring& /*aUrl*/);
	static void parseADChubUrl(const tstring& /*aUrl*/);
	static void parseMagnetUri(const tstring& /*aUrl*/, bool aOverride = false);
	static bool parseDBLClick(const tstring& /*aString*/, string::size_type start, string::size_type end);
	static bool urlDcADCRegistered;
	static bool urlMagnetRegistered;
	static int textUnderCursor(POINT p, CEdit& ctrl, tstring& x);
	static void openLink(const tstring& url);

	static double toBytes(TCHAR* aSize);

	static int getOsMajor();
	static int getOsMinor();

	//returns the position where the context menu should be
	//opened if it was invoked from the keyboard.
	//aPt is relative to the screen not the control.
	static void getContextMenuPos(CListViewCtrl& aList, POINT& aPt);
	static void getContextMenuPos(CTreeViewCtrl& aTree, POINT& aPt);
	static void getContextMenuPos(CEdit& aEdit,			POINT& aPt);

	static bool getUCParams(HWND parent, const UserCommand& cmd, StringMap& sm) throw();


	static void saveHeaderOrder(CListViewCtrl& ctrl, SettingsManager::StrSetting order,
		SettingsManager::StrSetting widths, int n, int* indexes, int* sizes) throw();

	template<class T> static HWND hiddenCreateEx(T& p) throw() {
		HWND active = (HWND)::SendMessage(mdiClient, WM_MDIGETACTIVE, 0, 0);
		::LockWindowUpdate(mdiClient);
		HWND ret = p.CreateEx(mdiClient);
		if(active && ::IsWindow(active))
			::SendMessage(mdiClient, WM_MDIACTIVATE, (WPARAM)active, 0);
		::LockWindowUpdate(0);
		return ret;
	}
	template<class T> static HWND hiddenCreateEx(T* p) throw() {
		return hiddenCreateEx(*p);
	}

#endif
};

#endif // !defined(WIN_UTIL_H)
