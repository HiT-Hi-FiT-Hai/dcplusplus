// LineDlg.h: interface for the LineDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LineDLG_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)
#define AFX_LineDLG_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class LineDlg : public CDialogImpl<LineDlg>
{
	CEdit ctrlLine;
	CStatic ctrlDescription;
public:
	string line;
	string description;
	string title;

	enum { IDD = IDD_LINE };
	
	BEGIN_MSG_MAP(LineDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()
		
	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlLine.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ctrlLine.Attach(GetDlgItem(IDC_LINE));
		ctrlLine.SetFocus();
		ctrlLine.SetWindowText(line.c_str());
		
		ctrlDescription.Attach(GetDlgItem(IDC_DESCRIPTION));
		ctrlDescription.SetWindowText(description.c_str());

		SetWindowText(title.c_str());
		
		CenterWindow(GetParent());
		return FALSE;
	}
	
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if(wID == IDOK) {
			char buf[256];
			GetDlgItemText(IDC_Line, buf, 256);
			Line = buf;
		}
		EndDialog(wID);
		return 0;
	}
	
};

#endif // !defined(AFX_LineDLG_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)

/**
 * @file LineDlg.h
 * $Id: LineDlg.h,v 1.1 2001/12/27 16:07:40 arnetheduck Exp $
 * @if LOG
 * $Log: LineDlg.h,v $
 * Revision 1.1  2001/12/27 16:07:40  arnetheduck
 * Replaced PasswordDlg with LineDlg
 *
 * Revision 1.1  2001/12/19 23:08:39  arnetheduck
 * Line dialog window
 *
 * @endif
 */
