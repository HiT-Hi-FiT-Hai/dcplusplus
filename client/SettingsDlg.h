/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#if !defined(AFX_SETTINGSDLG_H__25031C63_A95B_43D9_8A1E_892FF932890B__INCLUDED_)
#define AFX_SETTINGSDLG_H__25031C63_A95B_43D9_8A1E_892FF932890B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define SETTINGS_BUF_LEN 1024

#include "ShareManager.h"
#include "ExListViewCtrl.h"
#include "Util.h"

class SettingsDlg : public CDialogImpl<SettingsDlg>  
{
	CComboBox ctrlConnection;
	ExListViewCtrl ctrlDirectories;
	CStatic ctrlTotal;
	
public:
	string nick;
	string email;
	string description;
	string connection;
	string server;
	string port;
	string directory;
	int connectionType;
	int slots;
	
	enum { IDD = IDD_SETTINGS };
	
	BEGIN_MSG_MAP(SettingsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_HANDLER(IDC_ACTIVE, BN_CLICKED, OnClickedActive)
		COMMAND_HANDLER(IDC_PASSIVE, BN_CLICKED, OnClickedActive)
		COMMAND_HANDLER(IDC_BROWSEDIR, BN_CLICKED, onClickedBrowseDir)
		NOTIFY_HANDLER(IDC_DIRECTORIES, LVN_ITEMCHANGED, onItemchangedDirectories)
		COMMAND_HANDLER(IDC_ADD, BN_CLICKED, OnClickedAdd)
		COMMAND_HANDLER(IDC_REMOVE, BN_CLICKED, OnClickedRemove)
	END_MSG_MAP()
		
	LRESULT onItemchangedDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
		
		if(lv->uNewState & LVIS_FOCUSED) {
			::EnableWindow(GetDlgItem(IDC_REMOVE), TRUE);
		}
		return 0;		
	}

	LRESULT onClickedBrowseDir(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
		string dir;
		if(Util::browseDirectory(dir)) {
			SetDlgItemText(IDC_DOWNLOADDIR, dir.c_str());
		}
		return 0;
	}
	
	LRESULT OnClickedAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
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

	LRESULT OnClickedRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
		char buf[MAX_PATH];
		LVITEM item;
		ZeroMemory(&item, sizeof(item));
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
		
	
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ctrlConnection.Attach(GetDlgItem(IDC_CONNECTION));
		ctrlDirectories.Attach(GetDlgItem(IDC_DIRECTORIES));
		ctrlTotal.Attach(GetDlgItem(IDC_TOTAL));

		for(int i = 0; i < Settings::SPEED_LAST; i++) {
			ctrlConnection.AddString(Settings::connectionSpeeds[i]);
		}
		char buf[16];

		SetDlgItemText(IDC_NICK, nick.c_str());
		SetDlgItemText(IDC_EMAIL, email.c_str());
		SetDlgItemText(IDC_DESCRIPTION, description.c_str());
		SetDlgItemText(IDC_SERVER, server.c_str());
		SetDlgItemText(IDC_PORT, port.c_str());
		SetDlgItemText(IDC_SLOTS, itoa(slots, buf, 10));
		SetDlgItemText(IDC_DOWNLOADDIR, directory.c_str());
		
		CUpDownCtrl updown;
		updown.Attach(GetDlgItem(IDC_SLOTS));
		updown.SetRange(100, 0);
		
		if(connectionType == Settings::CONNECTION_ACTIVE) {
			CheckRadioButton(IDC_ACTIVE, IDC_PASSIVE, IDC_ACTIVE);
		} else if(connectionType == Settings::CONNECTION_PASSIVE) {
			CheckRadioButton(IDC_ACTIVE, IDC_PASSIVE, IDC_PASSIVE);
		}

		if(IsDlgButtonChecked(IDC_ACTIVE)) {
			::EnableWindow(GetDlgItem(IDC_SERVER), TRUE);
			::EnableWindow(GetDlgItem(IDC_PORT), TRUE);
		} else {
			::EnableWindow(GetDlgItem(IDC_SERVER), FALSE);
			::EnableWindow(GetDlgItem(IDC_PORT), FALSE);
		}
		
		ctrlConnection.SetCurSel(ctrlConnection.FindString(0, connection.c_str()));
		
		ctrlDirectories.InsertColumn(0, "Directory", LVCFMT_LEFT, 400, 0);
		ctrlDirectories.InsertColumn(1, "Size", LVCFMT_RIGHT, 100, 1);
		StringList directories = ShareManager::getInstance()->getDirectories();
		for(StringIter j = directories.begin(); j != directories.end(); j++) {
			int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), *j);
			ctrlDirectories.SetItemText(i, 1, Util::formatBytes(ShareManager::getInstance()->getShareSize(*j)).c_str());
		}

		ctrlTotal.SetWindowText(Util::formatBytes(ShareManager::getInstance()->getShareSize()).c_str());

		CenterWindow(GetParent());
		return TRUE;
	}
	
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if(wID == IDOK) {
			char buf[SETTINGS_BUF_LEN];
			GetDlgItemText(IDC_NICK, buf, SETTINGS_BUF_LEN);
			nick = buf;
			GetDlgItemText(IDC_EMAIL, buf, SETTINGS_BUF_LEN);
			email = buf;
			GetDlgItemText(IDC_DESCRIPTION, buf, SETTINGS_BUF_LEN);
			description = buf;
			GetDlgItemText(IDC_CONNECTION, buf, SETTINGS_BUF_LEN);
			connection = buf;
			GetDlgItemText(IDC_DOWNLOADDIR, buf, SETTINGS_BUF_LEN);
			directory = buf;
			GetDlgItemText(IDC_SERVER, buf, SETTINGS_BUF_LEN);
			server = buf;
			GetDlgItemText(IDC_PORT, buf, SETTINGS_BUF_LEN);
			port = buf;
			GetDlgItemText(IDC_SLOTS, buf, SETTINGS_BUF_LEN);
			slots = atoi(buf);
			
			if(IsDlgButtonChecked(IDC_ACTIVE)) {
				connectionType = Settings::CONNECTION_ACTIVE;
			} else if(IsDlgButtonChecked(IDC_PASSIVE)) {
				connectionType = Settings::CONNECTION_PASSIVE;
			} else {
				connectionType = -1;
			}
		}
		EndDialog(wID);
		return 0;
	}
	
	LRESULT OnClickedActive(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if(IsDlgButtonChecked(IDC_ACTIVE)) {
			::EnableWindow(GetDlgItem(IDC_SERVER), TRUE);
			::EnableWindow(GetDlgItem(IDC_PORT), TRUE);
		} else {
			::EnableWindow(GetDlgItem(IDC_SERVER), FALSE);
			::EnableWindow(GetDlgItem(IDC_PORT), FALSE);
		}
		return 0;
	}
};

#endif // !defined(AFX_SETTINGSDLG_H__25031C63_A95B_43D9_8A1E_892FF932890B__INCLUDED_)

/**
 * @file SettingsDlg.h
 * $Id: SettingsDlg.h,v 1.8 2002/01/02 16:12:33 arnetheduck Exp $
 * @if LOG
 * $Log: SettingsDlg.h,v $
 * Revision 1.8  2002/01/02 16:12:33  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.7  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.6  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.5  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.4  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.3  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.2  2001/11/22 20:42:18  arnetheduck
 * Fixed Settings dialog (Speed setting actually works now!)
 *
 * Revision 1.1  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * @endif
 */
