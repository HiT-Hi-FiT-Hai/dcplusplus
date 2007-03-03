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

#include "SplashWindow.h"

#include <client/version.h>
#include <client/Text.h>
#include <client/ResourceManager.h>

#include "WinUtil.h"

SplashWindow::SplashWindow() {
	int height = GetSystemMetrics(SM_CYFULLSCREEN);
	int width = GetSystemMetrics(SM_CXFULLSCREEN);
	Seed cs;
	cs.location = SmartWin::Rectangle((height/2-20), (width / 2 - 150), 40, 300);
	cs.style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	cs.exStyle = WS_EX_STATICEDGE;
	cs.caption = APPNAME;
	createWindow(cs);
	
	cs.style |= ES_CENTER | ES_READONLY | WS_VISIBLE;
	
	WidgetTextBox::Seed tcs;
	tcs.location = cs.location;
	tcs.style = cs.style | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	tcs.exStyle = WS_EX_STATICEDGE;
	tcs.caption = cs.caption;
	
	text = createTextBox(tcs);

	text->setFont(SmartWin::DefaultGuiFont);
	
	tcs.location.size.y = WinUtil::getTextHeight(text->handle(), text->getFont()->getHandle()) + 4;
	::HideCaret(text->handle());
	text->setBounds(tcs.location);
	text->bringToFront();
	text->updateWidget();
}

void SplashWindow::operator()(const string& str) {
	text->setText(Text::toT(STRING(LOADING) + "(" + str + ")"));
	text->updateWidget();
}
