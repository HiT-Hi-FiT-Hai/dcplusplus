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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "PropertiesDlg.h"

#include "GeneralPage.h"
#include "DownloadPage.h"
#include "UploadPage.h"
#include "AppearancePage.h"
#include "AdvancedPage.h"
#include "Advanced2Page.h"
#include "UCPage.h"
#include "FavoriteDirsPage.h"

PropertiesDlg::PropertiesDlg(SettingsManager *s) : TreePropertySheet(CTSTRING(SETTINGS))
{
	pages[0] = new GeneralPage(s);
	pages[1] = new DownloadPage(s);
	pages[2] = new UploadPage(s);
	pages[3] = new AppearancePage(s);
	pages[4] = new Advanced2Page(s);
	pages[5] = new AdvancedPage(s);
	pages[6] = new UCPage(s);
	pages[7] = new FavoriteDirsPage(s);

	for(int i=0; i<numPages; i++) {
		AddPage(pages[i]->getPSP());
	}

	// Hide "Apply" button
	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	m_psh.dwFlags &= ~PSH_HASHELP;
}

PropertiesDlg::~PropertiesDlg()
{
	for(int i=0; i<numPages; i++) {
		delete pages[i];
	}
}

void PropertiesDlg::write()
{
	for(int i=0; i<numPages; i++)
	{
		// Check HWND of page to see if it has been created
		const HWND page = PropSheet_IndexToHwnd((HWND)*this, i);

		if(page != NULL)
			pages[i]->write();
	}
}

LRESULT PropertiesDlg::onOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	write();
	bHandled = FALSE;
	return TRUE;
}

/**
 * @file
 * $Id: PropertiesDlg.cpp,v 1.10 2004/11/02 11:03:06 arnetheduck Exp $
 */

