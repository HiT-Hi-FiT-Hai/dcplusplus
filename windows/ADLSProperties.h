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

#if !defined(AFX_ADLSPROPERTIES_H__12B4C4DD_D28F_47FB_AFE8_8C75E5C4FF96__INCLUDED_)
#define AFX_ADLSPROPERTIES_H__12B4C4DD_D28F_47FB_AFE8_8C75E5C4FF96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ADLSearch;

///////////////////////////////////////////////////////////////////////////////
//
//	Dialog for new/edit ADL searches
//
///////////////////////////////////////////////////////////////////////////////
class ADLSProperties : public CDialogImpl<ADLSProperties>  
{
public:

	// Constructor/destructor
	ADLSProperties::ADLSProperties(ADLSearch *_search) : search(_search) { };
	virtual ~ADLSProperties() { };

	// Dilaog unique id
	enum { IDD = IDD_ADLS_PROPERTIES };
	
	// Inline message map
	BEGIN_MSG_MAP(ADLSProperties)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()
	
	// Message handlers
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:

	// Current search
	ADLSearch* search;
};

#endif // !defined(AFX_ADLSPROPERTIES_H__12B4C4DD_D28F_47FB_AFE8_8C75E5C4FF96__INCLUDED_)

/**
 * @file
 * $Id: ADLSProperties.h,v 1.2 2004/09/06 12:32:43 arnetheduck Exp $
 */
