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

#include "resource.h"

#include "ADLSProperties.h"

#include <dcpp/ADLSearch.h>
#include <dcpp/FavoriteManager.h>
#include "WinUtil.h"

static const WinUtil::HelpItem helpItems[] = {
	{ IDC_ADLSP_SEARCH, IDH_ADLSP_SEARCH_STRING },
	{ IDC_SEARCH_STRING, IDH_ADLSP_SEARCH_STRING },
	{ IDC_ADLSP_TYPE, IDH_ADLSP_SOURCE_TYPE },
	{ IDC_SOURCE_TYPE, IDH_ADLSP_SOURCE_TYPE },
	{ IDC_ADLSP_SIZE_MIN, IDH_ADLSP_MIN_FILE_SIZE },
	{ IDC_MIN_FILE_SIZE, IDH_ADLSP_MIN_FILE_SIZE },
	{ IDC_ADLSP_SIZE_MAX, IDH_ADLSP_MAX_FILE_SIZE },
	{ IDC_MAX_FILE_SIZE, IDH_ADLSP_MAX_FILE_SIZE },
	{ IDC_ADLSP_UNITS, IDH_ADLSP_SIZE_TYPE },
	{ IDC_SIZE_TYPE, IDH_ADLSP_SIZE_TYPE },
	{ IDC_ADLSP_DESTINATION, IDH_ADLSP_DEST_DIR },
	{ IDC_DEST_DIR, IDH_ADLSP_DEST_DIR },
	{ IDC_IS_ACTIVE, IDH_ADLSP_ENABLED },
	{ IDC_AUTOQUEUE, IDH_ADLSP_AUTOQUEUE },
	{ IDOK, IDH_DCPP_OK },
	{ IDCANCEL, IDH_DCPP_CANCEL },
	{ 0, 0 }
};

ADLSProperties::ADLSProperties(SmartWin::Widget* parent, ADLSearch *_search) :
	WidgetFactory<SmartWin::ModalDialog>(parent),
	searchString(0),
	searchType(0),
	minSize(0),
	maxSize(0),
	sizeType(0),
	destDir(0),
	active(0),
	autoQueue(0),
	search(_search)
{
	onInitDialog(std::tr1::bind(&ADLSProperties::handleInitDialog, this));
	onFocus(std::tr1::bind(&ADLSProperties::handleFocus, this));
}

ADLSProperties::~ADLSProperties() {
}

bool ADLSProperties::handleInitDialog() {
	setHelpId(IDH_ADLSP);

	WinUtil::setHelpIds(this, helpItems);

	// Translate dialog
	setText(T_("ADLSearch Properties"));
	setItemText(IDC_ADLSP_SEARCH, T_("Search String"));
	setItemText(IDC_ADLSP_TYPE, T_("Source Type"));
	setItemText(IDC_ADLSP_SIZE_MIN, T_("Min FileSize"));
	setItemText(IDC_ADLSP_SIZE_MAX, T_("Max FileSize"));
	setItemText(IDC_ADLSP_UNITS, T_("Size Type"));
	setItemText(IDC_ADLSP_DESTINATION, T_("Destination Directory"));

	searchString = attachTextBox(IDC_SEARCH_STRING);
	searchString->setText(Text::toT(search->searchString));
	searchString->setFocus();

	searchType = attachComboBox(IDC_SOURCE_TYPE);
	searchType->addValue(T_("Filename"));
	searchType->addValue(T_("Directory"));
	searchType->addValue(T_("Full Path"));
	searchType->setSelected(search->sourceType);

	minSize = attachTextBox(IDC_MIN_FILE_SIZE);
	minSize->setText((search->minFileSize > 0) ? Text::toT(Util::toString(search->minFileSize)) : Util::emptyStringT);

	maxSize = attachTextBox(IDC_MAX_FILE_SIZE);
	maxSize->setText((search->maxFileSize > 0) ? Text::toT(Util::toString(search->maxFileSize)) : Util::emptyStringT);

	sizeType = attachComboBox(IDC_SIZE_TYPE);
	sizeType->addValue(T_("B"));
	sizeType->addValue(T_("KiB"));
	sizeType->addValue(T_("MiB"));
	sizeType->addValue(T_("GiB"));
	sizeType->setSelected(search->typeFileSize);

	destDir = attachTextBox(IDC_DEST_DIR);
	destDir->setText(Text::toT(search->destDir));

	active = attachChild<CheckBox>(IDC_IS_ACTIVE);
	active->setText(T_("Enabled"));
	active->setChecked(search->isActive);

	autoQueue = attachChild<CheckBox>(IDC_AUTOQUEUE);
	autoQueue->setText(T_("Download Matches"));
	autoQueue->setChecked(search->isAutoQueue);

	ButtonPtr button = attachChild<Button>(IDOK);
	button->onClicked(std::tr1::bind(&ADLSProperties::handleOKClicked, this));

	button = attachChild<Button>(IDCANCEL);
	button->onClicked(std::tr1::bind(&ADLSProperties::endDialog, this, IDCANCEL));

	centerWindow();
	
	return false;
}

void ADLSProperties::handleFocus() {
	searchString->setFocus();
}

void ADLSProperties::handleOKClicked() {
	search->searchString = Text::fromT(searchString->getText());

	search->sourceType = ADLSearch::SourceType(searchType->getSelected());

	tstring minFileSize = minSize->getText();
	search->minFileSize = minFileSize.empty() ? -1 : Util::toInt64(Text::fromT(minFileSize));
	
	tstring maxFileSize = maxSize->getText();
	search->maxFileSize = maxFileSize.empty() ? -1 : Util::toInt64(Text::fromT(maxFileSize));
	
	search->typeFileSize = ADLSearch::SizeType(sizeType->getSelected());

	search->destDir = Text::fromT(destDir->getText());

	search->isActive = active->getChecked();

	search->isAutoQueue = autoQueue->getChecked();

	endDialog(IDOK);
}
