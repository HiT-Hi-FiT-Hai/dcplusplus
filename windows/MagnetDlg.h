/* 
* Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#if !defined(MAGNETDLG_H)
#define MAGNETDLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (Modders) Enjoy my liberally commented out source code.  The plan is to enable the
// magnet link add an entry to the download queue, with just the hash (if that is the
// only information the magnet contains).  DC++ has to find sources for the file anyway,
// and can take filename, size, etc. values from there.
//                                                        - GargoyleMT

class CMagnetDlg : public CDialogImpl<CMagnetDlg > {
public:
	enum { IDD = IDD_MAGNET };

	CMagnetDlg(const tstring& aHash, const tstring& aFileName) : mHash(aHash), mFileName(aFileName) { };
	virtual ~CMagnetDlg() { };

	BEGIN_MSG_MAP(CMagnetDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDOK, onCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, onCloseCmd)
		COMMAND_ID_HANDLER(IDC_MAGNET_QUEUE, onRadioButton)
		COMMAND_ID_HANDLER(IDC_MAGNET_NOTHING, onRadioButton)
		COMMAND_ID_HANDLER(IDC_MAGNET_SEARCH, onRadioButton)
	END_MSG_MAP();

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		// zombies.
		SetWindowText(CTSTRING(MAGNET_DLG_TITLE));
		CenterWindow(GetParent());

		// fill in dialog bits
		SetDlgItemText(IDC_MAGNET_HASH, CTSTRING(MAGNET_DLG_HASH));
		SetDlgItemText(IDC_MAGNET_NAME, CTSTRING(MAGNET_DLG_FILE));
		SetDlgItemText(IDC_MAGNET_QUEUE, CTSTRING(MAGNET_DLG_QUEUE));
		::ShowWindow(GetDlgItem(IDC_MAGNET_QUEUE), false);
		SetDlgItemText(IDC_MAGNET_SEARCH, CTSTRING(MAGNET_DLG_SEARCH));
		SetDlgItemText(IDC_MAGNET_NOTHING, CTSTRING(MAGNET_DLG_NOTHING));
		SetDlgItemText(IDC_MAGNET_REMEMBER, CTSTRING(MAGNET_DLG_REMEMBER));
		::ShowWindow(GetDlgItem(IDC_MAGNET_REMEMBER), false);
		SetDlgItemText(IDC_MAGNET_TEXT, CTSTRING(MAGNET_DLG_TEXT_GOOD));

		// file details
		SetDlgItemText(IDC_MAGNET_DISP_HASH, mHash.c_str());
		SetDlgItemText(IDC_MAGNET_DISP_NAME, mFileName.c_str());

		// radio button
		CheckRadioButton(IDC_MAGNET_QUEUE, IDC_MAGNET_NOTHING, IDC_MAGNET_SEARCH);

		// focus
		CEdit focusThis;
		focusThis.Attach(GetDlgItem(IDC_MAGNET_SEARCH));
		focusThis.SetFocus();

		return 0;
	}

	LRESULT onCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(wID == IDOK) {
			//if(IsDlgButtonChecked(IDC_MAGNET_REMEMBER) == BST_CHECKED) {
			//	SettingsManager::getInstance()->set(SettingsManager::MAGNET_ASK,  false);
			//	if(IsDlgButtonChecked(IDC_MAGNET_QUEUE))
			//		SettingsManager::getInstance()->set(SettingsManager::MAGNET_ACTION, SettingsManager::MAGNET_AUTO_DOWNLOAD);
			//	else if(IsDlgButtonChecked(IDC_MAGNET_SEARCH))
			//		SettingsManager::getInstance()->set(SettingsManager::MAGNET_ACTION, SettingsManager::MAGNET_AUTO_SEARCH);
			//}

			if(IsDlgButtonChecked(IDC_MAGNET_SEARCH)) {
				TTHValue tmphash(WinUtil::fromT(mHash));
				WinUtil::searchHash(&tmphash);
			} //else if(IsDlgButtonChecked(IDC_MAGNET_QUEUE)) {
				// FIXME: Write this code when the queue is more tth-centric
			//} 
		}
		EndDialog(wID);
		return 0;
	}

	LRESULT onRadioButton(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		switch(wID){
			case IDC_MAGNET_QUEUE:
            case IDC_MAGNET_SEARCH:
				//::EnableWindow(GetDlgItem(IDC_MAGNET_REMEMBER), true);
				break;
			case IDC_MAGNET_NOTHING:
				//if(IsDlgButtonChecked(IDC_MAGNET_REMEMBER) == BST_CHECKED) {
					//::CheckDlgButton(m_hWnd, IDC_MAGNET_REMEMBER, false);
				//}
				//::EnableWindow(GetDlgItem(IDC_MAGNET_REMEMBER), false);
				break;
		};
		return 0;
	}

private:
	tstring mHash, mFileName;
};

#endif // MAGNETDLG_H

/**
* @file
* $Id: MagnetDlg.h,v 1.3 2004/09/07 01:36:53 arnetheduck Exp $
*/

