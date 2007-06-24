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

#include "SystemFrame.h"

#include "StupidWin.h"

SystemFrame::SystemFrame(SmartWin::Widget* mdiParent) : 
	SmartWin::Widget(mdiParent), 
	log(0) 
{
	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY;
		cs.exStyle = WS_EX_CLIENTEDGE;
		log = createTextBox(cs);
		add_widget(log);
	}

	initStatus();
	layout();
	
	deque<pair<time_t, string> > oldMessages = LogManager::getInstance()->getLastLogs();
	// Technically, we might miss a message or two here, but who cares...
	LogManager::getInstance()->addListener(this);

	for(deque<pair<time_t, string> >::iterator i = oldMessages.begin(); i != oldMessages.end(); ++i) {
		addLine(i->first, Text::toT(i->second));
	}
}

SystemFrame::~SystemFrame() {
	
}

void SystemFrame::addLine(time_t t, const tstring& msg) {
	int limit = log->getTextLimit();
	if(StupidWin::getWindowTextLength(log) + static_cast<int>(msg.size()) > limit) {
		StupidWin::setRedraw(log, false);
		log->setSelection(0, StupidWin::lineIndex(log, StupidWin::lineFromChar(log, limit / 10)));
		log->replaceSelection(_T(""));
		StupidWin::setRedraw(log, true);
	}
	log->addTextLines(Text::toT("\r\n[" + Util::getShortTimeString(t) + "] ") + msg);

#ifdef PORT_ME
	if(BOOLSETTING(BOLD_SYSTEM_LOG))
		setDirty();
#endif
}

void SystemFrame::layout() {
	const int border = 2;

	SmartWin::Rectangle r(this->getClientAreaSize());

	SmartWin::Rectangle rs = layoutStatus();

	r.size.y -= rs.size.y + border;

	log->setBounds(r);
}

void SystemFrame::spoken(WPARAM wp, LPARAM lp) {
	std::auto_ptr<std::pair<time_t, tstring> > msg(reinterpret_cast<std::pair<time_t, tstring>*>(wp));
	addLine(msg->first, msg->second);
}

bool SystemFrame::preClosing() {
	LogManager::getInstance()->removeListener(this);
	return true;	
}

void SystemFrame::on(Message, time_t t, const string& message) throw() { 
	speak(reinterpret_cast<WPARAM>(new pair<time_t, tstring>(t, Text::toT(message)))); 
}

#ifdef PORT_ME

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "SystemFrame.h"
#include "WinUtil.h"
#include "../client/File.h"

LRESULT SystemFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ctrlPad.SetFont(WinUtil::font);

	ctrlClientContainer.SubclassWindow(ctrlPad.m_hWnd);

	bHandled = FALSE;
	return 1;
}

LRESULT SystemFrame::onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
	HWND focus = GetFocus();
	bHandled = false;
	if(focus == ctrlPad.m_hWnd) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		tstring x;
		tstring::size_type start = (tstring::size_type)WinUtil::textUnderCursor(pt, ctrlPad, x);
		tstring::size_type end = x.find(_T(" "), start);

		if(end == tstring::npos)
			end = x.length();

		bHandled = WinUtil::parseDBLClick(x, start, end);
	}
	return 0;
}

#endif
