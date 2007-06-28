/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_WIN32_APPEARANCE_2_PAGE_H
#define DCPLUSPLUS_WIN32_APPEARANCE_2_PAGE_H

#include "PropPage.h"
#include "WidgetFactory.h"

class Appearance2Page : public WidgetFactory<SmartWin::WidgetDialog, Appearance2Page, SmartWin::MessageMapPolicyDialogWidget>, public PropPage
{
public:
	Appearance2Page(SmartWin::Widget* parent);
	virtual ~Appearance2Page();

#ifdef PORT_ME
	BEGIN_MSG_MAP(Appearance2Page)
		MESSAGE_HANDLER(WM_HELP, onHelp)
		NOTIFY_CODE_HANDLER_EX(PSN_HELP, onHelpInfo)
	END_MSG_MAP()

	LRESULT onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onHelpInfo(LPNMHDR /*pnmh*/);
#endif

	virtual void write();

private:
	static Item items[];
	static TextItem texts[];

	WidgetStaticPtr example;

	COLORREF fg, bg, upBar, downBar;
	HBRUSH bgbrush;
	HFONT fontObj;
	LOGFONT font;

	HRESULT handleExampleColor(WidgetStaticPtr, LPARAM /*lParam*/, WPARAM wParam);
	void handleBackgroundClicked(WidgetButtonPtr);
	void handleTextClicked(WidgetButtonPtr);
	void handleULClicked(WidgetButtonPtr);
	void handleDLClicked(WidgetButtonPtr);
	void handleBrowseClicked(WidgetButtonPtr);
};

#endif // !defined(DCPLUSPLUS_WIN32_APPEARANCE_2_PAGE_H)
