#include "stdafx.h"
#include "dcplusplus.h"
#include "UploadPage.h"
#include "Util.h"
#include "ShareManager.h"
#include "SettingsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item UploadPage::items[] = {
	{ IDC_SLOTS, SettingsManager::SLOTS, PropPage::T_INT }, 
	{ 0, 0, PropPage::T_END }
};

UploadPage::UploadPage(SettingsManager *s) : PropPage(s)
{
}

UploadPage::~UploadPage()
{
}

LRESULT UploadPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ctrlDirectories.Attach(GetDlgItem(IDC_DIRECTORIES));
	ctrlTotal.Attach(GetDlgItem(IDC_TOTAL));

	PropPage::read((HWND)*this, items);

	// Prepare shared dir list
	ctrlDirectories.InsertColumn(0, "Directory", LVCFMT_LEFT, 277, 0);
	ctrlDirectories.InsertColumn(1, "Size", LVCFMT_RIGHT, 90, 1);
	StringList directories = ShareManager::getInstance()->getDirectories();
	for(StringIter j = directories.begin(); j != directories.end(); j++)
	{
		int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), *j);
		ctrlDirectories.SetItemText(i, 1, Util::formatBytes(ShareManager::getInstance()->getShareSize(*j)).c_str());
	}
	
	ctrlTotal.SetWindowText(Util::formatBytes(ShareManager::getInstance()->getShareSize()).c_str());

	CUpDownCtrl updown;
	updown.Attach(GetDlgItem(IDC_SLOTSPIN));
	updown.SetRange(1, 100);	
	return TRUE;
}

void UploadPage::write()
{
	PropPage::write((HWND)*this, items);

	if(SETTING(SLOTS) < 1)
		settings->set(SettingsManager::SLOTS, 1);

	// Do specialized writing here
	ShareManager::getInstance()->refresh();
}

LRESULT UploadPage::onItemchangedDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	
	if(lv->uNewState & LVIS_FOCUSED)
		::EnableWindow(GetDlgItem(IDC_REMOVE), TRUE);
	return 0;		
}

LRESULT UploadPage::onClickedAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	string target;
	if(Util::browseDirectory(target)) {
		try {
			ShareManager::getInstance()->addDirectory(target);
			int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), target);
			ctrlDirectories.SetItemText(i, 1, Util::formatBytes(ShareManager::getInstance()->getShareSize(target)).c_str());
			ctrlTotal.SetWindowText(Util::formatBytes(ShareManager::getInstance()->getShareSize()).c_str());
		} catch(ShareException e) {
			MessageBox(e.getError().c_str());
		}
	}
	
	return 0;
}

LRESULT UploadPage::onClickedRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char buf[MAX_PATH];
	LVITEM item;
	::ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.cchTextMax = sizeof(buf);
	item.pszText = buf;
	if(ctrlDirectories.GetSelectedItem(&item)) {
		ShareManager::getInstance()->removeDirectory(buf);
		ctrlTotal.SetWindowText(Util::formatBytes(ShareManager::getInstance()->getShareSize()).c_str());
		ctrlDirectories.DeleteItem(item.iItem);
	}
	
	return 0;
}