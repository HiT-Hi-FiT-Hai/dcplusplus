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

#ifndef APPEARANCEPAGE_H
#define APPEARANCEPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"
#include "ExListViewCtrl.h"

class AppearancePage : public CPropertyPage<IDD_APPEARANCEPAGE>, public PropPage
{
public:
	AppearancePage(SettingsManager *s) : PropPage(s) { 
		SetTitle(CSTRING(SETTINGS_APPEARANCE));
	};

	virtual ~AppearancePage();

	BEGIN_MSG_MAP(AppearancePage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		COMMAND_ID_HANDLER(IDC_SELTEXT, onClickedText)
		COMMAND_ID_HANDLER(IDC_SELWINCOLOR, onClickedBackground)
		COMMAND_ID_HANDLER(IDC_BROWSE, onBrowse)
		COMMAND_ID_HANDLER(IDC_SETTINGS_UPLOAD_BAR_COLOR, onPickColor)
		COMMAND_ID_HANDLER(IDC_SETTINGS_DOWNLOAD_BAR_COLOR, onPickColor)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedText(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedBackground(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onCtlColor(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onPickColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);


	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
	
protected:
	string EncodeFont(LOGFONT const& font);
	void DecodeFont(string setting, LOGFONT &dest);

	static Item items[];
	static TextItem texts[];
	static ListItem listItems[];

	CStatic ctrlExample;
	COLORREF fg, bg, upBar, downBar;
	HBRUSH bgbrush;
	HFONT fontObj;
	LOGFONT font;
};

#endif //APPEARANCEPAGE_H

/**
 * @file
 * $Id: AppearancePage.h,v 1.9 2003/12/04 10:31:41 arnetheduck Exp $
 */
