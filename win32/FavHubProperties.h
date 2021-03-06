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

#ifndef DCPLUSPLUS_WIN32_FAV_HUB_PROPERTIES_H
#define DCPLUSPLUS_WIN32_FAV_HUB_PROPERTIES_H

#include <dcpp/forward.h>
#include "WidgetFactory.h"

class FavHubProperties : public WidgetFactory<dwt::ModalDialog>
{
public:
	FavHubProperties(dwt::Widget* parent, FavoriteHubEntry *_entry);
	virtual ~FavHubProperties();

	int run() { return createDialog(IDD_FAVORITEHUB); }

private:
	TextBoxPtr name;
	TextBoxPtr address;
	TextBoxPtr description;
	TextBoxPtr nick;
	TextBoxPtr password;
	TextBoxPtr userDescription;

	FavoriteHubEntry *entry;

	bool handleInitDialog();
	void handleFocus();

	void handleTextChanged(TextBoxPtr textBox);

	void handleOKClicked();
};

#endif // !defined(DCPLUSPLUS_WIN32_FAV_HUB_PROPERTIES_H)
