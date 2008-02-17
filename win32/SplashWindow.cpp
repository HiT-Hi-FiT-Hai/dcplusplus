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
#include <dcpp/DCPlusPlus.h>

#include "SplashWindow.h"

#include <dcpp/version.h>
#include <dcpp/Text.h>

#include "WinUtil.h"

SplashWindow::SplashWindow() : SmartWin::WidgetFactory<SmartWin::WidgetWindow>(0) {
	{
		Seed cs;
		cs.style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		cs.exStyle = WS_EX_STATICEDGE;
		cs.caption = _T(APPNAME);
		tmp = new SmartWin::WidgetFactory<SmartWin::WidgetWindow>(0);
		tmp->createWindow(cs);
	}
	{
		Seed cs;
		cs.style = WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		cs.exStyle = WS_EX_STATICEDGE;
		cs.caption = _T(APPNAME);
		createWindow(cs);
	}
	tstring caption = _T(APPNAME) _T(" ") _T(VERSIONSTRING);
	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | ES_CENTER | ES_READONLY;
		cs.exStyle = WS_EX_STATICEDGE;
		text = createTextBox(cs);
	}

	text->setFont(SmartWin::DefaultGuiFont);
	
	SmartWin::Point textSize(text->getTextSize(caption));
	SmartWin::Point desktopSize(getDesktopSize());
	int xmid = desktopSize.x / 2;
	int ymid = desktopSize.y / 2;
	int xtext = 300;
	int ytext = textSize.y + 6;
	
	SmartWin::Rectangle r(xmid - xtext/2, ymid - ytext/2, xtext, ytext);
	setBounds(r);
	text->setBounds(0, 0, xtext, ytext);

	::HideCaret(text->handle());
	text->setVisible(true);
	text->bringToFront();
	text->updateWidget();
}

SplashWindow::~SplashWindow() {
	tmp->close();
}

void SplashWindow::operator()(const string& status) {
	text->setText(str(TF_("Loading DC++, please wait... (%1%)") % Text::toT(status) ));
	text->updateWidget();
}
