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

#ifndef DOWNLOADPAGE_H
#define DOWNLOADPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"

class DownloadPage : public CPropertyPage<IDD_DOWNLOADPAGE>, public PropPage
{
public:
	DownloadPage(SettingsManager *s) : PropPage(s) { };
	virtual ~DownloadPage() { };

	BEGIN_MSG_MAP(DownloadPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_BROWSEDIR, onClickedBrowseDir)
		COMMAND_ID_HANDLER(IDC_BROWSETEMPDIR, onClickedBrowseTempDir)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onClickedBrowseDir(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedBrowseTempDir(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
	virtual void setTitle(const string& t) { SetTitle(t.c_str()); };
	
protected:
	static Item items[];
};

#endif //DOWNLOADPAGE_H

/**
 * @file
 * $Id: DownloadPage.h,v 1.7 2003/09/22 13:17:24 arnetheduck Exp $
 */
