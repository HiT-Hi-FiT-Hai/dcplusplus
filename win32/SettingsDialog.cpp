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

#include "SettingsDialog.h"
#include "resource.h"

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

static const TCHAR SEPARATOR = _T('\\');
static const size_t MAX_NAME_LENGTH = 256;

SettingsDialog::SettingsDialog(SmartWin::Widget* parent) : WidgetFactory<SmartWin::WidgetModalDialog>(parent), currentPage(0) {
	onInitDialog(std::tr1::bind(&SettingsDialog::initDialog, this));
}

bool SettingsDialog::initDialog() {
	subclassButton(IDOK)->onClicked(std::tr1::bind(&SettingsDialog::handleOKClicked, this));

	subclassButton(IDCANCEL)->onClicked(std::tr1::bind(&SettingsDialog::endDialog, this, IDCANCEL));

	pageTree = subclassTreeView(IDC_SETTINGS_PAGES);
	pageTree->onSelectionChanged(std::tr1::bind(&SettingsDialog::selectionChanged, this));
	
	setText(TSTRING(SETTINGS));
	
	addPage(TSTRING(SETTINGS_GENERAL), new GeneralPage(this));
	addPage(TSTRING(SETTINGS_NETWORK), new NetworkPage(this));
	addPage(TSTRING(SETTINGS_DOWNLOADS), new DownloadPage(this));
	addPage(TSTRING(SETTINGS_FAVORITE_DIRS_PAGE), new FavoriteDirsPage(this));
	addPage(TSTRING(SETTINGS_QUEUE), new QueuePage(this));
	addPage(TSTRING(SETTINGS_UPLOADS), new UploadPage(this));
	addPage(TSTRING(SETTINGS_APPEARANCE), new AppearancePage(this));
	addPage(TSTRING(SETTINGS_APPEARANCE2), new Appearance2Page(this));
	addPage(TSTRING(SETTINGS_TABS), new TabsPage(this));
	addPage(TSTRING(SETTINGS_WINDOWS), new WindowsPage(this));
	addPage(TSTRING(SETTINGS_ADVANCED), new AdvancedPage(this));
	addPage(TSTRING(SETTINGS_LOGS), new LogPage(this));
	addPage(TSTRING(SETTINGS_ADVANCED3), new Advanced3Page(this));
	addPage(TSTRING(SETTINGS_USER_COMMANDS), new UCPage(this));
	addPage(TSTRING(SETTINGS_CERTIFICATES), new CertificatesPage(this));
	
	return false;
}

SettingsDialog::~SettingsDialog() {
}

int SettingsDialog::run() {
	return createDialog(IDD_SETTINGS);
}

void SettingsDialog::addPage(const tstring& title, PropPage* page) {
	pages.push_back(page);
	createTree(title, TVI_ROOT, page);
}

void SettingsDialog::handleOKClicked() {
	write();
	endDialog(IDOK);
}

void SettingsDialog::selectionChanged() {
	HTREEITEM item = TreeView_GetSelection(pageTree->handle());
	if(item == NULL) {
		showPage(0);
	} else {
		TVITEM tvitem = { TVIF_PARAM | TVIF_HANDLE };
		tvitem.hItem = item;
		if(!TreeView_GetItem(pageTree->handle(), &tvitem)) {
			showPage(0);
		} else {
			showPage(reinterpret_cast<PropPage*>(tvitem.lParam));
		}
	}
}

void SettingsDialog::showPage(PropPage* page) {
	if(currentPage) {
		::ShowWindow(dynamic_cast<SmartWin::Widget*>(currentPage)->handle(), SW_HIDE);
		currentPage = 0;
	}
	if(page) {
		::ShowWindow(dynamic_cast<SmartWin::Widget*>(page)->handle(), SW_SHOW);
		currentPage = page;
	}
}

HTREEITEM SettingsDialog::createTree(const tstring& str, HTREEITEM parent, PropPage* page) {
	TVINSERTSTRUCT tvi;
	tvi.hInsertAfter = TVI_LAST;
	tvi.hParent = parent;

	HTREEITEM first = (parent == TVI_ROOT) ? TreeView_GetRoot(pageTree->handle()) : TreeView_GetChild(pageTree->handle(), parent);

	string::size_type i = str.find(SEPARATOR);
	if(i == string::npos) {
		// Last dir, the actual page
		HTREEITEM item = findItem(str, first);
		if(item == NULL) {
			// Doesn't exist, add
			tvi.item.mask = TVIF_PARAM | TVIF_TEXT;
			tvi.item.pszText = const_cast<LPTSTR>(str.c_str());
			tvi.item.lParam = reinterpret_cast<LPARAM>(page);
			item = TreeView_InsertItem(pageTree->handle(), &tvi);
			TreeView_Expand(pageTree->handle(), parent, TVE_EXPAND);
			return item;
		} else {
#ifdef PORT_ME
			// Update page
			if(ctrlTree.GetItemData(item) == -1)
				ctrlTree.SetItemData(item, page);
#endif
			return item;
		}
	} else {
		tstring name = str.substr(0, i);
		HTREEITEM item = findItem(name, first);
		if(item == NULL) {
			// Doesn't exist, add...
			tvi.item.mask = TVIF_PARAM | TVIF_TEXT;
			tvi.item.lParam = -1;
			tvi.item.pszText = const_cast<LPTSTR>(name.c_str());
			item = TreeView_InsertItem(pageTree->handle(), &tvi);
		}
		TreeView_Expand(pageTree->handle(), parent, TVE_EXPAND);
		// Recurse...
		return createTree(str.substr(i+1), item, page);
	}
}

HTREEITEM SettingsDialog::findItem(const tstring& str, HTREEITEM start) {
	while(start != NULL) {
		if(pageTree->getText(start) == str) {
			return start;
		}
		start = TreeView_GetNextSibling(pageTree->handle(), start);
	}
	return start;
}

void SettingsDialog::write() {
	for(PageList::iterator i = pages.begin(); i != pages.end(); ++i) {
		(*i)->write();
	}
}

#ifdef PORT_ME

PropertiesDlg::PropertiesDlg(HWND parent, SettingsManager *s) : TreePropertySheet(CTSTRING(SETTINGS), 0, parent)
{
	// Hide "Apply" button
	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	m_psh.dwFlags &= ~PSH_HASHELP;
}

#endif
