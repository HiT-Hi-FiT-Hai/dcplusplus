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

#include "SettingsDialog.h"
#include "resource.h"

#include "GeneralPage.h"
#include <client/Pointer.h>

static const TCHAR SEPARATOR = _T('\\');
static const size_t MAX_NAME_LENGTH = 256;

SettingsDialog::SettingsDialog(SmartWin::Widget* parent) : SmartWin::Widget(parent) {
	onInitDialog(&SettingsDialog::initDialog);
}

bool SettingsDialog::initDialog() {
	pageTree = subclassTreeView(IDC_SETTINGS_PAGES);
	
	setText(TSTRING(SETTINGS));
	
	addPage(TSTRING(SETTINGS_GENERAL), new GeneralPage);
	
	return true;
}

SettingsDialog::~SettingsDialog() {
	std::for_each(pages.begin(), pages.end(), DeleteFunction());
}

int SettingsDialog::run() {
	return createDialog(IDD_SETTINGS);
}

void SettingsDialog::addPage(const tstring& title, PropPage* page) {
	pages.push_back(page);
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
#ifdef PORT_ME
			ctrlTree.Expand(parent);
#endif
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
#ifdef PORT_ME
		ctrlTree.Expand(parent);
#endif
		// Recurse...
		return createTree(str.substr(i+1), item, page);
	}
}

HTREEITEM SettingsDialog::findItem(const tstring& str, HTREEITEM start) {
	TCHAR buf[MAX_NAME_LENGTH];

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

#include "DownloadPage.h"
#include "UploadPage.h"
#include "AppearancePage.h"
#include "AdvancedPage.h"
#include "LogPage.h"
#include "UCPage.h"
#include "FavoriteDirsPage.h"
#include "Appearance2Page.h"
#include "Advanced3Page.h"
#include "NetworkPage.h"
#include "WindowsPage.h"
#include "QueuePage.h"
#include "CertificatesPage.h"
#include "TabsPage.h"

PropertiesDlg::PropertiesDlg(HWND parent, SettingsManager *s) : TreePropertySheet(CTSTRING(SETTINGS), 0, parent)
{
	int n = 0;
	pages[n++] = new GeneralPage(s);
	pages[n++] = new NetworkPage(s);
	pages[n++] = new DownloadPage(s);
	pages[n++] = new FavoriteDirsPage(s);
	pages[n++] = new QueuePage(s);
	pages[n++] = new UploadPage(s);
	pages[n++] = new AppearancePage(s);
	pages[n++] = new Appearance2Page(s);
	pages[n++] = new TabsPage(s);
	pages[n++] = new WindowsPage(s);
	pages[n++] = new AdvancedPage(s);
	pages[n++] = new LogPage(s);
	pages[n++] = new Advanced3Page(s);
	pages[n++] = new UCPage(s);
	pages[n++] = new CertificatesPage(s);

	// Hide "Apply" button
	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	m_psh.dwFlags &= ~PSH_HASHELP;
}

LRESULT PropertiesDlg::onOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	write();
	bHandled = FALSE;
	return TRUE;
}
#endif
