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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "CommandDlg.h"

LRESULT CommandDlg::onType(WORD , WORD wID, HWND , BOOL& ) {
	int type = 0;
	if(IsDlgButtonChecked(IDC_SETTINGS_SEPARATOR)) {
		type = 0;
	} else if(IsDlgButtonChecked(IDC_SETTINGS_RAW)) {
		type = 1;
	} else if(IsDlgButtonChecked(IDC_SETTINGS_CHAT)) {
		type = 2;
	} else if(IsDlgButtonChecked(IDC_SETTINGS_PM)) {
		type = 3;
	}
	switch(type) {
		case 0:
			::EnableWindow(GetDlgItem(IDC_NAME), FALSE);
			::EnableWindow(GetDlgItem(IDC_COMMAND), FALSE);
			::EnableWindow(GetDlgItem(IDC_NICK), FALSE);
			break;
		case 1:
		case 2:
			::EnableWindow(GetDlgItem(IDC_NAME), TRUE);
			::EnableWindow(GetDlgItem(IDC_COMMAND), TRUE);
			::EnableWindow(GetDlgItem(IDC_NICK), FALSE);
			break;
		case 4:
			::EnableWindow(GetDlgItem(IDC_NAME), TRUE);
			::EnableWindow(GetDlgItem(IDC_COMMAND), TRUE);
			::EnableWindow(GetDlgItem(IDC_NICK), FALSE);
			break;
	}
	return 0;
}

/**
* @file
* $Id: CommandDlg.cpp,v 1.1 2003/10/21 17:12:21 arnetheduck Exp $
*/
