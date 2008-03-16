/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#include "stdafx.h"

#include "AboutDlg.h"

#include <dcpp/SimpleXML.h>
#include <dcpp/version.h>
#include "WinUtil.h"

static const char thanks[] = "Big thanks to all donators and people who have contributed with ideas "
"and code! Thanks go out to sourceforge.net for hosting the project. "
"This product uses bzip2 (www.bzip.org), thanks to Julian Seward and team for providing it. "
"This product uses zlib (www.zlib.net), thanks to Jean-loup Gailly and Mark Adler for providing it. "
"This product includes GeoIP data created by MaxMind, available from http://maxmind.com/. "
"This product includes software developed by the OpenSSL Project for use in the OpenSSL Toolkit. (http://www.openssl.org/). "
"The following people have contributed code to DC++ (I hope I haven't missed someone, they're "
"roughly in chronological order...=):\r\n"
"geoff, carxor, luca rota, dan kline, mike, anton, zc, sarf, farcry, kyrre aalerud, opera, "
"patbateman, xeroc, fusbar, vladimir marko, kenneth skovhede, ondrea, todd pederzani, who, "
"sedulus, sandos, henrik engstr\303\266m, dwomac, robert777, saurod, atomicjo, bzbetty, orkblutt, "
"distiller, citruz, dan fulger, cologic, christer palm, twink, ilkka sepp\303\244l\303\244, johnny, ciber, "
"theparanoidone, gadget, naga, tremor, joakim tosteberg, pofis, psf8500, lauris ievins, "
"defr, ullner, fleetcommand, liny, xan, olle svensson, mark gillespie, jeremy huddleston, "
"bsod, sulan, jonathan stone, tim burton, izzzo, guitarm, paka, nils maier, jens oknelid, yoji, "
"krzysztof tyszecki, poison, mikejj, pur, bigmuscle, martin, jove, bart vullings, "
"steven sheehy, tobias nygren, poy, dorian, stephan hohe, mafa_45, mikael eman, james ross,"
"stanislav maslovski, david grundberg, pavel andreev, yakov suraev. "
"Keep it coming!";

AboutDlg::AboutDlg(SmartWin::Widget* parent) : WidgetFactory<SmartWin::WidgetModalDialog>(parent) {
	onInitDialog(std::tr1::bind(&AboutDlg::handleInitDialog, this));
	onSpeaker(std::tr1::bind(&AboutDlg::handleSpeaker, this, _1, _2));
}

AboutDlg::~AboutDlg() {
}

bool AboutDlg::handleInitDialog() {
	setText(T_("About DC++"));
	
	setItemText(IDC_VERSION, Text::toT(APPNAME " " VERSIONSTRING) + T_("\n(c) Copyright 2001-2008 Jacek Sieka\nEx-codeveloper: Per Lind\303\251n\nGraphics: Martin Skogevall et al.\nDC++ is licenced under GPL\nhttp://dcplusplus.sourceforge.net/"));
	setItemText(IDC_TTH, WinUtil::tth);
	setItemText(IDC_THANKS, Text::toT(thanks));
	setItemText(IDC_TOTALS, str(TF_("Upload: %1%, Download: %2%") % Text::toT(Util::formatBytes(SETTING(TOTAL_UPLOAD))) % Text::toT(Util::formatBytes(SETTING(TOTAL_DOWNLOAD)))));
	setItemText(IDC_GREETZ, T_("Greetz and Contributors"));
	setItemText(IDC_TOTALS, T_("Totals"));
	setItemText(IDC_LATEST_VERSION, T_("Latest stable version"));
	
	if(SETTING(TOTAL_DOWNLOAD) > 0) {
		setItemText(IDC_RATIO, str(TF_("Ratio (up/down): %1$0.2f") % (((double)SETTING(TOTAL_UPLOAD)) / ((double)SETTING(TOTAL_DOWNLOAD)))));
	} else {
		setItemText(IDC_RATIO, str(TF_("No transfers yet") % (((double)SETTING(TOTAL_UPLOAD)) / ((double)SETTING(TOTAL_DOWNLOAD)))));
	}
	setItemText(IDC_LATEST, T_("Downloading..."));

	attachButton(IDOK)->onClicked(std::tr1::bind(&AboutDlg::endDialog, this, IDOK));

	centerWindow();

	c.addListener(this);
	c.downloadFile("http://dcplusplus.sourceforge.net/version.xml");

	return false;
}

LRESULT AboutDlg::handleSpeaker(WPARAM wParam, LPARAM lParam) {
 	switch(wParam) {
 	case SPEAK_VERSIONDATA: {
  		boost::scoped_ptr<tstring> x(reinterpret_cast<tstring*>(lParam));
		setItemText(IDC_LATEST, *x);
	} break;
 	}
	return 0;
}

void AboutDlg::on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
	downBuf.append((char*)buf, len);
}

void AboutDlg::on(HttpConnectionListener::Complete, HttpConnection* conn, const string&) throw() {
	if(!downBuf.empty()) {
		SimpleXML xml;
		xml.fromXML(downBuf);
		if(xml.findChild("DCUpdate")) {
			xml.stepIn();
			if(xml.findChild("Version")) {
				tstring* x = new tstring(Text::toT(xml.getChildData()));
				speak(SPEAK_VERSIONDATA, reinterpret_cast<LPARAM>(x));
			}
		}
	}
	conn->removeListener(this);
}

void AboutDlg::on(HttpConnectionListener::Failed, HttpConnection* conn, const string& aLine) throw() {
	tstring* x = new tstring(Text::toT(aLine));
	speak(SPEAK_VERSIONDATA, reinterpret_cast<LPARAM>(x));
	conn->removeListener(this);
}
