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

#include <dcpp/StringTokenizer.h>
#include <dcpp/Util.h>
#include <dcpp/forward.h>
#include <dcpp/MerkleTree.h>

#ifdef PORT_ME
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

#endif

class MainWindow;

class WinUtil {
public:
	static tstring tth;

	static SmartWin::BrushPtr bgBrush;
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
	static TStringList lastDirs;
	static MainWindow* mainWindow;
	//static SmartWin::WidgetTabView* mdiParent;
	static DWORD helpCookie;
	
	typedef unordered_map<string, int> ImageMap;
	typedef ImageMap::iterator ImageIter;
	static ImageMap fileIndexes;
	
	struct Seeds {
		static const SmartWin::WidgetButton::Seed button;
		static const SmartWin::WidgetComboBox::Seed comboBoxStatic;
		static const SmartWin::WidgetComboBox::Seed comboBoxEdit;
		static const SmartWin::WidgetListView::Seed listView;
		static const SmartWin::WidgetMenuExtended::Seed menuExtended;
		static const SmartWin::WidgetTextBox::Seed textBox;
		static const SmartWin::WidgetTreeView::Seed treeView;
	};
	
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
	
	template<typename T>
	static TStringList getStrings(const T& t) {
		const size_t n = sizeof(t) / sizeof(t[0]);
		TStringList ret(n);
		for(size_t i = 0; i < n; ++i) {
			ret[i] = T_(t[i]);
		}
		return ret;
	}

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
	static void addHashItems(const SmartWin::WidgetMenu::ObjectType& menu, const TTHValue& tth, const tstring& filename);
	static void bitziLink(const TTHValue& /*aHash*/);
	static void copyMagnet(const TTHValue& /*aHash*/, const tstring& /*aFile*/);
	static void searchHash(const TTHValue& /*aHash*/);

	static void addLastDir(const tstring& dir);

	static void openLink(const tstring& url);
	static bool browseFile(tstring& target, HWND owner = NULL, bool save = true, const tstring& initialDir = Util::emptyStringT, const TCHAR* types = NULL, const TCHAR* defExt = NULL);
	static bool browseDirectory(tstring& target, HWND owner = NULL);

	static int getOsMajor();
	static int getOsMinor();

	static void setClipboard(const tstring& str);

	static bool getUCParams(SmartWin::Widget* parent, const UserCommand& cmd, StringMap& sm) throw();
	
	static bool parseDBLClick(const tstring& aString);
	static void parseDchubUrl(const tstring& /*aUrl*/);
	static void parseADChubUrl(const tstring& /*aUrl*/);
	static void parseMagnetUri(const tstring& /*aUrl*/, bool aOverride = false);

	static tstring getHelpFile() {
		return Text::toT(Util::getDataPath() + "DCPlusPlus.chm");
	}

	static bool getVersionInfo(OSVERSIONINFOEX& ver);

	// URL related
	static void registerDchubHandler();
	static void registerADChubHandler();
	static void registerMagnetHandler();
	static void unRegisterDchubHandler();
	static void unRegisterADChubHandler();
	static void unRegisterMagnetHandler();
	static bool urlDcADCRegistered;
	static bool urlMagnetRegistered;

	static string getAppName() {
		TCHAR buf[MAX_PATH+1];
		DWORD x = GetModuleFileName(NULL, buf, MAX_PATH);
		return Text::fromT(tstring(buf, x));
	}

#ifdef PORT_ME
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

	static int textUnderCursor(POINT p, CEdit& ctrl, tstring& x);

	static double toBytes(TCHAR* aSize);

#endif
};

#endif // !defined(WIN_UTIL_H)
