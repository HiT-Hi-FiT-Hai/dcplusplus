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

#include "NotepadFrame.h"

#include <client/File.h>
#include <client/Text.h>

NotepadFrame::NotepadFrame(SmartWin::Widget* mdiParent) : 
	SmartWin::Widget(mdiParent), 
	pad(0) 
{
	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL;
		cs.exStyle = WS_EX_CLIENTEDGE;
		pad = createTextBox(cs);
		add_widget(pad);
	}
	
	initStatus();
	
	pad->setTextLimit(0);
	try {
		pad->setText(Text::toT(File(Util::getNotepadFile(), File::READ, File::OPEN).read()));
	} catch(const FileException& e) {
		// Ignore		
	}
	
	StupidWin::setModify(pad, false);

	layout();
}

NotepadFrame::~NotepadFrame() {

}

bool NotepadFrame::preClosing() {
	if(StupidWin::getModify(pad)) {
		try {
			dcdebug("Writing notepad contents\n");
			File(Util::getNotepadFile(), File::WRITE, File::CREATE | File::TRUNCATE).write(Text::fromT(pad->getText()));
		} catch(const FileException& e) {
			dcdebug("Writing failed: %s\n", e.getError().c_str());
			///@todo Notify user			
		}
	}
	return true;
}

void NotepadFrame::layout() {
	const int border = 2;
	
	SmartWin::Rectangle r(SmartWin::Point(0, 0), getClientAreaSize());

	SmartWin::Rectangle rs = layoutStatus();
	r.size.y -= rs.size.y + border;

	pad->setBounds(r);
}

#ifdef PORT_ME

LRESULT NotepadFrame::onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
	HWND focus = GetFocus();
	bHandled = false;
	if(focus == ctrlPad.m_hWnd) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		tstring x;
		tstring::size_type start = (tstring::size_type)WinUtil::textUnderCursor(pt, ctrlPad, x);
		tstring::size_type end = x.find(_T(" "), start);

		if(end == string::npos)
			end = x.length();

		bHandled = WinUtil::parseDBLClick(x, start, end);
	}
	return 0;
}
#endif
