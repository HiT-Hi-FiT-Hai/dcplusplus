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

#ifndef DCPLUSPLUS_WIN32_A_D_L_S_PROPERTIES_H
#define DCPLUSPLUS_WIN32_A_D_L_S_PROPERTIES_H

#include <dcpp/ADLSearch.h>
#include "WidgetFactory.h"

class ADLSProperties : public WidgetFactory<SmartWin::WidgetModalDialog>
{
public:
	ADLSProperties(SmartWin::Widget* parent, ADLSearch *_search);
	virtual ~ADLSProperties();

	int run() { return createDialog(IDD_ADLS_PROPERTIES); }

private:
	WidgetTextBoxPtr searchString;
	WidgetComboBoxPtr searchType;
	WidgetTextBoxPtr minSize;
	WidgetTextBoxPtr maxSize;
	WidgetComboBoxPtr sizeType;
	WidgetTextBoxPtr destDir;
	WidgetCheckBoxPtr active;
	WidgetCheckBoxPtr autoQueue;

	ADLSearch* search;

	bool handleInitDialog();
	void handleFocus();

	void handleOKClicked();
};

#endif // !defined(DCPLUSPLUS_WIN32_A_D_L_S_PROPERTIES_H)