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

#include "UploadPage.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/ShareManager.h>
#include <dcpp/version.h>
#include "WinUtil.h"
#include "StupidWin.h"
#include "LineDlg.h"
#include "HashProgressDlg.h"

PropPage::TextItem UploadPage::texts[] = {
	{ IDC_SETTINGS_SHARED_DIRECTORIES, ResourceManager::SETTINGS_SHARED_DIRECTORIES },
	{ IDC_SETTINGS_SHARE_SIZE, ResourceManager::SETTINGS_SHARE_SIZE },
	{ IDC_SHAREHIDDEN, ResourceManager::SETTINGS_SHARE_HIDDEN },
	{ IDC_REMOVE, ResourceManager::REMOVE },
	{ IDC_ADD, ResourceManager::SETTINGS_ADD_FOLDER },
	{ IDC_RENAME, ResourceManager::SETTINGS_RENAME_FOLDER },
	{ IDC_SETTINGS_UPLOADS_MIN_SPEED, ResourceManager::SETTINGS_UPLOADS_MIN_SPEED },
	{ IDC_SETTINGS_KBPS, ResourceManager::KiBPS },
	{ IDC_SETTINGS_UPLOADS_SLOTS, ResourceManager::SETTINGS_UPLOADS_SLOTS },
	{ IDC_SETTINGS_ONLY_HASHED, ResourceManager::SETTINGS_ONLY_HASHED },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item UploadPage::items[] = {
	{ IDC_SLOTS, SettingsManager::SLOTS, PropPage::T_INT },
	{ IDC_SHAREHIDDEN, SettingsManager::SHARE_HIDDEN, PropPage::T_BOOL },
	{ IDC_MIN_UPLOAD_SPEED, SettingsManager::MIN_UPLOAD_SPEED, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

UploadPage::UploadPage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_UPLOADPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items);

	directories = subclassList(IDC_DIRECTORIES);
	directories->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

	TStringList columns;
	columns.push_back(TSTRING(VIRTUAL_NAME));
	columns.push_back(TSTRING(DIRECTORY));
	columns.push_back(TSTRING(SIZE));
	directories->createColumns(columns);
	directories->setColumnWidth(0, 100);
	directories->setColumnWidth(1, directories->getSize().x - 220);
	directories->setColumnWidth(2, 100);

	StringPairList dirs = ShareManager::getInstance()->getDirectories();
	for(StringPairIter j = dirs.begin(); j != dirs.end(); j++) {
		TStringList row;
		row.push_back(Text::toT(j->first));
		row.push_back(Text::toT(j->second));
		row.push_back(Util::formatBytes(ShareManager::getInstance()->getShareSize(j->second)));
		directories->insertRow(row);
	}

	directories->onRaw(std::tr1::bind(&UploadPage::handleDoubleClick, this, _1, _2), SmartWin::Message(WM_NOTIFY, NM_DBLCLK));
	directories->onRaw(std::tr1::bind(&UploadPage::handleKeyDown, this, _1, _2), SmartWin::Message(WM_NOTIFY, LVN_KEYDOWN));
	directories->onRaw(std::tr1::bind(&UploadPage::handleItemChanged, this, _1, _2), SmartWin::Message(WM_NOTIFY, LVN_ITEMCHANGED));

	WidgetCheckBoxPtr shareHidden = subclassCheckBox(IDC_SHAREHIDDEN);
	shareHidden->onClicked(std::tr1::bind(&UploadPage::handleShareHiddenClicked, this, shareHidden));

	total = subclassStatic(IDC_TOTAL);
	total->setText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())));

	WidgetButtonPtr button = subclassButton(IDC_RENAME);
	button->onClicked(std::tr1::bind(&UploadPage::handleRenameClicked, this));

	button = subclassButton(IDC_REMOVE);
	button->onClicked(std::tr1::bind(&UploadPage::handleRemoveClicked, this));

	button = subclassButton(IDC_ADD);
	button->onClicked(std::tr1::bind(&UploadPage::handleAddClicked, this));

	WidgetSpinnerPtr spinner = subclassSpinner(IDC_SLOTSPIN);
	spinner->setRange(1, UD_MAXVAL);

	spinner = subclassSpinner(IDC_MIN_UPLOAD_SPIN);
	spinner->setRange(0, UD_MAXVAL);
}

UploadPage::~UploadPage() {
}

void UploadPage::write()
{
	PropPage::write(handle(), items);

	if(SETTING(SLOTS) < 1)
		SettingsManager::getInstance()->set(SettingsManager::SLOTS, 1);

	ShareManager::getInstance()->refresh();
}

