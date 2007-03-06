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

#ifndef DCPLUSPLUS_WIN32_NOTEPAD_FRAME_H
#define DCPLUSPLUS_WIN32_NOTEPAD_FRAME_H

#include "StaticFrame.h"

class NotepadFrame : public StaticFrame<NotepadFrame> {
public:
	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::NOTEPAD;

protected:
	friend class StaticFrame<NotepadFrame>;
	friend class MDIChildFrame<NotepadFrame>;
	
	NotepadFrame(SmartWin::Widget* mdiParent);
	virtual ~NotepadFrame();

	void layout();
	bool preClosing();

private:
	WidgetTextBoxPtr pad;
};

#ifdef PORT_ME
#include "FlatTabCtrl.h"
#include "WinUtil.h"

#define NOTEPAD_MESSAGE_MAP 13

class NotepadFrame : public MDITabChildWindowImpl<NotepadFrame>, public StaticFrame<NotepadFrame, ResourceManager::NOTEPAD>
{
public:
	DECLARE_FRAME_WND_CLASS_EX(_T("NotepadFrame"), IDR_NOTEPAD, 0, COLOR_3DFACE);

	NotepadFrame() : dirty(false),
		ctrlClientContainer(_T("edit"), this, NOTEPAD_MESSAGE_MAP) { }
	typedef MDITabChildWindowImpl<NotepadFrame> baseClass;
	BEGIN_MSG_MAP(NotepadFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(NOTEPAD_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, onLButton)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

private:

	bool dirty;
	CContainedWindow ctrlClientContainer;
};
#endif

#endif // !defined(NOTEPAD_FRAME_H)
