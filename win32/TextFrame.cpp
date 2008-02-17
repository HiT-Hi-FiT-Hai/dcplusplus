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

#include "TextFrame.h"

#include <dcpp/File.h>
#include <dcpp/Text.h>

static const size_t MAX_TEXT_LEN = 64*1024;

TextFrame::TextFrame(SmartWin::WidgetTabView* mdiParent, const string& fileName) : 
	BaseType(mdiParent, Text::toT(Util::getFileName(fileName))),
	pad(0) 
{
	WidgetTextBox::Seed cs = WinUtil::Seeds::textBox;
	cs.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY;
	cs.font = WinUtil::monoFont;
	pad = createTextBox(cs);
	addWidget(pad);

	pad->setTextLimit(0);
	
	try {
		pad->setText(Text::toT(Text::toDOS(File(fileName, File::READ, File::OPEN).read(MAX_TEXT_LEN))));
	} catch(const FileException& e) {
		pad->setText(Text::toT(e.getError()));
	}
	initStatus();
	layout();
}

void TextFrame::layout() {
	SmartWin::Rectangle r(this->getClientAreaSize());

	layoutStatus(r);

	pad->setBounds(r);
}
