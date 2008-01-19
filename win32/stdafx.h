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

#ifndef DCPLUSPLUS_WIN32_STDAFX_H
#define DCPLUSPLUS_WIN32_STDAFX_H

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>

#include <SmartWin.h>
#include <shlobj.h>
#include <malloc.h>
#include <htmlhelp.h>
#include <libintl.h>

enum {
	WM_SPEAKER  = WM_APP + 500
};

#ifdef PORT_ME

// Fix nt4 startup
#include <multimon.h>


#endif

using namespace dcpp;
using std::tr1::placeholders::_1;
using std::tr1::placeholders::_2;

#define LOCALEDIR dcpp::Util::getLocalePath().c_str()
#define PACKAGE "dcpp-win32"
#define _(String) gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#define T_(String) Text::toT(gettext(String))
#define CT_(String) T_(String).c_str()
#ifdef UNICODE
#define TF_(String) boost::wformat(Text::toT(gettext(String)))
#define TFN_(String1,String2, N) boost::wformat(Text::toT(ngettext(String1, String2, N)))
#else
#define TF_(String) boost::format(Text::toT(gettext(String)))
#define TFN_(String1,String2, N) boost::format(Text::toT(ngettext(String1, String2, N)))
#endif
#endif
