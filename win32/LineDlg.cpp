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
 
#include "stdafx.h"

#include "LineDlg.h"

LineDlg::LineDlg(SmartWin::Widget* parent, const tstring& title_, const tstring& desc_, const tstring& initial_, bool password_) : 
	WidgetFactory<SmartWin::WidgetModalDialog>(parent), 
	title(title_), 
	desc(desc_), 
	initial(initial_), 
	password(password_) 
{
	onInitDialog(std::tr1::bind(&LineDlg::initDialog, this));
	onFocus(std::tr1::bind(&LineDlg::focus, this));	
}

bool LineDlg::initDialog() {
	attachButton(IDOK)->onClicked(std::tr1::bind(&LineDlg::okClicked, this));
	attachButton(IDCANCEL)->onClicked(std::tr1::bind(&LineDlg::cancelClicked, this));
	attachStatic(IDC_DESCRIPTION)->setText(desc);

	line = attachTextBox(IDC_LINE);
	line->setFocus();
	line->setText(initial);
	line->setSelection();
	if(password) {
		line->setPassword();
	}
	
	setText(title);

	centerWindow();
	
	return false;
}

void LineDlg::focus() {
	line->setFocus();
}

bool LineDlg::closing() {
	endDialog(IDCANCEL);
	return false;		
}

void LineDlg::okClicked() {
	initial = line->getText();
	endDialog(IDOK);
}
void LineDlg::cancelClicked() {
	endDialog(IDCANCEL);
}
