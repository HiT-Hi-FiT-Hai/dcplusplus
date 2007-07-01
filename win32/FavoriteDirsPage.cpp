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

#include "FavoriteDirsPage.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/version.h>
#include "WinUtil.h"
#include "StupidWin.h"
#include "LineDlg.h"

PropPage::TextItem FavoriteDirsPage::texts[] = {
	{ IDC_SETTINGS_FAVORITE_DIRECTORIES, ResourceManager::SETTINGS_FAVORITE_DIRS },
	{ IDC_REMOVE, ResourceManager::REMOVE },
	{ IDC_ADD, ResourceManager::SETTINGS_ADD_FOLDER },
	{ IDC_RENAME, ResourceManager::SETTINGS_RENAME_FOLDER },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

FavoriteDirsPage::FavoriteDirsPage(SmartWin::Widget* parent) : SmartWin::Widget(parent), PropPage() {
	createDialog(IDD_FAVORITE_DIRSPAGE);

	PropPage::translate(handle(), texts);

	directories = static_cast<WidgetDataGridPtr>(subclassList(IDC_FAVORITE_DIRECTORIES));
	directories->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

	TStringList columns;
	columns.push_back(TSTRING(FAVORITE_DIR_NAME));
	columns.push_back(TSTRING(DIRECTORY));
	directories->createColumns(columns);
	directories->setColumnWidth(0, 100);
	directories->setColumnWidth(1, directories->getSize().x - 120);

	StringPairList dirs = FavoriteManager::getInstance()->getFavoriteDirs();
	for(StringPairIter j = dirs.begin(); j != dirs.end(); j++) {
		TStringList row;
		row.push_back(Text::toT(j->second));
		row.push_back(Text::toT(j->first));
		directories->insertRow(row);
	}

	directories->onRaw(&FavoriteDirsPage::handleDoubleClick, SmartWin::Message(WM_NOTIFY, NM_DBLCLK));
	directories->onRaw(&FavoriteDirsPage::handleKeyDown, SmartWin::Message(WM_NOTIFY, LVN_KEYDOWN));
	directories->onRaw(&FavoriteDirsPage::handleItemChanged, SmartWin::Message(WM_NOTIFY, LVN_ITEMCHANGED));

	WidgetButtonPtr button = subclassButton(IDC_RENAME);
	button->onClicked(&FavoriteDirsPage::handleRenameClicked);

	button = subclassButton(IDC_REMOVE);
	button->onClicked(&FavoriteDirsPage::handleRemoveClicked);

	button = subclassButton(IDC_ADD);
	button->onClicked(&FavoriteDirsPage::handleAddClicked);
}

FavoriteDirsPage::~FavoriteDirsPage() {
}

void FavoriteDirsPage::write()
{
//	PropPage::write(handle(), items);
}

HRESULT FavoriteDirsPage::handleDoubleClick(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/) {
#ifdef PORT_ME // posting messages doesn't seem to do anything
	LPNMITEMACTIVATE item = (LPNMITEMACTIVATE)lParam;
	if(item->iItem >= 0) {
		StupidWin::postMessage(this, WM_COMMAND, IDC_RENAME);
	} else if(item->iItem == -1) {
		StupidWin::postMessage(this, WM_COMMAND, IDC_ADD);
	}
#endif
	return 0;
}

HRESULT FavoriteDirsPage::handleKeyDown(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/) {
#ifdef PORT_ME // posting messages doesn't seem to do anything
	switch(((LPNMLVKEYDOWN)lParam)->wVKey) {
	case VK_INSERT:
		StupidWin::postMessage(this, WM_COMMAND, IDC_ADD);
		break;
	case VK_DELETE:
		StupidWin::postMessage(this, WM_COMMAND, IDC_REMOVE);
		break;
	}
#endif
	return 0;
}

HRESULT FavoriteDirsPage::handleItemChanged(DataGridMessageType, LPARAM /*lParam*/, WPARAM /*wParam*/) {
	BOOL hasSelection = directories->hasSelection() ? TRUE : FALSE;
	::EnableWindow(::GetDlgItem(handle(), IDC_RENAME), hasSelection);
	::EnableWindow(::GetDlgItem(handle(), IDC_REMOVE), hasSelection);
	return 0;
}

void FavoriteDirsPage::handleRenameClicked(WidgetButtonPtr) {
	int i = -1;
	while((i = directories->getNextItem(i, LVNI_SELECTED)) != -1) {
		tstring old = directories->getCellText(0, i);
		LineDlg dlg(this, TSTRING(FAVORITE_DIR_NAME), TSTRING(FAVORITE_DIR_NAME_LONG), old);
		if(dlg.run() == IDOK) {
			tstring line = dlg.getLine();
			if (FavoriteManager::getInstance()->renameFavoriteDir(Text::fromT(old), Text::fromT(line))) {
				directories->setCellText(0, i, line);
			} else {
				createMessageBox().show(TSTRING(DIRECTORY_ADD_ERROR), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_OK, WidgetMessageBox::BOX_ICONSTOP);
			}
		}
	}
}

void FavoriteDirsPage::handleRemoveClicked(WidgetButtonPtr) {
	int i = -1;
	while((i = directories->getNextItem(-1, LVNI_SELECTED)) != -1)
		if(FavoriteManager::getInstance()->removeFavoriteDir(Text::fromT(directories->getCellText(1, i))))
			directories->removeRow(i);
}

void FavoriteDirsPage::handleAddClicked(WidgetButtonPtr) {
	tstring target;
	if(WinUtil::browseDirectory(target, handle()))
		addDirectory(target);
}

void FavoriteDirsPage::addDirectory(const tstring& aPath) {
	tstring path = aPath;
	if( path[ path.length() -1 ] != PATH_SEPARATOR )
		path += PATH_SEPARATOR;

	LineDlg dlg(this, TSTRING(FAVORITE_DIR_NAME), TSTRING(FAVORITE_DIR_NAME_LONG), Util::getLastDir(path));
	if(dlg.run() == IDOK) {
		tstring line = dlg.getLine();
		if (FavoriteManager::getInstance()->addFavoriteDir(Text::fromT(path), Text::fromT(line))) {
			TStringList row;
			row.push_back(line);
			row.push_back(path);
			directories->insertRow(row);
		} else {
			createMessageBox().show(TSTRING(DIRECTORY_ADD_ERROR), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_OK, WidgetMessageBox::BOX_ICONSTOP);
		}
	}
}

#ifdef PORT_ME

LRESULT FavoriteDirsPage::onDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/){
	HDROP drop = (HDROP)wParam;
	AutoArray<TCHAR> buf(MAX_PATH);
	UINT nrFiles;

	nrFiles = DragQueryFile(drop, (UINT)-1, NULL, 0);

	for(UINT i = 0; i < nrFiles; ++i){
		if(DragQueryFile(drop, i, buf, MAX_PATH)){
			if(PathIsDirectory(buf))
				addDirectory(tstring(buf));
		}
	}

	DragFinish(drop);

	return 0;
}

LRESULT FavoriteDirsPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FAVORITE_DIRSPAGE);
	return 0;
}

LRESULT FavoriteDirsPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FAVORITE_DIRSPAGE);
	return 0;
}
#endif
