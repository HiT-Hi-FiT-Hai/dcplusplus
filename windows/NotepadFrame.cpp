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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "NotepadFrame.h"
#include "WinUtil.h"
#include "../client/File.h"

NotepadFrame* NotepadFrame::frame = NULL;

LRESULT NotepadFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ctrlPad.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL, WS_EX_CLIENTEDGE);
	
	ctrlPad.LimitText(0);
	ctrlPad.SetFont(WinUtil::font);
	string tmp;
	try {
		tmp = File(Util::getAppPath() + "Notepad.txt", File::READ, File::OPEN).read();
	} catch(const FileException&) {
		// ...
	}
	
	if(tmp.empty()) {
		tmp = SETTING(NOTEPAD_TEXT);
		if(!tmp.empty()) {
			dirty = true;
			SettingsManager::getInstance()->set(SettingsManager::NOTEPAD_TEXT, Util::emptyString);
		}
	}

	ctrlPad.SetWindowText(tmp.c_str());
	ctrlPad.EmptyUndoBuffer();
	
	SetWindowText(CSTRING(NOTEPAD));
	frame = this;
	
	m_hMenu = WinUtil::mainMenu;

	bHandled = FALSE;
	return 1;
}

LRESULT NotepadFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	
	if(dirty || ctrlPad.GetModify()) {
		AutoArray<char> buf(ctrlPad.GetWindowTextLength() + 1);
		ctrlPad.GetWindowText(buf, ctrlPad.GetWindowTextLength() + 1);
		try {
			File(Util::getAppPath() + "Notepad.txt", File::WRITE, File::CREATE | File::TRUNCATE).write(buf, ctrlPad.GetWindowTextLength());
		} catch(const FileException&) {
			// Oops...
		}
	}

	frame = NULL;
	MDIDestroy(m_hWnd);
	return 0;
	
}

void NotepadFrame::UpdateLayout(BOOL /*bResizeBars*/ /* = TRUE */)
{
	CRect rc;

	GetClientRect(rc);
	
	rc.bottom -= 1;
	rc.top += 1;
	rc.left +=1;
	rc.right -=1;
	ctrlPad.MoveWindow(rc);
	
}

/**
 * @file
 * $Id: NotepadFrame.cpp,v 1.10 2003/10/07 15:46:27 arnetheduck Exp $
 */