HRESULT UploadPage::handleDoubleClick(WPARAM wParam, LPARAM lParam) {
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

HRESULT UploadPage::handleKeyDown(WPARAM wParam, LPARAM lParam) {
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

HRESULT UploadPage::handleItemChanged(WPARAM wParam, LPARAM lParam) {
	BOOL hasSelection = directories->hasSelection() ? TRUE : FALSE;
	::EnableWindow(::GetDlgItem(handle(), IDC_RENAME), hasSelection);
	::EnableWindow(::GetDlgItem(handle(), IDC_REMOVE), hasSelection);
	return 0;
}

void UploadPage::handleShareHiddenClicked(WidgetCheckBoxPtr checkBox) {
	// Save the checkbox state so that ShareManager knows to include/exclude hidden files
	Item i = items[1]; // The checkbox. Explicit index used - bad!
	SettingsManager::getInstance()->set((SettingsManager::IntSetting)i.setting, checkBox->getChecked());

	// Refresh the share. This is a blocking refresh. Might cause problems?
	// Hopefully people won't click the checkbox enough for it to be an issue. :-)
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true, false, true);

	// Clear the GUI list, for insertion of updated shares
	directories->removeAllRows();
	StringPairList dirs = ShareManager::getInstance()->getDirectories();
	for(StringPairIter j = dirs.begin(); j != dirs.end(); j++)
	{
		TStringList row;
		row.push_back(Text::toT(j->first));
		row.push_back(Text::toT(j->second));
		row.push_back(Util::formatBytes(ShareManager::getInstance()->getShareSize(j->second)));
		directories->insertRow(row);
	}

	// Display the new total share size
	total->setText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())));
}

void UploadPage::handleRenameClicked() {
	bool setDirty = false;

	int i = -1;
	while((i = directories->getNextItem(i, LVNI_SELECTED)) != -1) {
		tstring vName = directories->getCellText(0, i);
		tstring rPath = directories->getCellText(1, i);
		try {
			LineDlg dlg(this, TSTRING(VIRTUAL_NAME), TSTRING(VIRTUAL_NAME_LONG), vName);
			if(dlg.run() == IDOK) {
				tstring line = dlg.getLine();
				if (Util::stricmp(vName, line) != 0) {
					ShareManager::getInstance()->renameDirectory(Text::fromT(rPath), Text::fromT(line));
					directories->setCellText(0, i, line);

					setDirty = true;
				} else {
					createMessageBox().show(TSTRING(SKIP_RENAME), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_OK, WidgetMessageBox::BOX_ICONINFORMATION);
				}
			}
		} catch(const ShareException& e) {
			createMessageBox().show(Text::toT(e.getError()), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_OK, WidgetMessageBox::BOX_ICONSTOP);
		}
	}

	if(setDirty)
		ShareManager::getInstance()->setDirty();
}

void UploadPage::handleRemoveClicked() {
	int i = -1;
	while((i = directories->getNextItem(-1, LVNI_SELECTED)) != -1) {
		ShareManager::getInstance()->removeDirectory(Text::fromT(directories->getCellText(1, i)));
		total->setText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())));
		directories->removeRow(i);
	}
}

void UploadPage::handleAddClicked() {
	tstring target;
	if(WinUtil::browseDirectory(target, handle())) {
		addDirectory(target);
		HashProgressDlg(this, true).run();
	}
}

void UploadPage::addDirectory(const tstring& aPath) {
	tstring path = aPath;
	if( path[ path.length() -1 ] != _T('\\') )
		path += _T('\\');

	try {
		LineDlg dlg(this, TSTRING(VIRTUAL_NAME), TSTRING(VIRTUAL_NAME_LONG), Text::toT(ShareManager::getInstance()->validateVirtual(Util::getLastDir(Text::fromT(path)))));
		if(dlg.run() == IDOK) {
			tstring line = dlg.getLine();
			ShareManager::getInstance()->addDirectory(Text::fromT(path), Text::fromT(line));
			TStringList row;
			row.push_back(line);
			row.push_back(path);
			row.push_back(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize(Text::fromT(path)))));
			directories->insertRow(row);
			total->setText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())));
		}
	} catch(const ShareException& e) {
		createMessageBox().show(Text::toT(e.getError()), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_OK, WidgetMessageBox::BOX_ICONSTOP);
	}
}

#ifdef PORT_ME

LRESULT UploadPage::onDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/){
	HDROP drop = (HDROP)wParam;
	TCHAR buf[MAX_PATH];
	UINT nrFiles;

	nrFiles = DragQueryFile(drop, (UINT)-1, NULL, 0);

	for(UINT i = 0; i < nrFiles; ++i){
		if(DragQueryFile(drop, i, buf, MAX_PATH)){
			if(PathIsDirectory(buf))
				addDirectory(buf);
		}
	}

	DragFinish(drop);

	return 0;
}

LRESULT UploadPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UPLOADPAGE);
	return 0;
}

LRESULT UploadPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UPLOADPAGE);
	return 0;
}
#endif
