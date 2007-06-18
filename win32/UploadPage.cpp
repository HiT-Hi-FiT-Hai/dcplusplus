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

#include "resource.h"

#include "UploadPage.h"

#include <client/SettingsManager.h>
#include <client/ShareManager.h>

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

UploadPage::UploadPage(SmartWin::Widget* parent) : SmartWin::Widget(parent), PropPage() {
	createDialog(IDD_UPLOADPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items);

	HWND directories = ::GetDlgItem(handle(), IDC_DIRECTORIES);
	ListView_SetExtendedListViewStyle(directories, LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

	LVCOLUMN lv = { LVCF_FMT | LVCF_WIDTH| LVCF_TEXT | LVCF_SUBITEM  };

	lv.fmt = LVCFMT_LEFT;
	lv.cx = 100;
	lv.pszText = const_cast<LPTSTR>(CTSTRING(VIRTUAL_NAME));
	lv.iSubItem = 0;
	ListView_InsertColumn(directories, 0, &lv);

	lv.fmt = LVCFMT_CENTER;
	RECT rc;
	::GetClientRect(directories, &rc);
	lv.cx = rc.right - rc.left - 220;
	lv.pszText = const_cast<LPTSTR>(CTSTRING(DIRECTORY));
	lv.iSubItem = 1;
	ListView_InsertColumn(directories, 1, &lv);

	lv.fmt = LVCFMT_RIGHT;
	lv.cx = 100;
	lv.pszText = const_cast<LPTSTR>(CTSTRING(SIZE));
	lv.iSubItem = 2;
	ListView_InsertColumn(directories, 2, &lv);

	LVITEM lvi = { LVIF_TEXT };
	StringPairList dirs = ShareManager::getInstance()->getDirectories();
	for(StringPairIter j = dirs.begin(); j != dirs.end(); j++) {
		lvi.iItem = ListView_GetItemCount(directories);
		lvi.pszText = const_cast<LPTSTR>(Text::toT(j->first).c_str());
		int i = ListView_InsertItem(directories, &lvi);
		ListView_SetItemText(directories, i, 1, const_cast<LPTSTR>(Text::toT(j->second).c_str()));
		ListView_SetItemText(directories, i, 2, const_cast<LPTSTR>(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize(j->second))).c_str()));
	}

#ifdef PORT_ME
	ctrlTotal.Attach(::GetDlgItem(handle(), IDC_TOTAL));
	ctrlTotal.SetWindowText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());

	CUpDownCtrl updown;
	updown.Attach(::GetDlgItem(handle(), IDC_SLOTSPIN));
	updown.SetRange(1, UD_MAXVAL);
	updown.Detach();
	updown.Attach(::GetDlgItem(handle(), IDC_MIN_UPLOAD_SPIN));
	updown.SetRange32(0, UD_MAXVAL);
#endif
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

LRESULT UploadPage::onItemchangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	::EnableWindow(::GetDlgItem(handle(), IDC_REMOVE), (lv->uNewState & LVIS_FOCUSED));
	::EnableWindow(::GetDlgItem(handle(), IDC_RENAME), (lv->uNewState & LVIS_FOCUSED));
	return 0;
}

LRESULT UploadPage::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	switch(kd->wVKey) {
	case VK_INSERT:
		PostMessage(WM_COMMAND, IDC_ADD, 0);
		break;
	case VK_DELETE:
		PostMessage(WM_COMMAND, IDC_REMOVE, 0);
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT UploadPage::onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;

	if(item->iItem >= 0) {
		PostMessage(WM_COMMAND, IDC_RENAME, 0);
	} else if(item->iItem == -1) {
		PostMessage(WM_COMMAND, IDC_ADD, 0);
	}

	return 0;
}

LRESULT UploadPage::onClickedAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	tstring target;
	if(WinUtil::browseDirectory(target, m_hWnd)) {
		addDirectory(target);
		HashProgressDlg(true).DoModal();
	}

	return 0;
}

