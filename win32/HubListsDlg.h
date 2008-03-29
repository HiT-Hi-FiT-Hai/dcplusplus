/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_WIN32_HUB_LISTS_DLG_H
#define DCPLUSPLUS_WIN32_HUB_LISTS_DLG_H

#include "WidgetFactory.h"

class HubListsDlg : public WidgetFactory<SmartWin::WidgetModalDialog>
{
public:
	HubListsDlg(SmartWin::Widget* parent);
	virtual ~HubListsDlg();

	int run() { return createDialog(IDD_HUB_LIST); }

private:
	TextBoxPtr editBox;
	TablePtr hubLists;

	bool handleInitDialog();
	void handleFocus();
	void handleDoubleClick();
	bool handleKeyDown(int c);
	void handleAddClicked();
	void handleMoveUpClicked();
	void handleMoveDownClicked();
	void handleEditClicked();
	void handleRemoveClicked();
	void handleOKClicked();

	void addHubList(const tstring& address, int index = -1);
};

#endif // !defined(DCPLUSPLUS_WIN32_HUB_LISTS_DLG_H)
