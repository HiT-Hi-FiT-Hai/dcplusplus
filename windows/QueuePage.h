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

#ifndef QUEUEPAGE_H
#define QUEUEPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"
#include "ExListViewCtrl.h"

class QueuePage : public CPropertyPage<IDD_QUEUEPAGE>, public PropPage
{
public:
	QueuePage(SettingsManager *s) : PropPage(s) { 
		SetTitle(CTSTRING(SETTINGS_QUEUE));
		m_psp.dwFlags |= PSP_HASHELP;
	};

	virtual ~QueuePage() { 
	};

	BEGIN_MSG_MAP(QueuePage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		NOTIFY_CODE_HANDLER_EX(PSN_HELP, onHelpInfo)
		MESSAGE_HANDLER(WM_HELP, onHelp)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onHelpInfo(LPNMHDR /*pnmh*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
	
protected:

	static Item items[];
	static TextItem texts[];
};

#endif //QUEUEPAGE_H
