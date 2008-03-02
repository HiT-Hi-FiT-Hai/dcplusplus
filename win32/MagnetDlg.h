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

#ifndef DCPLUSPLUS_WIN32_MAGNET_DLG_H
#define DCPLUSPLUS_WIN32_MAGNET_DLG_H

// (Modders) Enjoy my liberally commented out source code.  The plan is to enable the
// magnet link add an entry to the download queue, with just the hash (if that is the
// only information the magnet contains).  DC++ has to find sources for the file anyway,
// and can take filename, size, etc. values from there.
//                                                        - GargoyleMT

#include "WidgetFactory.h"

class MagnetDlg : public WidgetFactory<SmartWin::WidgetModalDialog>
{
public:
	MagnetDlg(SmartWin::Widget* parent, const tstring& aHash, const tstring& aFileName);
	virtual ~MagnetDlg();

	int run() { return createDialog(IDD_MAGNET); }

private:
	//WidgetRadioButtonPtr queue;
	WidgetRadioButtonPtr search;
	WidgetRadioButtonPtr doNothing;
	//WidgetCheckBoxPtr remember;

	tstring mHash;
	tstring mFileName;

	bool handleInitDialog();
	void handleFocus();
	//void handleRadioButtonClicked(WidgetRadioButtonPtr radioButton);
	void handleOKClicked();
};

#endif // !defined(DCPLUSPLUS_WIN32_MAGNET_DLG_H)
