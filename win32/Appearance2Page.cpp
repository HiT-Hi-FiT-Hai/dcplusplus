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

#include "resource.h"

#include "Appearance2Page.h"

#include <dcpp/SettingsManager.h>
#include "WinUtil.h"

PropPage::TextItem Appearance2Page::texts[] = {
	{ IDC_BEEP_NOTIFICATION, ResourceManager::SETTINGS_NOTIFICATION_SOUND },
	{ IDC_BROWSE, ResourceManager::BROWSE_ACCEL },
	{ IDC_SETTINGS_COLORS, ResourceManager::SETTINGS_COLORS },
	{ IDC_SELWINCOLOR, ResourceManager::SETTINGS_SELECT_WINDOW_COLOR },
	{ IDC_SELTEXT, ResourceManager::SETTINGS_SELECT_TEXT_FACE },
	{ IDC_COLOREXAMPLE, ResourceManager::SETTINGS_EXAMPLE_TEXT },
	{ IDC_SETTINGS_REQUIRES_RESTART, ResourceManager::SETTINGS_REQUIRES_RESTART },
	{ IDC_SETTINGS_UPLOAD_BAR_COLOR, ResourceManager::UPLOADS },
	{ IDC_SETTINGS_SOUNDS, ResourceManager::SETTINGS_SOUNDS },
	{ IDC_PRIVATE_MESSAGE_BEEP, ResourceManager::SETTINGS_PM_BEEP },
	{ IDC_PRIVATE_MESSAGE_BEEP_OPEN, ResourceManager::SETTINGS_PM_BEEP_OPEN },
	{ IDC_SETTINGS_DOWNLOAD_BAR_COLOR, ResourceManager::DOWNLOADS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
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
	WinUtil::decodeFont(Text::toT(SETTING(TEXT_FONT)), font);
	fg = SETTING(TEXT_COLOR);
	bg = SETTING(BACKGROUND_COLOR);
	bgbrush = ::CreateSolidBrush(bg);
	fontObj = ::CreateFontIndirect(&font);
	upBar = SETTING(UPLOAD_BAR_COLOR);
	downBar = SETTING(DOWNLOAD_BAR_COLOR);

	example = subclassStatic(IDC_COLOREXAMPLE);
	example->onRaw(std::tr1::bind(&Appearance2Page::handleExampleColor, this, _1, _2), WM_CTLCOLORSTATIC);

	WidgetButtonPtr button = subclassButton(IDC_SELWINCOLOR);
	button->onClicked(std::tr1::bind(&Appearance2Page::handleBackgroundClicked, this));

	button = subclassButton(IDC_SELTEXT);
	button->onClicked(std::tr1::bind(&Appearance2Page::handleTextClicked, this));

	button = subclassButton(IDC_SETTINGS_UPLOAD_BAR_COLOR);
	button->onClicked(std::tr1::bind(&Appearance2Page::handleULClicked, this));

	button = subclassButton(IDC_SETTINGS_DOWNLOAD_BAR_COLOR);
	button->onClicked(std::tr1::bind(&Appearance2Page::handleDLClicked, this));

	button = subclassButton(IDC_BROWSE);
	button->onClicked(std::tr1::bind(&Appearance2Page::handleBrowseClicked, this));
}

Appearance2Page::~Appearance2Page()
{
	::DeleteObject(bgbrush);
	::DeleteObject(fontObj);
}

void Appearance2Page::write()
{
	PropPage::write(handle(), items, 0,0);

	SettingsManager* settings = SettingsManager::getInstance();

	settings->set(SettingsManager::TEXT_COLOR, (int)fg);
	settings->set(SettingsManager::BACKGROUND_COLOR, (int)bg);
	settings->set(SettingsManager::UPLOAD_BAR_COLOR, (int)upBar);
	settings->set(SettingsManager::DOWNLOAD_BAR_COLOR, (int)downBar);

	tstring f = WinUtil::encodeFont(font);
	settings->set(SettingsManager::TEXT_FONT, Text::fromT(f));
}

HRESULT Appearance2Page::handleExampleColor(WPARAM wParam, LPARAM lParam) {
	HDC hDC((HDC)wParam);
	::SetBkMode(hDC, TRANSPARENT);
	::SetTextColor(hDC, fg);
	::SelectObject(hDC, fontObj);
	return (LRESULT)bgbrush;
}

void Appearance2Page::handleBackgroundClicked() {
	WidgetChooseColor::ColorParams initialColorParams(bg),
		colorParams = createChooseColor().showDialog(initialColorParams);
	if(colorParams.userPressedOk()) {
		::DeleteObject(bgbrush);
		bg = colorParams.getColor();
		bgbrush = CreateSolidBrush(bg);
		example->invalidateWidget();
	}
}

void Appearance2Page::handleTextClicked() {
	LOGFONT font_ = font;
	DWORD fg_ = fg;
	if(createChooseFont().showDialog(CF_EFFECTS | CF_SCREENFONTS, &font_, fg_)) {
		font = font_;
		fg = fg_;
		::DeleteObject(fontObj);
		fontObj = ::CreateFontIndirect(&font);
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

#ifdef PORT_ME

LRESULT Appearance2Page::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_APPEARANCE2PAGE);
	return 0;
}

LRESULT Appearance2Page::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_APPEARANCE2PAGE);
	return 0;
}
#endif
