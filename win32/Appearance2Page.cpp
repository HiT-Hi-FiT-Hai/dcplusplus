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

#include "Appearance2Page.h"

#include <dcpp/SettingsManager.h>
#include "WinUtil.h"

PropPage::TextItem Appearance2Page::texts[] = {
	{ IDC_BEEP_NOTIFICATION, N_("Notification sound") },
	{ IDC_BROWSE, N_("&Browse...") },
	{ IDC_SETTINGS_COLORS, N_("Colors") },
	{ IDC_SELWINCOLOR, N_("Select &window color") },
	{ IDC_SELTEXT, N_("Select &text style") },
	{ IDC_COLOREXAMPLE, N_("Donate \342\202\254\342\202\254\342\202\254:s! (ok, dirty dollars are fine as well =) (see help menu)") },
	{ IDC_SETTINGS_REQUIRES_RESTART, N_("Note; most of these options require that you restart DC++") },
	{ IDC_SETTINGS_UPLOAD_BAR_COLOR, N_("Uploads") },
	{ IDC_SETTINGS_SOUNDS, N_("Sounds") },
	{ IDC_PRIVATE_MESSAGE_BEEP, N_("Make an annoying sound every time a private message is received") },
	{ IDC_PRIVATE_MESSAGE_BEEP_OPEN, N_("Make an annoying sound when a private message window is opened") },
	{ IDC_SETTINGS_DOWNLOAD_BAR_COLOR, N_("Downloads") },
	{ 0, 0 }
};

PropPage::Item Appearance2Page::items[] = {
	{ IDC_PRIVATE_MESSAGE_BEEP, SettingsManager::PRIVATE_MESSAGE_BEEP, PropPage::T_BOOL },
	{ IDC_PRIVATE_MESSAGE_BEEP_OPEN, SettingsManager::PRIVATE_MESSAGE_BEEP_OPEN, PropPage::T_BOOL },
	{ IDC_BEEPFILE, SettingsManager::BEEPFILE, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

Appearance2Page::Appearance2Page(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_APPEARANCE2PAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items, 0, 0);

	fg = SETTING(TEXT_COLOR);
	bg = SETTING(BACKGROUND_COLOR);
	upBar = SETTING(UPLOAD_BAR_COLOR);
	downBar = SETTING(DOWNLOAD_BAR_COLOR);

	bgBrush = SmartWin::BrushPtr(new SmartWin::Brush(bg));

	WinUtil::decodeFont(Text::toT(SETTING(TEXT_FONT)), logFont);
	font = SmartWin::FontPtr(new SmartWin::Font(::CreateFontIndirect(&logFont), true));

	example = attachStatic(IDC_COLOREXAMPLE);
	example->onBackgroundColor(std::tr1::bind(&Appearance2Page::handleExampleColor, this, _1));

	WidgetButtonPtr button = attachButton(IDC_SELWINCOLOR);
	button->onClicked(std::tr1::bind(&Appearance2Page::handleBackgroundClicked, this));

	button = attachButton(IDC_SELTEXT);
	button->onClicked(std::tr1::bind(&Appearance2Page::handleTextClicked, this));

	button = attachButton(IDC_SETTINGS_UPLOAD_BAR_COLOR);
	button->onClicked(std::tr1::bind(&Appearance2Page::handleULClicked, this));

	button = attachButton(IDC_SETTINGS_DOWNLOAD_BAR_COLOR);
	button->onClicked(std::tr1::bind(&Appearance2Page::handleDLClicked, this));

	button = attachButton(IDC_BROWSE);
	button->onClicked(std::tr1::bind(&Appearance2Page::handleBrowseClicked, this));
}

Appearance2Page::~Appearance2Page() {
}

void Appearance2Page::write() {
	PropPage::write(handle(), items, 0,0);

	SettingsManager* settings = SettingsManager::getInstance();

	settings->set(SettingsManager::TEXT_COLOR, (int)fg);
	settings->set(SettingsManager::BACKGROUND_COLOR, (int)bg);
	settings->set(SettingsManager::UPLOAD_BAR_COLOR, (int)upBar);
	settings->set(SettingsManager::DOWNLOAD_BAR_COLOR, (int)downBar);
	settings->set(SettingsManager::TEXT_FONT, Text::fromT(WinUtil::encodeFont(logFont)));
}

SmartWin::BrushPtr Appearance2Page::handleExampleColor(SmartWin::Canvas& canvas) {
	canvas.setBkColor(bg);
	canvas.setTextColor(fg);
	canvas.selectFont(font);
	return bgBrush;
}

void Appearance2Page::handleBackgroundClicked() {
	WidgetChooseColor::ColorParams initialColorParams(bg),
		colorParams = createChooseColor().showDialog(initialColorParams);
	if(colorParams.userPressedOk()) {
		bg = colorParams.getColor();
		bgBrush = SmartWin::BrushPtr(new SmartWin::Brush(bg));
		example->invalidateWidget();
	}
}

void Appearance2Page::handleTextClicked() {
	LOGFONT logFont_ = logFont;
	DWORD fg_ = fg;
	if(createChooseFont().showDialog(CF_EFFECTS | CF_SCREENFONTS, &logFont_, fg_)) {
		logFont = logFont_;
		fg = fg_;
		font = SmartWin::FontPtr(new SmartWin::Font(::CreateFontIndirect(&logFont), true));
		example->invalidateWidget();
	}
}

void Appearance2Page::handleULClicked() {
	WidgetChooseColor::ColorParams initialColorParams(upBar),
		colorParams = createChooseColor().showDialog(initialColorParams);
	if(colorParams.userPressedOk())
		upBar = colorParams.getColor();
}

void Appearance2Page::handleDLClicked() {
	WidgetChooseColor::ColorParams initialColorParams(downBar),
		colorParams = createChooseColor().showDialog(initialColorParams);
	if(colorParams.userPressedOk())
		downBar = colorParams.getColor();
}

void Appearance2Page::handleBrowseClicked() {
	TCHAR buf[MAX_PATH];

	::GetDlgItemText(handle(), IDC_BEEPFILE, buf, MAX_PATH);
	tstring x = buf;

	if(WinUtil::browseFile(x, handle(), false) == IDOK) {
		::SetDlgItemText(handle(), IDC_BEEPFILE, x.c_str());
	}
}
