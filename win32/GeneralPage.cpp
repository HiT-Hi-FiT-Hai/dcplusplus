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

#include "resource.h"

#include "GeneralPage.h"

#include <dcpp/SettingsManager.h>
#include "WinUtil.h"

static const WinUtil::HelpItem helpItems[] = {
	{ IDC_SETTINGS_PERSONAL_INFORMATION, IDH_SETTINGS_GENERAL_PERSONAL_INFORMATION },
	{ IDC_SETTINGS_NICK, IDH_SETTINGS_GENERAL_NICK },
	{ IDC_NICK, IDH_SETTINGS_GENERAL_NICK },
	{ IDC_SETTINGS_EMAIL, IDH_SETTINGS_GENERAL_EMAIL },
	{ IDC_EMAIL, IDH_SETTINGS_GENERAL_EMAIL },
	{ IDC_SETTINGS_DESCRIPTION, IDH_SETTINGS_GENERAL_DESCRIPTION },
	{ IDC_DESCRIPTION, IDH_SETTINGS_GENERAL_DESCRIPTION },
	{ IDC_SETTINGS_UPLOAD_LINE_SPEED, IDH_SETTINGS_GENERAL_CONNECTION },
	{ IDC_CONNECTION, IDH_SETTINGS_GENERAL_CONNECTION },
	{ IDC_SETTINGS_MEBIBITS, IDH_SETTINGS_GENERAL_CONNECTION },
	{ 0, 0 }
};

PropPage::TextItem GeneralPage::texts[] = {
	{ IDC_SETTINGS_PERSONAL_INFORMATION, N_("Personal Information") },
	{ IDC_SETTINGS_NICK, N_("Nick") },
	{ IDC_SETTINGS_EMAIL, N_("E-Mail") },
	{ IDC_SETTINGS_DESCRIPTION, N_("Description") },
	{ IDC_SETTINGS_UPLOAD_LINE_SPEED, N_("Line speed (upload)") },
	{ IDC_SETTINGS_MEBIBITS, N_("MiBits/s") },
	{ 0, 0 }
};

PropPage::Item GeneralPage::items[] = {
	{ IDC_NICK,			SettingsManager::NICK,			PropPage::T_STR },
	{ IDC_EMAIL,		SettingsManager::EMAIL,			PropPage::T_STR },
	{ IDC_DESCRIPTION,	SettingsManager::DESCRIPTION,	PropPage::T_STR },
	{ IDC_CONNECTION,	SettingsManager::UPLOAD_SPEED,	PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

GeneralPage::GeneralPage(SmartWin::Widget* parent) : PropPage(parent), nick(0) {
	createDialog(IDD_GENERALPAGE);
	setHelpId(IDH_GENERALPAGE);

	WinUtil::setHelpIds(this, helpItems);
	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items);

	ComboBoxPtr connections = attachComboBox(IDC_CONNECTION);

	int selected = 0, j = 0;
	for(StringIter i = SettingsManager::connectionSpeeds.begin(); i != SettingsManager::connectionSpeeds.end(); ++i, ++j) {
		connections->addValue(Text::toT(*i).c_str());
		if(selected == 0 && SETTING(UPLOAD_SPEED) == *i) {
			selected = j; 
		}
	}

	connections->setSelected(selected);

	nick = attachTextBox(IDC_NICK);
	nick->setTextLimit(35);
	nick->onTextChanged(std::tr1::bind(&GeneralPage::handleNickTextChanged, this));

	attachTextBox(IDC_EMAIL);

	attachTextBox(IDC_DESCRIPTION)->setTextLimit(35);
}

GeneralPage::~GeneralPage() {
}

void GeneralPage::write() {
	PropPage::write(handle(), items);
}

void GeneralPage::handleNickTextChanged() {
	tstring text = nick->getText();
	bool update = false;

	// Strip ' '
	tstring::size_type i;
	while((i = text.find(' ')) != string::npos) {
		text.erase(i, 1);
		update = true;
	}

	if(update) {
		// Something changed; update window text without changing cursor pos
		long caretPos = nick->getCaretPos() - 1;
		nick->setText(text);
		nick->setSelection(caretPos, caretPos);
	}
}
