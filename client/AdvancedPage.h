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

#ifndef ADVANCEDPAGE_H
#define ADVANCEDPAGE_H

#include "PropPage.h"

class AdvancedPage : public CPropertyPage<IDD_ADVANCEDPAGE>, public PropPage
{
public:
	AdvancedPage(SettingsManager *s) : PropPage(s) { };
	~AdvancedPage() { };

	BEGIN_MSG_MAP(PropPage1)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

protected:
	static Item items[];
};

#endif //ADVANCEDPAGE_H

/**
 * @file AdvancedPage.h
 * $Id: AdvancedPage.h,v 1.3 2002/03/15 15:12:35 arnetheduck Exp $
 * @if LOG
 * $Log: AdvancedPage.h,v $
 * Revision 1.3  2002/03/15 15:12:35  arnetheduck
 * 0.16
 *
 * Revision 1.2  2002/01/26 12:52:51  arnetheduck
 * More minor fixes
 *
 * @endif
 */

