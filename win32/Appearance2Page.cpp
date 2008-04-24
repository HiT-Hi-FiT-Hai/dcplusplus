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

#include "Appearance2Page.h"

#include <dcpp/SettingsManager.h>
#include "WinUtil.h"

static const WinUtil::HelpItem helpItems[] = {
	{ IDC_SETTINGS_COLORS, IDH_SETTINGS_APPEARANCE2_COLORS },
	{ IDC_SELWINCOLOR, IDH_SETTINGS_APPEARANCE2_SELWINCOLOR },
	{ IDC_SELTEXT, IDH_SETTINGS_APPEARANCE2_SELTEXT },
	{ IDC_COLOREXAMPLE, IDH_SETTINGS_APPEARANCE2_COLORS },
	{ IDC_SETTINGS_UPLOAD_BAR_COLOR, IDH_SETTINGS_APPEARANCE2_UPLOAD_BAR_COLOR },
	{ IDC_SETTINGS_DOWNLOAD_BAR_COLOR, IDH_SETTINGS_APPEARANCE2_DOWNLOAD_BAR_COLOR },
	{ IDC_BEEP_NOTIFICATION, IDH_SETTINGS_APPEARANCE2_BEEPFILE },
	{ IDC_BEEPFILE, IDH_SETTINGS_APPEARANCE2_BEEPFILE },
	{ IDC_BROWSE, IDH_SETTINGS_APPEARANCE2_BEEPFILE },
	{ IDC_SETTINGS_REQUIRES_RESTART, IDH_SETTINGS_APPEARANCE_REQUIRES_RESTART },
	{ 0, 0 }
};

PropPage::TextItem Appearance2Page::texts[] = {
	{ IDC_SETTINGS_COLORS, N_("Colors") },
	{ IDC_SELWINCOLOR, N_("Select &window color") },
	{ IDC_SELTEXT, N_("Select &text style") },
	{ IDC_COLOREXAMPLE, N_("Donate \342\202\254\342\202\254\342\202\254:s! (ok, dirty dollars are fine as well =) (see help menu)") },
	{ IDC_SETTINGS_REQUIRES_RESTART, N_("Note; most of these options require that you restart DC++") },
	{ IDC_SETTINGS_UPLOAD_BAR_COLOR, N_("Uploads") },
	{ IDC_SETTINGS_SOUNDS, N_("Sounds") },
	{ IDC_SETTINGS_DOWNLOAD_BAR_COLOR, N_("Downloads") },
	{ IDC_BEEP_NOTIFICATION, N_("Notification sound") },
	{ IDC_BROWSE, N_("&Browse...") },
	{ 0, 0 }
};

Appearance2Page::SoundOption Appearance2Page::soundOptions[] = {
	{ N_("Every time a main chat message is received"), SettingsManager::SOUND_MAIN_CHAT, Util::emptyStringT, IDH_SETTINGS_APPEARANCE2_SOUND_MAIN_CHAT },
	{ N_("Every time a private message is received"), SettingsManager::SOUND_PM, Util::emptyStringT, IDH_SETTINGS_APPEARANCE2_SOUND_PM },
	{ N_("When a private message window is opened"), SettingsManager::SOUND_PM_WINDOW, Util::emptyStringT, IDH_SETTINGS_APPEARANCE2_SOUND_PM_WINDOW },
	{ 0, 0, Util::emptyStringT }
};

Appearance2Page::Appearance2Page(dwt::Widget* parent) : PropPage(parent), oldSelection(-1) {
	createDialog(IDD_APPEARANCE2PAGE);
	setHelpId(IDH_APPEARANCE2PAGE);

	WinUtil::setHelpIds(this, helpItems);
	PropPage::translate(handle(), texts);

	fg = SETTING(TEXT_COLOR);
	bg = SETTING(BACKGROUND_COLOR);
	upBar = SETTING(UPLOAD_BAR_COLOR);
	downBar = SETTING(DOWNLOAD_BAR_COLOR);

	WinUtil::decodeFont(Text::toT(SETTING(TEXT_FONT)), logFont);
	font = dwt::FontPtr(new dwt::Font(::CreateFontIndirect(&logFont), true));

	attachChild(example, IDC_COLOREXAMPLE);
	example->setColor(fg, bg);
	example->setFont(font);

	attachChild(sounds, IDC_SOUNDS);
	PropPage::initList(sounds);

	{
		ButtonPtr button = attachChild<Button>(IDC_SELWINCOLOR);
		button->onClicked(std::tr1::bind(&Appearance2Page::handleBackgroundClicked, this));

		button = attachChild<Button>(IDC_SELTEXT);
		button->onClicked(std::tr1::bind(&Appearance2Page::handleTextClicked, this));

		button = attachChild<Button>(IDC_SETTINGS_UPLOAD_BAR_COLOR);
		button->onClicked(std::tr1::bind(&Appearance2Page::handleULClicked, this));

		button = attachChild<Button>(IDC_SETTINGS_DOWNLOAD_BAR_COLOR);
		button->onClicked(std::tr1::bind(&Appearance2Page::handleDLClicked, this));
	}

	attachChild(beepFileLabel, IDC_BEEP_NOTIFICATION);

	attachChild(beepFile, IDC_BEEPFILE);

	attachChild(browse, IDC_BROWSE);
	browse->onClicked(std::tr1::bind(&Appearance2Page::handleBrowseClicked, this));

	setBeepEnabled(false);

	for(size_t i = 0; soundOptions[i].setting != 0; ++i) {
		soundOptions[i].file = Text::toT(SettingsManager::getInstance()->get((SettingsManager::StrSetting)soundOptions[i].setting));

		TStringList row;
		row.push_back(T_(soundOptions[i].text));
		sounds->setChecked(sounds->insert(row), !soundOptions[i].file.empty());
	}

	sounds->setColumnWidth(0, LVSCW_AUTOSIZE);

	saveSoundOptions();

	sounds->onHelp(std::tr1::bind(&Appearance2Page::handleSoundsHelp, this, _1, _2));
	sounds->onSelectionChanged(std::tr1::bind(&Appearance2Page::handleSelectionChanged, this));
}

