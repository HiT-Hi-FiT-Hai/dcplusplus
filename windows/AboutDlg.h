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

#if !defined(AFX_ABOUTDLG_H__D12815FA_21C0_4C20_9718_892C9F8CD196__INCLUDED_)
#define AFX_ABOUTDLG_H__D12815FA_21C0_4C20_9718_892C9F8CD196__INCLUDED_

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		SetDlgItemText(IDC_VERSION, "DC++ v" VERSIONSTRING "\n(c) Copyright 2001-2002 Jacek Sieka\n\nhttp://dcplusplus.sourceforge.net/");
		SetDlgItemText(IDC_THANKS, "Thanks go out to sourceforge for hosting the project and nro for hosting the first and (so far) only mirror...more thanks go out to the people testing the application and to all those who have been discussing it on sourceforge and all over the world...you probably know who you are...also, thanks to the bzip2 team, they're the ones that made the bzip2 compression library used in DC++");
		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
};

#endif // !defined(AFX_ABOUTDLG_H__D12815FA_21C0_4C20_9718_892C9F8CD196__INCLUDED_)

/**
 * @file AboutDlg.h
 * $Id: AboutDlg.h,v 1.3 2002/04/22 13:58:15 arnetheduck Exp $
 */

