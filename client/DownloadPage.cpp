#include "stdafx.h"
#include "dcplusplus.h"
#include "DownloadPage.h"
#include "SettingsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item DownloadPage::items[] = {
	{ IDC_DOWNLOADDIR,	SettingsManager::DOWNLOAD_DIRECTORY, PropPage::T_STR }, 
	{ 0, 0, PropPage::T_END }
};

LRESULT DownloadPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::read((HWND)*this, items);

	// Do specialized reading here
	return TRUE;
}

void DownloadPage::write()
{
	
	PropPage::write((HWND)*this, items);
	
	const string& s = SETTING(DOWNLOAD_DIRECTORY);
	if(s.length() > 0 && s[s.length() - 1] != '\\') {
		SettingsManager::getInstance()->set(SettingsManager::DOWNLOAD_DIRECTORY, s + '\\');
	}
}

LRESULT DownloadPage::onClickedBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	string dir;
	if(Util::browseDirectory(dir))
	{
		// Adjust path string
		if(dir.size() > 0 && dir[dir.size() - 1] != '\\')
			dir += '\\';
	
		SetDlgItemText(IDC_DOWNLOADDIR, dir.c_str());
	}
	return 0;
}
