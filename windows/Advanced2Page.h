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

#ifndef ADVANCED2PAGE_H
#define ADVANCED2PAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"

class Advanced2Page : public CPropertyPage<IDD_ADVANCED2PAGE>, public PropPage
{
public:
	Advanced2Page(SettingsManager *s) : PropPage(s) { 
		SetTitle(CTSTRING(SETTINGS_LOGS));
		 m_psp.dwFlags |= PSP_HASHELP;
	};

	virtual ~Advanced2Page() { };

	BEGIN_MSG_MAP(Advanced2Page)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		MESSAGE_HANDLER(WM_HELP, onHelp)
		COMMAND_ID_HANDLER(IDC_BROWSE_LOG, onClickedBrowseDir)
		COMMAND_ID_HANDLER(IDC_LOG_MAIN_CHAT, onUpdateEdits)
		COMMAND_ID_HANDLER(IDC_LOG_PRIVATE_CHAT, onUpdateEdits)
		COMMAND_ID_HANDLER(IDC_LOG_DOWNLOADS, onUpdateEdits)
		COMMAND_ID_HANDLER(IDC_LOG_UPLOADS, onUpdateEdits)
		NOTIFY_CODE_HANDLER_EX(PSN_HELP, onHelpInfo)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClickedBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onHelpInfo(LPNMHDR /*pnmh*/);

	LRESULT onUpdateEdits(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		updateEdits();
		return 0;
	}
	
	void updateEdits();
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

protected:
	static Item items[];
	static TextItem texts[];
};

#endif //ADVANCED2PAGE_H

/**
 * @file
 * $Id: Advanced2Page.h,v 1.10 2004/09/27 12:02:41 arnetheduck Exp $
 */

