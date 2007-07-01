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

#ifndef DCPLUSPLUS_WIN32_ABOUT_DLG_H
#define DCPLUSPLUS_WIN32_ABOUT_DLG_H

#include <dcpp/HttpConnection.h>
#include "resource.h"
#include "AspectSpeaker.h"

class AboutDlg :
	public SmartWin::WidgetFactory<SmartWin::WidgetModalDialog, AboutDlg, SmartWin::MessageMapPolicyModalDialogWidget>,
	public AspectSpeaker<AboutDlg>,
	private HttpConnectionListener
{
public:
	AboutDlg(SmartWin::Widget* parent);
	virtual ~AboutDlg();

	int run() { return createDialog(IDD_ABOUTBOX); }

private:
	enum Speakers {
		SPEAK_VERSIONDATA
	};

	HttpConnection c;

	string downBuf;

	bool handleInitDialog();
	HRESULT spoken(LPARAM lParam, WPARAM wParam);

	void handleOKClicked(WidgetButtonPtr);

	virtual void on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw();
	virtual void on(HttpConnectionListener::Complete, HttpConnection* conn, const string&) throw();
	virtual void on(HttpConnectionListener::Failed, HttpConnection* conn, const string& aLine) throw();
};

#endif // !defined(DCPLUSPLUS_WIN32_ABOUT_DLG_H)
