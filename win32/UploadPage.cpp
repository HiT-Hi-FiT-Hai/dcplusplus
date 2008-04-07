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

#include "UploadPage.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/ShareManager.h>
#include <dcpp/version.h>
#include "WinUtil.h"
#include "LineDlg.h"
#include "HashProgressDlg.h"

static const WinUtil::HelpItem helpItems[] = {
	{ IDC_SETTINGS_SHARED_DIRECTORIES, IDH_SETTINGS_UPLOAD_DIRECTORIES },
	{ IDC_DIRECTORIES, IDH_SETTINGS_UPLOAD_DIRECTORIES },
	{ IDC_SETTINGS_ONLY_HASHED, IDH_SETTINGS_UPLOAD_DIRECTORIES },
	{ IDC_SHAREHIDDEN, IDH_SETTINGS_UPLOAD_SHAREHIDDEN },
	{ IDC_SETTINGS_SHARE_SIZE, IDH_SETTINGS_UPLOAD_DIRECTORIES },
	{ IDC_TOTAL, IDH_SETTINGS_UPLOAD_DIRECTORIES },
	{ IDC_RENAME, IDH_SETTINGS_UPLOAD_RENAME },
	{ IDC_REMOVE, IDH_SETTINGS_UPLOAD_REMOVE },
	{ IDC_ADD, IDH_SETTINGS_UPLOAD_ADD },
	{ IDC_SETTINGS_UPLOADS_MIN_SPEED, IDH_SETTINGS_UPLOAD_MIN_UPLOAD_SPEED },
	{ IDC_MIN_UPLOAD_SPEED, IDH_SETTINGS_UPLOAD_MIN_UPLOAD_SPEED },
	{ IDC_MIN_UPLOAD_SPIN, IDH_SETTINGS_UPLOAD_MIN_UPLOAD_SPEED },
	{ IDC_SETTINGS_KBPS, IDH_SETTINGS_UPLOAD_MIN_UPLOAD_SPEED },
	{ IDC_SETTINGS_UPLOADS_SLOTS, IDH_SETTINGS_UPLOAD_SLOTS },
	{ IDC_SLOTS, IDH_SETTINGS_UPLOAD_SLOTS },
	{ IDC_SLOTSPIN, IDH_SETTINGS_UPLOAD_SLOTS },
	{ 0, 0 }
};

PropPage::TextItem UploadPage::texts[] = {
	{ IDC_SETTINGS_SHARED_DIRECTORIES, N_("Shared directories") },
	{ IDC_SETTINGS_SHARE_SIZE, N_("Total size:") },
	{ IDC_SHAREHIDDEN, N_("Share hidden files") },
	{ IDC_REMOVE, N_("&Remove") },
	{ IDC_ADD, N_("&Add folder") },
	{ IDC_RENAME, N_("Rename") },
	{ IDC_SETTINGS_UPLOADS_MIN_SPEED, N_("Automatically open an extra slot if speed is below (0 = disable)") },
	{ IDC_SETTINGS_KBPS, N_("KiB/s") },
	{ IDC_SETTINGS_UPLOADS_SLOTS, N_("Upload slots") },
	{ IDC_SETTINGS_ONLY_HASHED, N_("Note; Files appear in the share only after they've been hashed!") },
	{ 0, 0 }
};

PropPage::Item UploadPage::items[] = {
	{ IDC_SLOTS, SettingsManager::SLOTS, PropPage::T_INT },
	{ IDC_SHAREHIDDEN, SettingsManager::SHARE_HIDDEN, PropPage::T_BOOL },
	{ IDC_MIN_UPLOAD_SPEED, SettingsManager::MIN_UPLOAD_SPEED, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

UploadPage::UploadPage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_UPLOADPAGE);
	setHelpId(IDH_UPLOADPAGE);

	WinUtil::setHelpIds(this, helpItems);
	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items);

	attachChild(directories, IDC_DIRECTORIES);
	directories->setTableStyle(LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

	TStringList columns;
	columns.push_back(T_("Virtual name"));
	columns.push_back(T_("Directory"));
	columns.push_back(T_("Size"));
	directories->createColumns(columns);
	directories->setColumnWidth(0, 100);
	directories->setColumnWidth(1, directories->getSize().x - 220);
	directories->setColumnWidth(2, 100);

	StringPairList dirs = ShareManager::getInstance()->getDirectories();
	for(StringPairIter j = dirs.begin(); j != dirs.end(); j++) {
		TStringList row;
		row.push_back(Text::toT(j->first));
		row.push_back(Text::toT(j->second));
		row.push_back(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize(j->second))));
		directories->insert(row);
	}

	directories->onDblClicked(std::tr1::bind(&UploadPage::handleDoubleClick, this));
	directories->onKeyDown(std::tr1::bind(&UploadPage::handleKeyDown, this, _1));
	directories->onRaw(std::tr1::bind(&UploadPage::handleItemChanged, this), SmartWin::Message(WM_NOTIFY, LVN_ITEMCHANGED));

	onDragDrop(std::tr1::bind(&UploadPage::handleDragDrop, this, _1));

	CheckBoxPtr shareHidden = attachCheckBox(IDC_SHAREHIDDEN);
	shareHidden->onClicked(std::tr1::bind(&UploadPage::handleShareHiddenClicked, this, shareHidden));

	attachChild(total, IDC_TOTAL);
	total->setText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())));

	ButtonPtr button = attachChild<SmartWin::Button>(IDC_RENAME);
	button->onClicked(std::tr1::bind(&UploadPage::handleRenameClicked, this));

	attachChild(button, IDC_REMOVE);
	button->onClicked(std::tr1::bind(&UploadPage::handleRemoveClicked, this));

	attachChild(button, IDC_ADD);
	button->onClicked(std::tr1::bind(&UploadPage::handleAddClicked, this));

	SpinnerPtr spinner;
	attachChild(spinner, IDC_SLOTSPIN);
	spinner->setRange(1, UD_MAXVAL);

	attachChild(spinner, IDC_MIN_UPLOAD_SPIN);
	spinner->setRange(0, UD_MAXVAL);

	attachTextBox(IDC_MIN_UPLOAD_SPEED);
	attachTextBox(IDC_SLOTS);
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

