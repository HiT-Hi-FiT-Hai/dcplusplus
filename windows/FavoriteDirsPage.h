/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(FAVORITE_DIRS_PAGE_H)
#define FAVORITE_DIRS_PAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"
#include "ExListViewCtrl.h"
#include "WinUtil.h"

class FavoriteDirsPage : public CPropertyPage<IDD_FAVORITE_DIRSPAGE>, public PropPage
{
public:
	FavoriteDirsPage(SettingsManager *s) : PropPage(s) {
		SetTitle(CTSTRING(SETTINGS_FAVORITE_DIRS_PAGE));
		m_psp.dwFlags |= PSP_HASHELP | PSP_RTLREADING;
	}
	virtual ~FavoriteDirsPage() {
		ctrlDirectories.Detach();
	}

	BEGIN_MSG_MAP(FavoriteDirsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		MESSAGE_HANDLER(WM_HELP, onHelp)
		MESSAGE_HANDLER(WM_DROPFILES, onDropFiles)
		NOTIFY_HANDLER(IDC_FAVORITE_DIRECTORIES, LVN_ITEMCHANGED, onItemchangedDirectories)
		COMMAND_ID_HANDLER(IDC_ADD, onClickedAdd)
		COMMAND_ID_HANDLER(IDC_REMOVE, onClickedRemove)
		COMMAND_ID_HANDLER(IDC_RENAME, onClickedRename)
		NOTIFY_CODE_HANDLER_EX(PSN_HELP, onHelpInfo)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onItemchangedDirectories(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT onClickedAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClickedRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClickedRename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onHelpInfo(LPNMHDR);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
	
protected:
	static TextItem texts[];
	ExListViewCtrl ctrlDirectories;

	void addDirectory(const tstring& aPath);
};

#endif // !defined(FAVORITE_DIR_SPAGE_H)

/**
 * @file
 * $Id: FavoriteDirsPage.h,v 1.8 2006/02/19 16:19:06 arnetheduck Exp $
 */