LRESULT UploadPage::onClickedRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR buf[MAX_PATH];
	LVITEM item;
	::ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.cchTextMax = sizeof(buf);
	item.pszText = buf;

	int i = -1;
	while((i = ctrlDirectories.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		item.iItem = i;
		item.iSubItem = 1;
		ctrlDirectories.GetItem(&item);
		ShareManager::getInstance()->removeDirectory(Text::fromT(buf));
		ctrlTotal.SetWindowText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());
		ctrlDirectories.DeleteItem(i);
	}

	return 0;
}

LRESULT UploadPage::onClickedRename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR buf[MAX_PATH];
	LVITEM item;
	::ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.cchTextMax = sizeof(buf);
	item.pszText = buf;

	bool setDirty = false;

	int i = -1;
	while((i = ctrlDirectories.GetNextItem(i, LVNI_SELECTED)) != -1) {
		item.iItem = i;
		item.iSubItem = 0;
		ctrlDirectories.GetItem(&item);
		tstring vName = buf;
		item.iSubItem = 1;
		ctrlDirectories.GetItem(&item);
		tstring rPath = buf;
		try {
			LineDlg virt;
			virt.title = TSTRING(VIRTUAL_NAME);
			virt.description = TSTRING(VIRTUAL_NAME_LONG);
			virt.line = vName;
			if(virt.DoModal(m_hWnd) == IDOK) {
				if (Util::stricmp(buf, virt.line) != 0) {
					ShareManager::getInstance()->renameDirectory(Text::fromT(rPath), Text::fromT(virt.line));
					ctrlDirectories.SetItemText(i, 0, virt.line.c_str());

					setDirty = true;
				} else {
					MessageBox(CTSTRING(SKIP_RENAME), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONINFORMATION | MB_OK);
				}
			}
		} catch(const ShareException& e) {
			MessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
	}

	if( setDirty )
		ShareManager::getInstance()->setDirty();

	return 0;
}

LRESULT UploadPage::onClickedShareHidden(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Save the checkbox state so that ShareManager knows to include/exclude hidden files
	Item i = items[1]; // The checkbox. Explicit index used - bad!
	if(::IsDlgButtonChecked((HWND)* this, i.itemID) == BST_CHECKED){
		settings->set((SettingsManager::IntSetting)i.setting, true);
	} else {
		settings->set((SettingsManager::IntSetting)i.setting, false);
	}

	// Refresh the share. This is a blocking refresh. Might cause problems?
	// Hopefully people won't click the checkbox enough for it to be an issue. :-)
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true, false, true);

	// Clear the GUI list, for insertion of updated shares
	ctrlDirectories.DeleteAllItems();
	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for(StringPairIter j = directories.begin(); j != directories.end(); j++)
	{
		int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), Text::toT(j->first));
		ctrlDirectories.SetItemText(i, 1, Text::toT(j->second).c_str() );
		ctrlDirectories.SetItemText(i, 2, Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize(j->second))).c_str());
	}

	// Display the new total share size
	ctrlTotal.SetWindowText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());
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

void UploadPage::addDirectory(const tstring& aPath){
	tstring path = aPath;
	if( path[ path.length() -1 ] != _T('\\') )
		path += _T('\\');

	try {
		LineDlg virt;
		virt.title = TSTRING(VIRTUAL_NAME);
		virt.description = TSTRING(VIRTUAL_NAME_LONG);
		virt.line = Text::toT(ShareManager::getInstance()->validateVirtual(
			Util::getLastDir(Text::fromT(path))));
		if(virt.DoModal(m_hWnd) == IDOK) {
			ShareManager::getInstance()->addDirectory(Text::fromT(path), Text::fromT(virt.line));
			int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), virt.line );
			ctrlDirectories.SetItemText(i, 1, path.c_str());
			ctrlDirectories.SetItemText(i, 2, Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize(Text::fromT(path)))).c_str());
			ctrlTotal.SetWindowText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());
		}
	} catch(const ShareException& e) {
		MessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
	}
}
#endif