Appearance2Page::~Appearance2Page() {
}

void Appearance2Page::write() {
	SettingsManager* settings = SettingsManager::getInstance();

	settings->set(SettingsManager::TEXT_COLOR, (int)fg);
	settings->set(SettingsManager::BACKGROUND_COLOR, (int)bg);
	settings->set(SettingsManager::UPLOAD_BAR_COLOR, (int)upBar);
	settings->set(SettingsManager::DOWNLOAD_BAR_COLOR, (int)downBar);
	settings->set(SettingsManager::TEXT_FONT, Text::fromT(WinUtil::encodeFont(logFont)));

	saveSoundOptions();
	for(size_t i = 0; soundOptions[i].setting != 0; ++i)
		settings->set((SettingsManager::StrSetting)soundOptions[i].setting, Text::fromT(soundOptions[i].file));
}

void Appearance2Page::handleBackgroundClicked() {
	ColorDialog::ColorParams colorParams(bg);
	if(createColorDialog().open(colorParams)) {
		bg = colorParams.getColor();
		example->setColor(fg, bg);
		example->redraw();
	}
}

void Appearance2Page::handleTextClicked() {
	LOGFONT logFont_ = logFont;
	DWORD fg_ = fg;
	if(createFontDialog().open(CF_EFFECTS | CF_SCREENFONTS, logFont_, fg_)) {
		logFont = logFont_;
		fg = fg_;
		font = dwt::FontPtr(new dwt::Font(::CreateFontIndirect(&logFont), true));
		example->setColor(fg, bg);
		example->setFont(font);
		example->redraw();
	}
}

void Appearance2Page::handleULClicked() {
	ColorDialog::ColorParams colorParams(upBar);
	if(createColorDialog().open(colorParams)) {
		upBar = colorParams.getColor();
	}
}

void Appearance2Page::handleDLClicked() {
	ColorDialog::ColorParams colorParams(downBar);
	if(createColorDialog().open(colorParams)) {
		downBar = colorParams.getColor();
	}
}

void Appearance2Page::handleSoundsHelp(HWND hWnd, unsigned id) {
	// same as PropPage::handleListHelp
	int item = sounds->hitTest(dwt::ScreenCoordinate(dwt::Point::fromLParam(::GetMessagePos())));
	if(item >= 0 && soundOptions[item].helpId)
		id = soundOptions[item].helpId;
	WinUtil::help(hWnd, id);
}

void Appearance2Page::handleSelectionChanged() {
	saveSoundOptions();

	int sel = sounds->getSelected();
	if(sel >= 0) {
		bool checked = sounds->isChecked(sel);
		setBeepEnabled(checked);
		beepFile->setText(
			(checked && soundOptions[sel].file != _T("beep"))
			? soundOptions[sel].file
			: Util::emptyStringT
		);
	} else {
		setBeepEnabled(false);
		beepFile->setText(Util::emptyStringT);
	}
	oldSelection = sel;
}

void Appearance2Page::handleBrowseClicked() {
	tstring x = beepFile->getText();
	if(createLoadDialog().open(x)) {
		beepFile->setText(x);
	}
}

void Appearance2Page::setBeepEnabled(bool enabled) {
	beepFileLabel->setEnabled(enabled);
	beepFile->setEnabled(enabled);
	browse->setEnabled(enabled);
}

void Appearance2Page::saveSoundOptions() {
	for(size_t i = 0; i < sounds->size(); ++i) {
		if(sounds->isChecked(i)) {
			if(soundOptions[i].file.empty())
				soundOptions[i].file = _T("beep");
		} else
			soundOptions[i].file = Util::emptyStringT;
	}

	if(oldSelection >= 0) {
		if(sounds->isChecked(oldSelection)) {
			tstring text = beepFile->getText();
			soundOptions[oldSelection].file = text.empty() ? _T("beep") : text;
		} else
			soundOptions[oldSelection].file = Util::emptyStringT;
	}
}