void UploadPage::handleDoubleClick() {
	if(directories->hasSelected()) {
		handleRenameClicked();
	} else {
		handleAddClicked();
	}
}

bool UploadPage::handleKeyDown(int c) {
	switch(c) {
	case VK_INSERT:
		handleAddClicked();
		return true;
	case VK_DELETE:
		handleRemoveClicked();
		return true;
	}
	return false;
}

LRESULT UploadPage::handleItemChanged() {
	BOOL hasSelected = directories->hasSelected() ? TRUE : FALSE;
	::EnableWindow(::GetDlgItem(handle(), IDC_RENAME), hasSelected);
	::EnableWindow(::GetDlgItem(handle(), IDC_REMOVE), hasSelected);
	return 0;
}

void UploadPage::handleDragDrop(const TStringList& files) {
	for(TStringIterC i = files.begin(); i != files.end(); ++i)
		if(PathIsDirectory(i->c_str()))
			addDirectory(*i);
}

void UploadPage::handleShareHiddenClicked(CheckBoxPtr checkBox) {
	// Save the checkbox state so that ShareManager knows to include/exclude hidden files
	Item i = items[1]; // The checkbox. Explicit index used - bad!
	SettingsManager::getInstance()->set((SettingsManager::IntSetting)i.setting, checkBox->getChecked());

	// Refresh the share. This is a blocking refresh. Might cause problems?
	// Hopefully people won't click the checkbox enough for it to be an issue. :-)
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true, false, true);

	// Clear the GUI list, for insertion of updated shares
	directories->clear();
	StringPairList dirs = ShareManager::getInstance()->getDirectories();
	for(StringPairIter j = dirs.begin(); j != dirs.end(); j++)
	{
		TStringList row;
		row.push_back(Text::toT(j->first));
		row.push_back(Text::toT(j->second));
		row.push_back(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize(j->second))));
		directories->insert(row);
	}

	// Display the new total share size
	total->setText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())));
}

void UploadPage::handleRenameClicked() {
	bool setDirty = false;

	int i = -1;
	while((i = directories->getNext(i, LVNI_SELECTED)) != -1) {
		tstring vName = directories->getText(i, 0);
		tstring rPath = directories->getText(i, 1);
		try {
			LineDlg dlg(this, T_("Virtual name"), T_("Name under which the others see the directory"), vName);
			if(dlg.run() == IDOK) {
				tstring line = dlg.getLine();
				if (Util::stricmp(vName, line) != 0) {
					ShareManager::getInstance()->renameDirectory(Text::fromT(rPath), Text::fromT(line));
					directories->setText(i, 0, line);

					setDirty = true;
				} else {
					createMessageBox().show(T_("New virtual name matches old name, skipping..."), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MessageBox::BOX_OK, MessageBox::BOX_ICONINFORMATION);
				}
			}
		} catch(const ShareException& e) {
			createMessageBox().show(Text::toT(e.getError()), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MessageBox::BOX_OK, MessageBox::BOX_ICONSTOP);
		}
	}

	if(setDirty)
		ShareManager::getInstance()->setDirty();
}

void UploadPage::handleRemoveClicked() {
	int i = -1;
	while((i = directories->getNext(-1, LVNI_SELECTED)) != -1) {
		ShareManager::getInstance()->removeDirectory(Text::fromT(directories->getText(i, 1)));
		total->setText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())));
		directories->erase(i);
	}
}

void UploadPage::handleAddClicked() {
	tstring target;
	if(createFolderDialog().open(target)) {
		addDirectory(target);
		HashProgressDlg(this, true).run();
	}
}

void UploadPage::addDirectory(const tstring& aPath) {
	tstring path = aPath;
	if( path[ path.length() -1 ] != _T('\\') )
		path += _T('\\');

	try {
		LineDlg dlg(this, T_("Virtual name"), T_("Name under which the others see the directory"), Text::toT(ShareManager::getInstance()->validateVirtual(Util::getLastDir(Text::fromT(path)))));
		if(dlg.run() == IDOK) {
			tstring line = dlg.getLine();
			ShareManager::getInstance()->addDirectory(Text::fromT(path), Text::fromT(line));
			TStringList row;
			row.push_back(line);
			row.push_back(path);
			row.push_back(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize(Text::fromT(path)))));
			directories->insert(row);
			total->setText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())));
		}
	} catch(const ShareException& e) {
		createMessageBox().show(Text::toT(e.getError()), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MessageBox::BOX_OK, MessageBox::BOX_ICONSTOP);
	}
}
