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

#include "SettingsDialog.h"
#include "resource.h"

#include "WinUtil.h"

#include "GeneralPage.h"
#include "NetworkPage.h"
#include "DownloadPage.h"
#include "FavoriteDirsPage.h"
#include "QueuePage.h"
#include "UploadPage.h"
#include "AppearancePage.h"
#include "Appearance2Page.h"
#include "TabsPage.h"
#include "WindowsPage.h"
#include "AdvancedPage.h"
#include "LogPage.h"
#include "Advanced3Page.h"
#include "UCPage.h"
#include "CertificatesPage.h"

static const WinUtil::HelpItem helpItems[] = {
	{ IDC_SETTINGS_PAGES, IDH_SETTINGS_TREE },
	{ IDOK, IDH_DCPP_OK },
	{ IDCANCEL, IDH_DCPP_CANCEL },
	{ IDHELP, IDH_DCPP_HELP },
	{ 0, 0 }
};

static const TCHAR SEPARATOR = _T('\\');
static const size_t MAX_NAME_LENGTH = 256;

SettingsDialog::SettingsDialog(dwt::Widget* parent) : WidgetFactory<dwt::ModalDialog>(parent), currentPage(0) {
	onInitDialog(std::tr1::bind(&SettingsDialog::initDialog, this));
	onHelp(std::tr1::bind(&SettingsDialog::handleHelp, this, _1, _2));
}

int SettingsDialog::run() {
	return createDialog(IDD_SETTINGS);
}

SettingsDialog::~SettingsDialog() {
}

bool SettingsDialog::initDialog() {
	// set this to IDH_STARTPAGE so that clicking in an empty space of the dialog generates a WM_HELP message with no error; then SettingsDialog::handleHelp will convert IDH_STARTPAGE to the current page's help id
	setHelpId(IDH_STARTPAGE);

	WinUtil::setHelpIds(this, helpItems);

	setText(T_("Settings"));

	attachChild(pageTree, IDC_SETTINGS_PAGES);
	pageTree->onSelectionChanged(std::tr1::bind(&SettingsDialog::handleSelectionChanged, this));

	{
		ButtonPtr button = attachChild<Button>(IDOK);
		button->setText(T_("OK"));
		button->onClicked(std::tr1::bind(&SettingsDialog::handleOKClicked, this));

		button = attachChild<Button>(IDCANCEL);
		button->setText(T_("Cancel"));
		button->onClicked(std::tr1::bind(&SettingsDialog::endDialog, this, IDCANCEL));

		button = attachChild<Button>(IDHELP);
		button->setText(T_("Help"));
		button->onClicked(std::tr1::bind(&SettingsDialog::handleHelp, this, handle(), IDH_STARTPAGE));
	}

	addPage(T_("Personal information"), new GeneralPage(this));

	addPage(T_("Connection settings"), new NetworkPage(this));

	{
		HTREEITEM item = addPage(T_("Downloads"), new DownloadPage(this));
		addPage(T_("Favorites"), new FavoriteDirsPage(this), item);
		addPage(T_("Queue"), new QueuePage(this), item);
	}

	addPage(T_("Sharing"), new UploadPage(this));

	{
		HTREEITEM item = addPage(T_("Appearance"), new AppearancePage(this));
		addPage(T_("Colors and sounds"), new Appearance2Page(this), item);
		addPage(T_("Tabs"), new TabsPage(this), item);
		addPage(T_("Windows"), new WindowsPage(this), item);
	}

	{
		HTREEITEM item = addPage(T_("Advanced"), new AdvancedPage(this));
		addPage(T_("Logs"), new LogPage(this), item);
		addPage(T_("Experts only"), new Advanced3Page(this), item);
		addPage(T_("User Commands"), new UCPage(this), item);
		addPage(T_("Security Certificates"), new CertificatesPage(this), item);
	}

	updateTitle();

	return false;
}

HTREEITEM SettingsDialog::addPage(const tstring& title, PropPage* page, HTREEITEM parent) {
	pages.push_back(page);

	HTREEITEM item = pageTree->insert(title, parent, reinterpret_cast<LPARAM>(page));
	pageTree->expand(parent);
	return item;
}

void SettingsDialog::handleHelp(HWND hWnd, unsigned id) {
	if(id == IDH_STARTPAGE && currentPage)
		id = currentPage->getHelpId();
	WinUtil::help(hWnd, id);
}

void SettingsDialog::handleSelectionChanged() {
	PropPage* page = reinterpret_cast<PropPage*>(pageTree->getData(pageTree->getSelected()));
	if(page) {
		if(currentPage) {
			currentPage->setVisible(false);
			currentPage = 0;
		}

		page->setVisible(true);
		currentPage = page;

		updateTitle();
	}
}

void SettingsDialog::handleOKClicked() {
	write();
	endDialog(IDOK);
}

void SettingsDialog::updateTitle() {
	tstring title = pageTree->getSelectedText();
	setText(title.empty() ? T_("Settings") : T_("Settings") + _T(" - [") + title + _T("]"));
}

void SettingsDialog::write() {
	for(PageList::iterator i = pages.begin(); i != pages.end(); ++i) {
		(*i)->write();
	}
}

