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

#ifndef __WINUTIL_H
#define __WINUTIL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/Util.h"
#include "../client/SettingsManager.h"

class FlatTabCtrl;

class WinUtil {
public:
	static CImageList fileImages;
	typedef HASH_MAP<string, int> ImageMap;
	typedef ImageMap::iterator ImageIter;
	static ImageMap fileIndexes;
	static HBRUSH bgBrush;
	static COLORREF textColor;
	static COLORREF bgColor;
	static HFONT font;
	static int fontHeight;
	static HFONT boldFont;
	static CMenu mainMenu;
	static int dirIconIndex;
	static StringList lastDirs;
	static string lastKick;
	static string lastRedirect;
	static string lastServer;
	static HWND mainWnd;
	static FlatTabCtrl* tabCtrl;
	static string commands;

	static void buildMenu();
	static void decodeFont(const string& setting, LOGFONT &dest);

	/**
	 * Check if this is a common /-command.
	 * @param cmd The whole text string, will be updated to contain only the command.
	 * @param param Set to any parameters.
	 * @param message Message that should be sent to the chat.
	 * @param status Message that should be shown in the status line.
	 * @return True if the command was processed, false otherwise.
	 */
	static bool checkCommand(HWND mdiClient, string& cmd, string& param, string& message, string& status);

	static int getTextWidth(const string& str, HWND hWnd) {
		HDC dc = ::GetDC(hWnd);
		int sz = getTextWidth(str, dc);
		::ReleaseDC(mainWnd, dc);
		return sz;
	}
	static int getTextWidth(const string& str, HDC dc) {
		SIZE sz = { 0, 0 };
		::GetTextExtentPoint32(dc, str.c_str(), str.length(), &sz);
		return sz.cx;		
	}

	static int getTextHeight(HWND wnd, HFONT fnt) {
		HDC dc = ::GetDC(wnd);
		int h = getTextHeight(dc, fnt);
		::ReleaseDC(wnd, dc);
		return h;
	}

	static int getTextHeight(HDC dc, HFONT fnt) {
		HGDIOBJ old = ::SelectObject(dc, fnt);
		int h = getTextHeight(dc);
		::SelectObject(dc, old);
		return h;
	}

	static int getTextHeight(HDC dc) {
		TEXTMETRIC tm;
		::GetTextMetrics(dc, &tm);
		return tm.tmHeight;
	}

	static void addLastDir(const string& dir) {
		if(find(lastDirs.begin(), lastDirs.end(), dir) != lastDirs.end()) {
			return;
		}
		if(lastDirs.size() == 10) {
			lastDirs.erase(lastDirs.begin());
		}
		lastDirs.push_back(dir);
	}
	
	static string encodeFont(LOGFONT const& font)
	{
		string res(font.lfFaceName);
		res += ',';
		res += Util::toString(font.lfHeight);
		res += ',';
		res += Util::toString(font.lfWeight);
		res += ',';
		res += Util::toString(font.lfItalic);
		return res;
	}
	
	static bool browseFile(string& target, HWND owner = NULL, bool save = true, const string& initialDir = Util::emptyString);
	static bool browseDirectory(string& target, HWND owner = NULL);

	static int getIconIndex(const string& aFileName) {
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
			CImageList il = (HIMAGELIST)::SHGetFileInfo(aFileName.c_str(), FILE_ATTRIBUTE_NORMAL, &fi, sizeof(fi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
			while(il.GetImageCount() > fileImages.GetImageCount()) {
				HICON hi = il.GetIcon(fileImages.GetImageCount());
				fileImages.AddIcon(hi);
				DestroyIcon(hi);
			}
			fileIndexes[x] = fi.iIcon;
			return fi.iIcon;
		} else {
			return 2;
		}
	}

	static int getDirIconIndex() {
		return dirIconIndex;
	}
	
	static void splitTokens(int* array, const string& tokens, int maxItems = -1);
	static void saveHeaderOrder(CListViewCtrl& ctrl, SettingsManager::StrSetting order, SettingsManager::StrSetting widths, int n, int* indexes, int* sizes);
	
private:
	static int CALLBACK browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM /*lp*/, LPARAM pData);		
	
};

#endif // __WINUTIL_H

/**
 * @file
 * $Id: WinUtil.h,v 1.10 2003/05/13 11:34:07 arnetheduck Exp $
 */
