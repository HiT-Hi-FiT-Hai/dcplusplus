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
 
#include "stdafx.h"
#include <client/DCPlusPlus.h>

#include "LineDlg.h"

LineDlg::LineDlg(SmartWin::Widget* parent, const tstring& title_, const tstring& desc_, const tstring& initial_, bool password_) : 
	SmartWin::Widget(parent), 
	title(title_), 
	desc(desc_), 
	initial(initial_), 
	password(password_) 
{
	onInitDialog(&LineDlg::initDialog);
	onFocus(&LineDlg::focus);	
}

bool LineDlg::initDialog() {
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

void LineDlg::focus() {
	line->setFocus();
}

bool LineDlg::closing() {
	endDialog(IDCANCEL);
	return false;		
}

void LineDlg::okClicked(WidgetButtonPtr) {
	initial = line->getText();
	endDialog(IDOK);
}
void LineDlg::cancelClicked(WidgetButtonPtr) {
	endDialog(IDCANCEL);
}
