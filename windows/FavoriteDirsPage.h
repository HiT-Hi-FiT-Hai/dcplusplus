#ifndef FAVORITEDIRSPAGE_H
#define FAVORITEDIRSPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"
#include "ExListViewCtrl.h"
#include "WinUtil.h"

class FavoriteDirsPage : public CPropertyPage<IDD_FAVORITE_DIRSPAGE>, public PropPage
{
public:
	FavoriteDirsPage(SettingsManager *s) : PropPage(s) { 
		SetTitle(CTSTRING(SETTINGS_FAVORITE_DIRS_PAGE));
		m_psp.dwFlags |= PSP_HASHELP;
	};
	~FavoriteDirsPage() {
		ctrlDirectories.Detach();
	};

	BEGIN_MSG_MAP(FavoriteDirsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		MESSAGE_HANDLER(WM_HELP, onHelp)
		MESSAGE_HANDLER(WM_DROPFILES, onDropFiles)
		NOTIFY_HANDLER(IDC_FAVORITE_DIRECTORIES, LVN_ITEMCHANGED, onItemchangedDirectories)
		COMMAND_ID_HANDLER(IDC_ADD, onClickedAdd)
		COMMAND_ID_HANDLER(IDC_REMOVE, onClickedRemove)
		COMMAND_ID_HANDLER(IDC_RENAME, onClickedRename)
		NOTIFY_CODE_HANDLER_EX(PSN_HELP, onHelpInfo)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onItemchangedDirectories(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT onClickedAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClickedRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClickedRename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onHelpInfo(LPNMHDR);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
	
protected:
	static TextItem texts[];
	ExListViewCtrl ctrlDirectories;

	void addDirectory(const tstring& aPath);
};

#endif //FAVORITEDIRSPAGE_H

/**
 * @file
 * $Id: FavoriteDirsPage.h,v 1.3 2004/11/09 20:29:25 arnetheduck Exp $
 */
