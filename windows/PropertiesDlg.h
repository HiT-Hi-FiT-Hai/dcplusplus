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

#if !defined(AFX_PROPERTIESDLG_H__9B8B3ABC_D165_47D8_AA4B_AF695F7A7D54__INCLUDED_)
#define AFX_PROPERTIESDLG_H__9B8B3ABC_D165_47D8_AA4B_AF695F7A7D54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"

class PropertiesDlg : public CPropertySheet
{
public:
	enum { numPages = 6 };

	BEGIN_MSG_MAP(PropertiesDlg)
		COMMAND_ID_HANDLER(IDOK, onOK)
	END_MSG_MAP()

	PropertiesDlg(SettingsManager *s);
	virtual ~PropertiesDlg();

	LRESULT onOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

protected:
	void write();

	PropPage *pages[numPages];
};

#endif // !defined(AFX_PROPERTIESDLG_H__9B8B3ABC_D165_47D8_AA4B_AF695F7A7D54__INCLUDED_)

/**
 * @file
 * $Id: PropertiesDlg.h,v 1.5 2003/07/15 14:53:12 arnetheduck Exp $
 */
