// PasswordDlg.h: interface for the PasswordDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PASSWORDDLG_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)
#define AFX_PASSWORDDLG_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class PasswordDlg : public CDialogImpl<PasswordDlg>
{
	CEdit ctrlPassword;
public:
	string password;
	
	enum { IDD = IDD_PASSWORD };
	
	BEGIN_MSG_MAP(PasswordDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()
		
	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlPassword.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ctrlPassword.Attach(GetDlgItem(IDC_PASSWORD));
		ctrlPassword.SetFocus();
		SetDlgItemText(IDC_PASSWORD, password.c_str());

		CenterWindow(GetParent());
		return FALSE;
	}
	
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if(wID == IDOK) {
			char buf[256];
			GetDlgItemText(IDC_PASSWORD, buf, 256);
			password = buf;
		}
		EndDialog(wID);
		return 0;
	}
	
};

#endif // !defined(AFX_PASSWORDDLG_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)

/**
 * @file PasswordDlg.h
 * $Id: PasswordDlg.h,v 1.1 2001/12/19 23:08:39 arnetheduck Exp $
 * @if LOG
 * $Log: PasswordDlg.h,v $
 * Revision 1.1  2001/12/19 23:08:39  arnetheduck
 * Password dialog window
 *
 * @endif
 */
