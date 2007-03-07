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

class LineDlg : public SmartWin::WidgetFactory<SmartWin::WidgetModalDialog, LineDlg, SmartWin::MessageMapPolicyModalDialogWidget>
{
public:
	LineDlg(SmartWin::Widget* parent, const tstring& title_, const tstring& desc_, bool password_ = false, const tstring& initial_ = "") : SmartWin::Widget(parent), title(title_), desc(desc_), initial(initial_), password(password_) {
		onInitDialog(&LineDlg::initDialog);
		onFocus(&LineDlg::focus);	
	}
	
	int run() { return createDialog(IDD_LINE); }
	
	tstring getLine() { return line->getText(); }
private:
	WidgetStaticPtr description;
	WidgetTextBoxPtr line;
	WidgetButtonPtr ok;
	WidgetButtonPtr cancel;

	tstring title;
	tstring desc;
	tstring initial;
	bool password;

	void focus() {
		line->setFocus();
	}
	
	bool initDialog() {
		ok = subclassButton(IDOK);
		ok->onClicked(&LineDlg::okClicked);

		cancel = subclassButton(IDCANCEL);
		cancel->onClicked(&LineDlg::cancelClicked);
		
		description = subclassStatic(IDC_DESCRIPTION);
		description->setText(desc);
		line = subclassTextBox(IDC_LINE);
		line->setFocus();
		line->setText(initial);
		line->setSelection();
		if(password) {
			line->setPassword();
		}
		
		setText(title);

#ifdef PORT_ME
		CenterWindow(GetParent());
#endif
		return false;
	}
	
	bool closing() {
		endDialog(IDCANCEL);
		return false;		
	}
	
	void okClicked(WidgetButtonPtr) {
		endDialog(IDOK);
	}
	void cancelClicked(WidgetButtonPtr) {
		endDialog(IDCANCEL);
	}
};

#endif // !defined(LINE_DLG_H)
