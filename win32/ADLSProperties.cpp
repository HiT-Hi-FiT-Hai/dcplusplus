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

#include "stdafx.h"

#include "resource.h"

#include "ADLSProperties.h"

#include <dcpp/ADLSearch.h>
#include <dcpp/FavoriteManager.h>

ADLSProperties::ADLSProperties(SmartWin::Widget* parent, ADLSearch *_search) :
	WidgetFactory<SmartWin::WidgetModalDialog>(parent),
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
	// Translate dialog
	setText(TSTRING(ADLS_PROPERTIES));
	::SetDlgItemText(handle(), IDC_ADLSP_SEARCH, CTSTRING(ADLS_SEARCH_STRING));
	::SetDlgItemText(handle(), IDC_ADLSP_TYPE, CTSTRING(ADLS_TYPE));
	::SetDlgItemText(handle(), IDC_ADLSP_SIZE_MIN, CTSTRING(ADLS_SIZE_MIN));
	::SetDlgItemText(handle(), IDC_ADLSP_SIZE_MAX, CTSTRING(ADLS_SIZE_MAX));
	::SetDlgItemText(handle(), IDC_ADLSP_UNITS, CTSTRING(ADLS_UNITS));
	::SetDlgItemText(handle(), IDC_ADLSP_DESTINATION, CTSTRING(ADLS_DESTINATION));

	searchString = subclassTextBox(IDC_SEARCH_STRING);
	searchString->setText(Text::toT(search->searchString));
	searchString->setFocus();

	searchType = subclassComboBox(IDC_SOURCE_TYPE);
	searchType->addValue(TSTRING(FILENAME));
	searchType->addValue(TSTRING(DIRECTORY));
	searchType->addValue(TSTRING(ADLS_FULL_PATH));
	searchType->setSelectedIndex(search->sourceType);

	minSize = subclassTextBox(IDC_MIN_FILE_SIZE);
	minSize->setText((search->minFileSize > 0) ? Text::toT(Util::toString(search->minFileSize)) : Util::emptyStringT);

	maxSize = subclassTextBox(IDC_MAX_FILE_SIZE);
	maxSize->setText((search->maxFileSize > 0) ? Text::toT(Util::toString(search->maxFileSize)) : Util::emptyStringT);

	sizeType = subclassComboBox(IDC_SIZE_TYPE);
	sizeType->addValue(TSTRING(B));
	sizeType->addValue(TSTRING(KiB));
	sizeType->addValue(TSTRING(MiB));
	sizeType->addValue(TSTRING(GiB));
	sizeType->setSelectedIndex(search->typeFileSize);

	destDir = subclassTextBox(IDC_DEST_DIR);
	destDir->setText(Text::toT(search->destDir));

	active = subclassCheckBox(IDC_IS_ACTIVE);
	active->setText(TSTRING(ADLS_ENABLED));
	active->setChecked(search->isActive);

	autoQueue = subclassCheckBox(IDC_AUTOQUEUE);
	autoQueue->setText(TSTRING(ADLS_DOWNLOAD));
	autoQueue->setChecked(search->isAutoQueue);

	WidgetButtonPtr button = subclassButton(IDOK);
	button->onClicked(std::tr1::bind(&ADLSProperties::handleOKClicked, this));

	button = subclassButton(IDCANCEL);
	button->onClicked(std::tr1::bind(&ADLSProperties::endDialog, this, IDCANCEL));

#ifdef PORT_ME
	CenterWindow(GetParent());
#endif
	return false;
}

void ADLSProperties::handleFocus() {
	searchString->setFocus();
}

void ADLSProperties::handleOKClicked() {
	search->searchString = searchString->getText();

	search->sourceType = (ADLSearch::SourceType)searchType->getSelectedIndex();

	tstring minFileSize = minSize->getText();

	search->minFileSize = minFileSize.empty() ? -1 : Util::toInt64(Text::fromT(minFileSize));
	tstring maxFileSize = maxSize->getText();

	search->maxFileSize = maxFileSize.empty() ? -1 : Util::toInt64(Text::fromT(maxFileSize));
	search->typeFileSize = (ADLSearch::SizeType)sizeType->getSelectedIndex();

	search->destDir = destDir->getText();

	search->isActive = active->getChecked();

	search->isAutoQueue = autoQueue->getChecked();

	endDialog(IDOK);
}
