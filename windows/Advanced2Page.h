/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

#include "PropPage.h"

class Advanced2Page : public CPropertyPage<IDD_ADVANCED2PAGE>, public PropPage
{
public:
	Advanced2Page(SettingsManager *s) : PropPage(s) { };
	~Advanced2Page() { };

	BEGIN_MSG_MAP(Advanced2Page)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_HANDLER(IDC_BROWSE_LOG, BN_CLICKED, onClickedBrowseDir)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onClickedBrowseDir(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

protected:
	static Item items[];
};

#endif //ADVANCED2PAGE_H

/**
 * @file Advanced2Page.h
 * $Id: Advanced2Page.h,v 1.1 2002/04/09 18:46:32 arnetheduck Exp $
 */

