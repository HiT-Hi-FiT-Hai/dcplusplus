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
#include <dcpp/DCPlusPlus.h>

#include "TextFrame.h"

#include <dcpp/File.h>
#include <dcpp/Text.h>

static const size_t MAX_TEXT_LEN = 64*1024;

TextFrame::TextFrame(SmartWin::WidgetMDIParent* mdiParent, const string& fileName) : 
	BaseType(mdiParent),
	pad(0) 
{
	WidgetTextBox::Seed cs;
	cs.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY;
	cs.exStyle = WS_EX_CLIENTEDGE;
	
	pad = createTextBox(cs);
	addWidget(pad);

	pad->setFont(WinUtil::monoFont);
	pad->setTextLimit(0);
	
	try {
		pad->setText(Text::toT(Text::toDOS(File(fileName, File::READ, File::OPEN).read(MAX_TEXT_LEN))));
	} catch(const FileException& e) {
		pad->setText(e.getError());
	}
	setText(Text::toT(Util::getFileName(fileName)));
	initStatus();
	layout();
}

void TextFrame::layout() {
	const int border = 2;

	SmartWin::Rectangle r(this->getClientAreaSize());

	SmartWin::Rectangle rs = layoutStatus();

	r.size.y -= rs.size.y + border;
	pad->setBounds(r);
}
