/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/HttpConnection.h"
#include "../client/SimpleXML.h"

static const char thanks[] = "Big thanks to all donators and people who have contributed with ideas "
"and code! Thanks go out to sourceforge for hosting the project. This application uses libbzip2, "
"thanks to Julian R Seward and team for providing it. This application uses STLPort "
"(www.stlport.org), a most excellent STL package. The following people have contributed code to "
"DC++ (I hope I haven't missed someone, they're in roughly chronological order...=):\r\n"
"geoff, carxor, luca rota, dan kline, mike, anton, zc, sarf, farcry, kyrre aalerud, opera, "
"patbateman, xeroc, fusbar, vladimir marko, kenneth skovhede, ondrea, todd pederzani, who, "
"sedulus, sandos, henrik engström, dwomac, robert777, saurod, atomicjo, bzbetty, orkblutt, "
"distiller, citruz, dan fulger, cologic, christer palm, twink. Keep it coming!";

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
		SetDlgItemText(IDC_VERSION, "DC++ v" VERSIONSTRING "\n(c) Copyright 2001-2003 Jacek Sieka\nCodeveloper: Per Lindén\nGraphics: Martin Skogevall\nDC++ is licenced under GPL\nhttp://dcplusplus.sourceforge.net/");
		CEdit ctrl(GetDlgItem(IDC_THANKS));
		ctrl.FmtLines(TRUE);
		ctrl.AppendText(thanks, TRUE);
		ctrl.Detach();
		SetDlgItemText(IDC_LATEST, CSTRING(DOWNLOADING));
		SetDlgItemText(IDC_TOTALS, ("Upload: " + Util::formatBytes(SETTING(TOTAL_UPLOAD)) + ", Download: " + 
			Util::formatBytes(SETTING(TOTAL_DOWNLOAD))).c_str());

		if(SETTING(TOTAL_DOWNLOAD) > 0) {
			char buf[64];
			sprintf(buf, "Ratio (up/down): %.2f", ((double)SETTING(TOTAL_UPLOAD)) / ((double)SETTING(TOTAL_DOWNLOAD)));
			SetDlgItemText(IDC_RATIO, buf);
		}
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
	
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const u_int8_t* buf, int len) throw() {
		switch(type) {
		case HttpConnectionListener::DATA:
			downBuf.append((char*)buf, len); break;
		default:
			dcassert(0);
		}
	}

	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const string& aLine) throw() {
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
			break;
		case HttpConnectionListener::FAILED: 
			c.removeListener(this);
			{
				string* x = new string(aLine);
				PostMessage(WM_VERSIONDATA, (WPARAM) x);
			}
			break;
		}
	}

	string downBuf;
};

#endif // !defined(AFX_ABOUTDLG_H__D12815FA_21C0_4C20_9718_892C9F8CD196__INCLUDED_)

/**
 * @file
 * $Id: AboutDlg.h,v 1.18 2003/11/14 15:37:36 arnetheduck Exp $
 */

