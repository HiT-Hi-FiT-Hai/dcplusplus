/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

#if !defined(AFX_ABOUTDLG_H__D12815FA_21C0_4C20_9718_892C9F8CD196__INCLUDED_)
#define AFX_ABOUTDLG_H__D12815FA_21C0_4C20_9718_892C9F8CD196__INCLUDED_

#include "../client/HttpConnection.h"
#include "../client/SimpleXML.h"

class CAboutDlg : public CDialogImpl<CAboutDlg>, private HttpConnectionListener
{
public:
	enum { IDD = IDD_ABOUTBOX };
	enum { WM_VERSIONDATA = WM_APP + 53 };

	CAboutDlg() { };
	virtual ~CAboutDlg() { };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_VERSIONDATA, onVersionData)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		SetDlgItemText(IDC_VERSION, "DC++ v" VERSIONSTRING "\n(c) Copyright 2001-2002 Jacek Sieka\n\nhttp://dcplusplus.sourceforge.net/");
		SetDlgItemText(IDC_THANKS, "Thanks go out to sourceforge for hosting the project and nro for hosting the first and (so far) only mirror...more thanks go out to the people testing the application and to all those who have been discussing it on sourceforge and all over the world...you probably know who you are...also, thanks to the bzip2 team, they're the ones that made the bzip2 compression library used in DC++");
		SetDlgItemText(IDC_LATEST, CSTRING(DOWNLOADING));
		CenterWindow(GetParent());
		c.addListener(this);
		c.downloadFile("http://dcplusplus.sourceforge.net/version.xml");
		return TRUE;
	}

	LRESULT onVersionData(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		string* x = (string*) wParam;
		SetDlgItemText(IDC_LATEST, x->c_str());
		delete x;
		return 0;
	}
		
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}

private:
	HttpConnection c;

	CAboutDlg(const CAboutDlg&) { dcassert(0); };
	
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const u_int8_t* buf, int len) {
		switch(type) {
		case HttpConnectionListener::DATA:
			downBuf.append((char*)buf, len); break;
		default:
			dcassert(0);
		}
	}

	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const string& aLine) {
		switch(type) {
		case HttpConnectionListener::FAILED: 
			c.removeListener(this);
			{
				string* x = new string(aLine);
				PostMessage(WM_VERSIONDATA, (WPARAM) x);
			}
		}
	}

	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/) {
		switch(type) {
		case HttpConnectionListener::COMPLETE:
			c.removeListener(this);
			if(!downBuf.empty()) {
				SimpleXML xml;
				xml.fromXML(downBuf);
				if(xml.findChild("DCUpdate")) {
					xml.stepIn();
					if(xml.findChild("Version")) {
						string* x = new string(xml.getChildData());
						PostMessage(WM_VERSIONDATA, (WPARAM) x);
					}
				}
			}
		}
	}

	string downBuf;
};

#endif // !defined(AFX_ABOUTDLG_H__D12815FA_21C0_4C20_9718_892C9F8CD196__INCLUDED_)

/**
 * @file AboutDlg.h
 * $Id: AboutDlg.h,v 1.4 2002/04/28 08:25:50 arnetheduck Exp $
 */

