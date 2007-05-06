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

#ifndef DCPLUSPLUS_WIN32_LINE_DLG_H
#define DCPLUSPLUS_WIN32_LINE_DLG_H

#include "resource.h"

#include <client/Util.h>

class LineDlg : public SmartWin::WidgetFactory<SmartWin::WidgetModalDialog, LineDlg, SmartWin::MessageMapPolicyModalDialogWidget>
{
public:
	LineDlg(SmartWin::Widget* parent, const tstring& title_, const tstring& desc_, bool password_ = false, const tstring& initial_ = Util::emptyStringT);
	
	int run() { return createDialog(IDD_LINE); }
	
	tstring getLine() { return initial; }
private:
	WidgetStaticPtr description;
	WidgetTextBoxPtr line;
	WidgetButtonPtr ok;
	WidgetButtonPtr cancel;

	tstring title;
	tstring desc;
	tstring initial;
	bool password;

	void focus();
	bool initDialog();
	bool closing();
	void okClicked(WidgetButtonPtr);
	void cancelClicked(WidgetButtonPtr);
};

#endif // !defined(LINE_DLG_H)
