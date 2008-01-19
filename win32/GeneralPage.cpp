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

#include "resource.h"

#include "GeneralPage.h"

#include <dcpp/SettingsManager.h>

PropPage::TextItem GeneralPage::texts[] = {
#ifdef PORT_ME
	{ IDC_SETTINGS_PERSONAL_INFORMATION, ResourceManager::SETTINGS_PERSONAL_INFORMATION },
	{ IDC_SETTINGS_NICK, ResourceManager::NICK },
	{ IDC_SETTINGS_EMAIL, ResourceManager::EMAIL },
	{ IDC_SETTINGS_DESCRIPTION, ResourceManager::DESCRIPTION },
	{ IDC_SETTINGS_UPLOAD_LINE_SPEED, ResourceManager::SETTINGS_UPLOAD_LINE_SPEED },
	{ IDC_SETTINGS_MEBIBITS, ResourceManager::MiBITSPS },
#endif
	{ 0, 0 }
};

PropPage::Item GeneralPage::items[] = {
	{ IDC_NICK,			SettingsManager::NICK,			PropPage::T_STR },
	{ IDC_EMAIL,		SettingsManager::EMAIL,			PropPage::T_STR },
	{ IDC_DESCRIPTION,	SettingsManager::DESCRIPTION,	PropPage::T_STR },
	{ IDC_CONNECTION,	SettingsManager::UPLOAD_SPEED,	PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

GeneralPage::GeneralPage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_GENERALPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items);

	WidgetComboBoxPtr connections = attachComboBox(IDC_CONNECTION);

	int selected = 0, j = 0;
	for(StringIter i = SettingsManager::connectionSpeeds.begin(); i != SettingsManager::connectionSpeeds.end(); ++i, ++j) {
		connections->addValue(Text::toT(*i).c_str());
		if(selected == 0 && SETTING(UPLOAD_SPEED) == *i) {
			selected = j; 
		}
	}

	connections->setSelectedIndex(selected);

	WidgetTextBoxPtr textBox;
#define TEXTBOX_ATTACH(id) \
	textBox = attachTextBox(id); \
	textBox->setTextLimit(35); \
	textBox->onTextChanged(std::tr1::bind(&GeneralPage::handleTextChanged, this, textBox))
	TEXTBOX_ATTACH(IDC_NICK);
	TEXTBOX_ATTACH(IDC_DESCRIPTION);
#undef TEXTBOX_ATTACH
}

GeneralPage::~GeneralPage() {
}

void GeneralPage::write() {
	PropPage::write(handle(), items);
}

void GeneralPage::handleTextChanged(WidgetTextBoxPtr textBox) {
	tstring text = textBox->getText();
	bool update = false;

	// Strip ' '
	tstring::size_type i;
	while((i = text.find(' ')) != string::npos) {
		text.erase(i, 1);
		update = true;
	}

	if(update) {
		// Something changed; update window text without changing cursor pos
		long caretPos = textBox->getCaretPos() - 1;
		textBox->setText(text);
		textBox->setSelection(caretPos, caretPos);
	}
}
