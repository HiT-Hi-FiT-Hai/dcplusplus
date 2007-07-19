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
#include <dcpp/DCPlusPlus.h>
#include "resource.h"

#include "SearchFrame.h"

#include <dcpp/ResourceManager.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/QueueManager.h>

int SearchFrame::columnIndexes[] = { COLUMN_FILENAME, COLUMN_NICK, COLUMN_TYPE, COLUMN_SIZE,
	COLUMN_PATH, COLUMN_SLOTS, COLUMN_CONNECTION, COLUMN_HUB, COLUMN_EXACT_SIZE, COLUMN_IP, COLUMN_TTH, COLUMN_CID };
int SearchFrame::columnSizes[] = { 200, 100, 50, 80, 100, 40, 70, 150, 80, 100, 125, 125 };
static ResourceManager::Strings columnNames[] = { ResourceManager::FILE, ResourceManager::USER, ResourceManager::TYPE, ResourceManager::SIZE,
	ResourceManager::PATH, ResourceManager::SLOTS, ResourceManager::CONNECTION,
	ResourceManager::HUB, ResourceManager::EXACT_SIZE, ResourceManager::IP_BARE, ResourceManager::TTH_ROOT, ResourceManager::CID };

TStringList SearchFrame::lastSearches;

SearchFrame::FrameSet SearchFrame::frames;

int SearchFrame::SearchInfo::getImage() {
	return sr->getType() == SearchResult::TYPE_FILE ? WinUtil::getIconIndex(Text::toT(sr->getFile())) : WinUtil::getDirIconIndex();
}

int SearchFrame::SearchInfo::compareItems(SearchInfo* a, SearchInfo* b, int col) {

	switch(col) {
		case COLUMN_TYPE:
			if(a->sr->getType() == b->sr->getType())
				return lstrcmpi(a->columns[COLUMN_TYPE].c_str(), b->columns[COLUMN_TYPE].c_str());
			else
				return(a->sr->getType() == SearchResult::TYPE_DIRECTORY) ? -1 : 1;
		case COLUMN_SLOTS:
			if(a->sr->getFreeSlots() == b->sr->getFreeSlots())
				return compare(a->sr->getSlots(), b->sr->getSlots());
			else
				return compare(a->sr->getFreeSlots(), b->sr->getFreeSlots());
		case COLUMN_SIZE:
		case COLUMN_EXACT_SIZE: return compare(a->sr->getSize(), b->sr->getSize());
		default: return lstrcmpi(a->getText(col).c_str(), b->getText(col).c_str());
	}
}


void SearchFrame::openWindow(SmartWin::Widget* mdiParent, const tstring& str /* = Util::emptyStringT */, LONGLONG size /* = 0 */, SearchManager::SizeModes mode /* = SearchManager::SIZE_ATLEAST */, SearchManager::TypeModes type /* = SearchManager::TYPE_ANY */) {
	SearchFrame* pChild = new SearchFrame(mdiParent, str, size, mode, type);
	frames.insert(pChild);
}

void SearchFrame::closeAll() {
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		StupidWin::postMessage(*i, WM_CLOSE);
}

SearchFrame::SearchFrame(SmartWin::Widget* mdiParent, const tstring& initialString_, LONGLONG initialSize_, SearchManager::SizeModes initialMode_, SearchManager::TypeModes initialType_) :
	SmartWin::Widget(mdiParent),
	BaseType(mdiParent),
	onlyFree(BOOLSETTING(SEARCH_ONLY_FREE_SLOTS)),
	bShowUI(true),
	isHash(false),
	initialString(initialString_),
	initialSize(initialSize_),
	initialMode(initialMode_),
	initialType(initialType_),
	droppedResults(0)
{
	{
		WidgetStatic::Seed cs;
		cs.exStyle = WS_EX_TRANSPARENT;
		
		searchLabel = createStatic(cs);
		searchLabel->setText(TSTRING(SEARCH_FOR));

		sizeLabel = createStatic(cs);
		sizeLabel->setText(TSTRING(SIZE));
		
		typeLabel = createStatic(cs);
		typeLabel->setText(TSTRING(FILE_TYPE));

		optionLabel = createStatic();
		optionLabel->setText(TSTRING(SEARCH_OPTIONS));

		hubsLabel = createStatic();
		hubsLabel->setText(TSTRING(HUBS));

	}

	{
		WidgetComboBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL;
		cs.exStyle = WS_EX_CLIENTEDGE;
		searchBox = createComboBox(cs);
		searchBox->setFont(WinUtil::font);
		addWidget(searchBox);
		
		for(TStringIter i = lastSearches.begin(); i != lastSearches.end(); ++i) {
			searchBox->insertValue(0, *i);
		}
	}

	{
		WidgetButton::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON;
		cs.caption = TSTRING(PURGE);
		purge = createButton(cs);

		purge->onClicked(std::tr1::bind(&SearchFrame::handlePurgeClicked, this));
	}

	{
		WidgetComboBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST;
		cs.exStyle = WS_EX_CLIENTEDGE;
		mode = createComboBox(cs);
		mode->setFont(WinUtil::font);
		addWidget(mode);

		mode->addValue(TSTRING(NORMAL));
		mode->addValue(TSTRING(AT_LEAST));
		mode->addValue(TSTRING(AT_MOST));
	}

	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_NUMBER;
		cs.exStyle = WS_EX_CLIENTEDGE;
		size = createTextBox(cs);
		size->setFont(WinUtil::font);
		addWidget(size);
	}

	{
		WidgetComboBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST;
		cs.exStyle = WS_EX_CLIENTEDGE;
		sizeMode = createComboBox(cs);
		sizeMode->setFont(WinUtil::font);
		addWidget(sizeMode);

		sizeMode->addValue(TSTRING(B));
		sizeMode->addValue(TSTRING(KiB));
		sizeMode->addValue(TSTRING(MiB));
		sizeMode->addValue(TSTRING(GiB));
		sizeMode->setSelectedIndex((initialSize == 0) ? 2 : 0);
	}

	{
		WidgetComboBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST;
		cs.exStyle = WS_EX_CLIENTEDGE;
		fileType = createComboBox(cs);
		fileType->setFont(WinUtil::font);
		addWidget(fileType);

		fileType->addValue(TSTRING(ANY));
		fileType->addValue(TSTRING(AUDIO));
		fileType->addValue(TSTRING(COMPRESSED));
		fileType->addValue(TSTRING(DOCUMENT));
		fileType->addValue(TSTRING(EXECUTABLE));
		fileType->addValue(TSTRING(PICTURE));
		fileType->addValue(TSTRING(VIDEO));
		fileType->addValue(TSTRING(DIRECTORY));
		fileType->addValue(_T("TTH"));
	}

	{
		WidgetCheckBox::Seed cs;
		cs.style |= WS_TABSTOP;
		cs.caption = TSTRING(ONLY_FREE_SLOTS);
		slots = createCheckBox(cs);
		slots->setChecked(onlyFree);

		slots->onClicked(std::tr1::bind(&SearchFrame::handleSlotsClicked, this)) ;
	}

	{
		WidgetDataGrid::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_NOCOLUMNHEADER;
		cs.exStyle = WS_EX_CLIENTEDGE;
		hubs = SmartWin::WidgetCreator<WidgetHubs>::create(this, cs);
		hubs->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
		addWidget(hubs);

		TStringList dummy;
		dummy.push_back(Util::emptyStringT);
		hubs->createColumns(dummy);

		hubs->setColor(WinUtil::textColor, WinUtil::bgColor);

		hubs->onRaw(std::tr1::bind(&SearchFrame::handleHubItemChanged, this, _1, _2), SmartWin::Message(WM_NOTIFY, LVN_ITEMCHANGED));
	}

	{
		WidgetButton::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON;
		cs.caption = TSTRING(SEARCH);
		doSearch = createButton(cs);

		doSearch->onClicked(std::tr1::bind(&SearchFrame::runSearch, this));
	}

	{
		WidgetDataGrid::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS;
		cs.exStyle = WS_EX_CLIENTEDGE;
		results = SmartWin::WidgetCreator<WidgetResults>::create(this, cs);
		results->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
		addWidget(results);

		results->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		results->setColumnOrder(WinUtil::splitTokens(SETTING(SEARCHFRAME_ORDER), columnIndexes));
		results->setColumnWidths(WinUtil::splitTokens(SETTING(SEARCHFRAME_WIDTHS), columnSizes));

		results->setColor(WinUtil::textColor, WinUtil::bgColor);
		results->setSmallImageList(WinUtil::fileImages);

		results->onRaw(std::tr1::bind(&SearchFrame::handleDoubleClick, this, _1, _2), SmartWin::Message(WM_NOTIFY, NM_DBLCLK));
		results->onRaw(std::tr1::bind(&SearchFrame::handleKeyDown, this, _1, _2), SmartWin::Message(WM_NOTIFY, LVN_KEYDOWN));
		results->onRaw(std::tr1::bind(&SearchFrame::handleContextMenu, this, _1, _2), SmartWin::Message(WM_CONTEXTMENU));
	}

	{
		WidgetCheckBox::Seed cs;
		cs.style |= WS_TABSTOP;
		cs.caption = _T("+/-");
		showUI = createCheckBox(cs);
		showUI->setChecked(bShowUI);

		showUI->onClicked(std::tr1::bind(&SearchFrame::handleShowUIClicked, this));
	}

	initStatus();
	
	statusSizes[STATUS_SHOW_UI] = 16; ///@todo get real checkbox width
	statusSizes[STATUS_DUMMY] = 16; ///@todo get real resizer width

	layout();

	onSpeaker(std::tr1::bind(&SearchFrame::handleSpeaker, this, _1, _2));

	hubs->insertItem(new HubInfo(Util::emptyStringT, TSTRING(ONLY_WHERE_OP), false));
	hubs->setRowChecked(0, false);

	ClientManager* clientMgr = ClientManager::getInstance();
	clientMgr->lock();
	clientMgr->addListener(this);
	Client::List& clients = clientMgr->getClients();
	for(Client::List::iterator it = clients.begin(); it != clients.end(); ++it) {
		Client* client = *it;
		if(!client->isConnected())
			continue;

		onHubAdded(new HubInfo(Text::toT(client->getHubUrl()), Text::toT(client->getHubName()), client->getMyIdentity().isOp()));
	}
	clientMgr->unlock();

	hubs->setColumnWidth(0, LVSCW_AUTOSIZE);

	SearchManager::getInstance()->addListener(this);

	if(!initialString.empty()) {
		lastSearches.push_back(initialString);
		searchBox->insertValue(0, initialString);
		searchBox->setSelectedIndex(0);
		mode->setSelectedIndex(initialMode);
		size->setText(Text::toT(Util::toString(initialSize)));
		fileType->setSelectedIndex(initialType);
		runSearch();
	} else {
		setText(TSTRING(SEARCH));
		mode->setSelectedIndex(1);
		fileType->setSelectedIndex(SETTING(LAST_SEARCH_TYPE));
	}
}

SearchFrame::~SearchFrame() {
}

void SearchFrame::layout() {
	const int border = 2;

	SmartWin::Rectangle r(getClientAreaSize()); 
	
	SmartWin::Rectangle rs = layoutStatus();
	mapWidget(STATUS_SHOW_UI, showUI);

	r.size.y -= rs.size.y + border;
	RECT rect = r, initialRect = rect;
	if(showUI->getChecked()) {
		const int width = 220, spacing = 50, labelH = 16, comboH = 140, lMargin = 2, rMargin = 4;

		rect.left += width;
		results->setBounds(SmartWin::Rectangle::FromRECT(rect));

		// "Search for"
		rect.right = width - rMargin;
		rect.left = lMargin;
		rect.top += 25;
		rect.bottom = rect.top + comboH + 21;
		searchBox->setBounds(SmartWin::Rectangle::FromRECT(rect));
		rect.bottom -= comboH;

		searchLabel->setBounds(SmartWin::Rectangle(rect.left + lMargin, rect.top - labelH, width - rMargin, labelH - 1));

		// "Purge"
		rect.right = rect.left + spacing;
		rect.left = lMargin;
		rect.top += 25;
		rect.bottom = rect.top + 21;
		purge->setBounds(SmartWin::Rectangle::FromRECT(rect));
		
		// "Search"
		rect.right = width - rMargin;
		rect.left = rect.right - 100;
		doSearch->setBounds(SmartWin::Rectangle::FromRECT(rect));
		
		// "Size"
		int w2 = width - rMargin - lMargin;
		rect.top += spacing;
		rect.bottom = rect.top + comboH;
		rect.left = lMargin;
		rect.right = w2/3;
		mode->setBounds(SmartWin::Rectangle::FromRECT(rect));

		sizeLabel->setBounds(SmartWin::Rectangle(rect.left + lMargin, rect.top - labelH, width - rMargin, labelH - 1));

		rect.left = rect.right + lMargin;
		rect.right += w2/3;
		rect.bottom = rect.top + 21;
		size->setBounds(SmartWin::Rectangle::FromRECT(rect));

		rect.left = rect.right + lMargin;
		rect.right = width - rMargin;
		rect.bottom = rect.top + comboH;
		sizeMode->setBounds(SmartWin::Rectangle::FromRECT(rect));

		// "File type"
		rect.left = lMargin;
		rect.right = width - rMargin;
		rect.top += spacing;
		rect.bottom = rect.top + comboH + 21;
		fileType->setBounds(SmartWin::Rectangle::FromRECT(rect));
		rect.bottom -= comboH;

		typeLabel->setBounds(SmartWin::Rectangle(rect.left + lMargin, rect.top - labelH, width - rMargin, labelH - 1));

		// "Search options"
		rect.left = lMargin;
		rect.right = width - rMargin;
		rect.top += spacing;
		rect.bottom += spacing;
		slots->setBounds(SmartWin::Rectangle::FromRECT(rect));

		optionLabel->setBounds(SmartWin::Rectangle(rect.left + lMargin, rect.top - labelH, width - rMargin, labelH - 1));

		// "Hubs"
		rect.left = lMargin;
		rect.right = width - rMargin;
		rect.top += spacing;
		rect.bottom = rect.top + comboH;
		if(rect.bottom + labelH + 21 > initialRect.bottom) {
			rect.bottom = initialRect.bottom - labelH - 21;
			if(rect.bottom < rect.top + (labelH*3)/2)
				rect.bottom = rect.top + (labelH*3)/2;
		}

		hubs->setBounds(SmartWin::Rectangle::FromRECT(rect));

		hubsLabel->setBounds(SmartWin::Rectangle(rect.left + lMargin, rect.top - labelH, width - rMargin, labelH - 1));

	} else {
		results->setBounds(SmartWin::Rectangle::FromRECT(rect));

		SmartWin::Rectangle rNULL(0, 0, 0, 0);
		searchBox->setBounds(rNULL);
		mode->setBounds(rNULL);
		purge->setBounds(rNULL);
		size->setBounds(rNULL);
		sizeMode->setBounds(rNULL);
		fileType->setBounds(rNULL);
		
		sizeLabel->setBounds(rNULL);
		typeLabel->setBounds(rNULL);
		optionLabel->setBounds(rNULL);
		hubsLabel->setBounds(rNULL);
	}
}

bool SearchFrame::preClosing() {
	SearchManager::getInstance()->removeListener(this);
	ClientManager::getInstance()->removeListener(this);

	frames.erase(this);

	return true;
}

void SearchFrame::postClosing() {
#ifdef PORT_ME
	for(int i = 0; i < results->GetItemCount(); i++) {
		delete results->getItemData(i);
	}
	results->DeleteAllItems();
	for(int i = 0; i < hubs->GetItemCount(); i++) {
		delete hubs->getItemData(i);
	}
	hubs->DeleteAllItems();

	WinUtil::saveHeaderOrder(ctrlResults, SettingsManager::SEARCHFRAME_ORDER,
		SettingsManager::SEARCHFRAME_WIDTHS, COLUMN_LAST, columnIndexes, columnSizes);
#endif
}

void SearchFrame::SearchInfo::view() {
	try {
		if(sr->getType() == SearchResult::TYPE_FILE) {
			QueueManager::getInstance()->add(Util::getTempPath() + sr->getFileName(),
				sr->getSize(), sr->getTTH(), sr->getUser(), 
				QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_TEXT);
		}
	} catch(const Exception&) {
	}
}

void SearchFrame::SearchInfo::Download::operator()(SearchInfo* si) {
	try {
		if(si->sr->getType() == SearchResult::TYPE_FILE) {
			string target = Text::fromT(tgt + si->columns[COLUMN_FILENAME]);
			QueueManager::getInstance()->add(target, si->sr->getSize(),
				si->sr->getTTH(), si->sr->getUser());

			if(WinUtil::isShift())
				QueueManager::getInstance()->setPriority(target, QueueItem::HIGHEST);
		} else {
			QueueManager::getInstance()->addDirectory(si->sr->getFile(), si->sr->getUser(), Text::fromT(tgt),
				WinUtil::isShift() ? QueueItem::HIGHEST : QueueItem::DEFAULT);
		}
	} catch(const Exception&) {
	}
}

void SearchFrame::SearchInfo::DownloadWhole::operator()(SearchInfo* si) {
	try {
		QueueItem::Priority prio = WinUtil::isShift() ? QueueItem::HIGHEST : QueueItem::DEFAULT;
		if(si->sr->getType() == SearchResult::TYPE_FILE) {
			QueueManager::getInstance()->addDirectory(Text::fromT(si->columns[COLUMN_PATH]), 
				si->sr->getUser(), Text::fromT(tgt), prio);
		} else {
			QueueManager::getInstance()->addDirectory(si->sr->getFile(), si->sr->getUser(), 
				Text::fromT(tgt), prio);
		}
	} catch(const Exception&) {
	}
}

void SearchFrame::SearchInfo::DownloadTarget::operator()(SearchInfo* si) {
	try {
		if(si->sr->getType() == SearchResult::TYPE_FILE) {
			string target = Text::fromT(tgt);
			QueueManager::getInstance()->add(target, si->sr->getSize(),
				si->sr->getTTH(), si->sr->getUser());

			if(WinUtil::isShift())
				QueueManager::getInstance()->setPriority(target, QueueItem::HIGHEST);
		} else {
			QueueManager::getInstance()->addDirectory(si->sr->getFile(), si->sr->getUser(), Text::fromT(tgt),
				WinUtil::isShift() ? QueueItem::HIGHEST : QueueItem::DEFAULT);
		}
	} catch(const Exception&) {
	}
}

void SearchFrame::SearchInfo::getList() {
	try {
		QueueManager::getInstance()->addList(sr->getUser(), QueueItem::FLAG_CLIENT_VIEW, Text::fromT(columns[COLUMN_PATH]));
	} catch(const Exception&) {
		// Ignore for now...
	}
}

void SearchFrame::SearchInfo::browseList() {
	try {
		QueueManager::getInstance()->addPfs(sr->getUser(), Text::fromT(columns[COLUMN_PATH]));
	} catch(const Exception&) {
		// Ignore for now...
	}
}

void SearchFrame::SearchInfo::CheckTTH::operator()(SearchInfo* si) {
	if(firstTTH) {
		tth = si->columns[COLUMN_TTH];
		hasTTH = true;
		firstTTH = false;
	} else if(hasTTH) {
		if(tth != si->columns[COLUMN_TTH]) {
			hasTTH = false;
		}
	}

	if(firstHubs && hubs.empty()) {
		hubs = ClientManager::getInstance()->getHubs(si->sr->getUser()->getCID());
		firstHubs = false;
	} else if(!hubs.empty()) {
		Util::intersect(hubs, ClientManager::getInstance()->getHubs(si->sr->getUser()->getCID()));
	}
}

void SearchFrame::SearchInfo::update() {
	if(sr->getType() == SearchResult::TYPE_FILE) {
		if(sr->getFile().rfind(_T('\\')) == tstring::npos) {
			columns[COLUMN_FILENAME] = Text::toT(sr->getFile());
		} else {
			columns[COLUMN_FILENAME] = Text::toT(Util::getFileName(sr->getFile()));
			columns[COLUMN_PATH] = Text::toT(Util::getFilePath(sr->getFile()));
		}

		columns[COLUMN_TYPE] = Text::toT(Util::getFileExt(Text::fromT(columns[COLUMN_FILENAME])));
		if(!columns[COLUMN_TYPE].empty() && columns[COLUMN_TYPE][0] == _T('.'))
			columns[COLUMN_TYPE].erase(0, 1);
		columns[COLUMN_SIZE] = Text::toT(Util::formatBytes(sr->getSize()));
		columns[COLUMN_EXACT_SIZE] = Text::toT(Util::formatExactSize(sr->getSize()));
	} else {
		columns[COLUMN_FILENAME] = Text::toT(sr->getFileName());
		columns[COLUMN_PATH] = Text::toT(sr->getFile());
		columns[COLUMN_TYPE] = TSTRING(DIRECTORY);
		if(sr->getSize() > 0) {
			columns[COLUMN_SIZE] = Text::toT(Util::formatBytes(sr->getSize()));
			columns[COLUMN_EXACT_SIZE] = Text::toT(Util::formatExactSize(sr->getSize()));
		}
	}
	columns[COLUMN_NICK] = WinUtil::getNicks(sr->getUser());
	columns[COLUMN_CONNECTION] = Text::toT(ClientManager::getInstance()->getConnection(sr->getUser()->getCID()));
	columns[COLUMN_HUB] = Text::toT(sr->getHubName());
	columns[COLUMN_SLOTS] = Text::toT(sr->getSlotString());
	columns[COLUMN_IP] = Text::toT(sr->getIP());
	if (!columns[COLUMN_IP].empty()) {
		// Only attempt to grab a country mapping if we actually have an IP address
		tstring tmpCountry = Text::toT(Util::getIpCountry(sr->getIP()));
		if(!tmpCountry.empty())
			columns[COLUMN_IP] = tmpCountry + _T(" (") + columns[COLUMN_IP] + _T(")");
	}
	if(sr->getType() == SearchResult::TYPE_FILE) {
        columns[COLUMN_TTH] = Text::toT(sr->getTTH().toBase32());
	}
	columns[COLUMN_CID] = Text::toT(sr->getUser()->getCID().toBase32());

}

HRESULT SearchFrame::handleSpeaker(WPARAM wParam, LPARAM lParam) {
 	switch(wParam) {
	case SPEAK_ADD_RESULT:
		{
			SearchInfo* si = reinterpret_cast<SearchInfo*>(lParam);
			SearchResult* sr = si->sr;
			// Check previous search results for dupes
			for(int i = 0, j = results->getRowCount(); i < j; ++i) {
				SearchInfo* si2 = results->getItemData(i);
				SearchResult* sr2 = si2->sr;
				if((sr->getUser()->getCID() == sr2->getUser()->getCID()) && (sr->getFile() == sr2->getFile())) {
					delete si;
					return 0;
				}
			}

			results->insertItem(si);
			setStatus(STATUS_COUNT, Text::toT(Util::toString(results->getRowCount()) + ' ' + STRING(ITEMS)));
#ifdef PORT_ME
			if(BOOLSETTING(BOLD_SEARCH))
				setDirty();
#endif
		}
		break;
	case SPEAK_FILTER_RESULT:
		setStatus(STATUS_FILTERED, Text::toT(Util::toString(droppedResults) + ' ' + STRING(FILTERED)));
		break;
	case SPEAK_HUB_ADDED:
		onHubAdded(reinterpret_cast<HubInfo*>(lParam));
		break;
	case SPEAK_HUB_CHANGED:
		onHubChanged(reinterpret_cast<HubInfo*>(lParam));
		break;
	case SPEAK_HUB_REMOVED:
 		onHubRemoved(reinterpret_cast<HubInfo*>(lParam));
		break;
 	}
	return 0;
}

void SearchFrame::handlePurgeClicked() {
	searchBox->removeAllItems();
	lastSearches.clear();
}

void SearchFrame::handleSlotsClicked() {
	onlyFree = slots->getChecked();
}

void SearchFrame::handleShowUIClicked() {
	bShowUI = showUI->getChecked();
	layout();
}

HRESULT SearchFrame::handleHubItemChanged(WPARAM wParam, LPARAM lParam) {
	LPNMLISTVIEW lv = (LPNMLISTVIEW)lParam;
	if(lv->iItem == 0 && (lv->uNewState ^ lv->uOldState) & LVIS_STATEIMAGEMASK) {
		if (((lv->uNewState & LVIS_STATEIMAGEMASK) >> 12) - 1) {
			for(int iItem = 0; (iItem = hubs->getNextItem(iItem, LVNI_ALL)) != -1; ) {
				HubInfo* client = hubs->getItemData(iItem);
				if (!client->op)
					hubs->setRowChecked(iItem, false);
			}
		}
	}
	return 0;
}

HRESULT SearchFrame::handleDoubleClick(WPARAM wParam, LPARAM lParam) {
	if(((LPNMITEMACTIVATE)lParam)->iItem != -1)
		results->forEachSelectedT(SearchInfo::Download(Text::toT(SETTING(DOWNLOAD_DIRECTORY))));
	return 0;
}

HRESULT SearchFrame::handleKeyDown(WPARAM wParam, LPARAM lParam) {
	if(((LPNMLVKEYDOWN)lParam)->wVKey == VK_DELETE)
		StupidWin::postMessage(this, WM_COMMAND, IDC_REMOVE);
	return 0;
}

HRESULT SearchFrame::handleContextMenu(WPARAM wParam, LPARAM lParam) {
	if(results->getSelectedCount() > 0) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if(pt.x == -1 && pt.y == -1) {
			pt = results->getContextMenuPos();
		}

		WidgetMenuPtr contextMenu = makeMenu();
		contextMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return TRUE;
	}
	return FALSE;
}

void SearchFrame::handleDownload() {
	results->forEachSelectedT(SearchInfo::Download(Text::toT(SETTING(DOWNLOAD_DIRECTORY))));
}

void SearchFrame::handleDownloadFavoriteDirs(unsigned id) {
	dcassert(id >= IDC_DOWNLOAD_FAVORITE_DIRS);
	size_t newId = (size_t)id - IDC_DOWNLOAD_FAVORITE_DIRS;

	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if(newId < spl.size()) {
		results->forEachSelectedT(SearchInfo::Download(Text::toT(spl[newId].first)));
	} else {
		dcassert((newId - spl.size()) < targets.size());
		results->forEachSelectedT(SearchInfo::DownloadTarget(Text::toT(targets[newId - spl.size()])));
	}
}

void SearchFrame::handleDownloadTo() {
	if(results->getSelectedCount() == 1) {
		int i = results->getNextItem(-1, LVNI_SELECTED);
		dcassert(i != -1);
		SearchInfo* si = results->getItemData(i);
		SearchResult* sr = si->sr;

		if(sr->getType() == SearchResult::TYPE_FILE) {
			tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY)) + si->columns[COLUMN_FILENAME];
			if(WinUtil::browseFile(target, handle())) {
				WinUtil::addLastDir(Util::getFilePath(target));
				results->forEachSelectedT(SearchInfo::DownloadTarget(target));
			}
		} else {
			tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
			if(WinUtil::browseDirectory(target, handle())) {
				WinUtil::addLastDir(target);
				results->forEachSelectedT(SearchInfo::Download(target));
			}
		}
	} else {
		tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
		if(WinUtil::browseDirectory(target, handle())) {
			WinUtil::addLastDir(target);
			results->forEachSelectedT(SearchInfo::Download(target));
		}
	}
}

void SearchFrame::handleDownloadTarget(unsigned id) {
	dcassert(id >= IDC_DOWNLOAD_TARGET);
	size_t newId = (size_t)id - IDC_DOWNLOAD_TARGET;

	if(newId < WinUtil::lastDirs.size()) {
		results->forEachSelectedT(SearchInfo::Download(WinUtil::lastDirs[newId]));
	} else {
		dcassert((newId - WinUtil::lastDirs.size()) < targets.size());
		results->forEachSelectedT(SearchInfo::DownloadTarget(Text::toT(targets[newId - WinUtil::lastDirs.size()])));
	}
}

void SearchFrame::handleDownloadDir() {
	results->forEachSelectedT(SearchInfo::DownloadWhole(Text::toT(SETTING(DOWNLOAD_DIRECTORY))));
}

void SearchFrame::handleDownloadWholeFavoriteDirs(unsigned id) {
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	dcassert((id-IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS) < spl.size());
	results->forEachSelectedT(SearchInfo::DownloadWhole(Text::toT(spl[id-IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS].first)));
}

void SearchFrame::handleDownloadWholeTarget(unsigned id) {
	dcassert((id-IDC_DOWNLOAD_WHOLE_TARGET) < WinUtil::lastDirs.size());
	results->forEachSelectedT(SearchInfo::DownloadWhole(WinUtil::lastDirs[id-IDC_DOWNLOAD_WHOLE_TARGET]));
}

void SearchFrame::handleDownloadDirTo() {
	tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
	if(WinUtil::browseDirectory(target, handle())) {
		WinUtil::addLastDir(target);
		results->forEachSelectedT(SearchInfo::DownloadWhole(target));
	}
}

void SearchFrame::handleViewAsText() {
	results->forEachSelected(&SearchInfo::view);
}

void SearchFrame::handleSearchAlternates() {
	if(results->getSelectedCount() == 1) {
		int i = results->getNextItem(-1, LVNI_SELECTED);
		SearchResult* sr = results->getItemData(i)->sr;
		if(sr->getType() == SearchResult::TYPE_FILE) {
			WinUtil::searchHash(sr->getTTH());
		}
	}
}

void SearchFrame::handleBitziLookup() {
	if(results->getSelectedCount() == 1) {
		int i = results->getNextItem(-1, LVNI_SELECTED);
		SearchResult* sr = results->getItemData(i)->sr;
		if(sr->getType() == SearchResult::TYPE_FILE) {
			WinUtil::bitziLink(sr->getTTH());
		}
	}
}

void SearchFrame::handleCopyMagnet() {
	if(results->getSelectedCount() == 1) {
		int i = results->getNextItem(-1, LVNI_SELECTED);
		SearchResult* sr = results->getItemData(i)->sr;
		if(sr->getType() == SearchResult::TYPE_FILE) {
			WinUtil::copyMagnet(sr->getTTH(), Text::toT(sr->getFileName()));
		}
	}
}

void SearchFrame::handleRemove() {
	int i = -1;
	while((i = results->getNextItem(-1, LVNI_SELECTED)) != -1) {
		delete results->getItemData(i);
		results->removeRow(i);
	}
}

SearchFrame::WidgetMenuPtr SearchFrame::makeMenu() {
	WidgetMenuPtr menu = createMenu(true);

	StringPairList favoriteDirs = FavoriteManager::getInstance()->getFavoriteDirs();
	SearchInfo::CheckTTH checkTTH = results->forEachSelectedT(SearchInfo::CheckTTH());

	menu->appendItem(IDC_DOWNLOAD, TSTRING(DOWNLOAD), std::tr1::bind(&SearchFrame::handleDownload, this));
	addTargetMenu(menu, favoriteDirs, checkTTH);
	menu->appendItem(IDC_DOWNLOADDIR, TSTRING(DOWNLOAD_WHOLE_DIR), std::tr1::bind(&SearchFrame::handleDownloadDir, this));
	addTargetDirMenu(menu, favoriteDirs);
	menu->appendItem(IDC_VIEW_AS_TEXT, TSTRING(VIEW_AS_TEXT), std::tr1::bind(&SearchFrame::handleViewAsText, this));
	menu->appendSeparatorItem();
	menu->appendItem(IDC_SEARCH_ALTERNATES, TSTRING(SEARCH_FOR_ALTERNATES), std::tr1::bind(&SearchFrame::handleSearchAlternates, this));
	menu->appendItem(IDC_BITZI_LOOKUP, TSTRING(LOOKUP_AT_BITZI), std::tr1::bind(&SearchFrame::handleBitziLookup, this));
	menu->appendItem(IDC_COPY_MAGNET, TSTRING(COPY_MAGNET), std::tr1::bind(&SearchFrame::handleCopyMagnet, this));
	menu->appendSeparatorItem();
	appendUserItems(menu);
	menu->appendSeparatorItem();
	menu->appendItem(IDC_REMOVE, TSTRING(REMOVE), std::tr1::bind(&SearchFrame::handleRemove, this));
	prepareMenu(menu, UserCommand::CONTEXT_SEARCH, checkTTH.hubs);

	menu->setDefaultItem(IDC_DOWNLOAD);

	return menu;
}

void SearchFrame::addTargetMenu(const WidgetMenuPtr& parent, const StringPairList& favoriteDirs, const SearchInfo::CheckTTH& checkTTH) {
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(DOWNLOAD_TO));

	int n = 0;
	if(favoriteDirs.size() > 0) {
		for(StringPairList::const_iterator i = favoriteDirs.begin(); i != favoriteDirs.end(); i++)
			menu->appendItem(IDC_DOWNLOAD_FAVORITE_DIRS + n++, Text::toT(i->second), std::tr1::bind(&SearchFrame::handleDownloadFavoriteDirs, this, _1));
		menu->appendSeparatorItem();
	}

	n = 0;
	menu->appendItem(IDC_DOWNLOADTO, TSTRING(BROWSE), std::tr1::bind(&SearchFrame::handleDownloadTo, this));
	if(WinUtil::lastDirs.size() > 0) {
		menu->appendSeparatorItem();
		for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i)
			menu->appendItem(IDC_DOWNLOAD_TARGET + n++, *i, std::tr1::bind(&SearchFrame::handleDownloadTarget, this, _1));
	}

	if(checkTTH.hasTTH) {
		targets.clear();

		QueueManager::getInstance()->getTargets(TTHValue(Text::fromT(checkTTH.tth)), targets);
		if(targets.size() > 0) {
			menu->appendSeparatorItem();
			for(StringIter i = targets.begin(); i != targets.end(); ++i)
				menu->appendItem(IDC_DOWNLOAD_TARGET + n++, Text::toT(*i), std::tr1::bind(&SearchFrame::handleDownloadTarget, this, _1));
		}
	}
}

void SearchFrame::addTargetDirMenu(const WidgetMenuPtr& parent, const StringPairList& favoriteDirs) {
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(DOWNLOAD_WHOLE_DIR_TO));

	int n = 0;
	if(favoriteDirs.size() > 0) {
		for(StringPairList::const_iterator i = favoriteDirs.begin(); i != favoriteDirs.end(); ++i)
			menu->appendItem(IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + n++, Text::toT(i->second), std::tr1::bind(&SearchFrame::handleDownloadWholeFavoriteDirs, this, _1));
		menu->appendSeparatorItem();
	}

	n = 0;
	menu->appendItem(IDC_DOWNLOADDIRTO, TSTRING(BROWSE), std::tr1::bind(&SearchFrame::handleDownloadDirTo, this));
	if(WinUtil::lastDirs.size() > 0) {
		menu->appendSeparatorItem();
		for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i)
			menu->appendItem(IDC_DOWNLOAD_WHOLE_TARGET + n++, *i, std::tr1::bind(&SearchFrame::handleDownloadWholeTarget, this, _1));
	}
}

void SearchFrame::on(SearchManagerListener::SR, SearchResult* aResult) throw() {
	// Check that this is really a relevant search result...
	{
		Lock l(cs);

		if(currentSearch.empty()) {
			return;
		}

		if(isHash) {
			if(aResult->getType() != SearchResult::TYPE_FILE || TTHValue(Text::fromT(currentSearch[0])) != aResult->getTTH()) {
				droppedResults++;
				speak(SPEAK_FILTER_RESULT);
				return;
			}
		} else {
			// match all here
			for(TStringIter j = currentSearch.begin(); j != currentSearch.end(); ++j) {
				if((*j->begin() != _T('-') && Util::findSubString(aResult->getFile(), Text::fromT(*j)) == -1) ||
					(*j->begin() == _T('-') && j->size() != 1 && Util::findSubString(aResult->getFile(), Text::fromT(j->substr(1))) != -1)
					)
				{
					droppedResults++;
					speak(SPEAK_FILTER_RESULT);
					return;
				}
			}
		}
	}

	// Reject results without free slots
	if((onlyFree && aResult->getFreeSlots() < 1))
	{
		droppedResults++;
		speak(SPEAK_FILTER_RESULT);
		return;
	}

	SearchInfo* i = new SearchInfo(aResult);
	speak(SPEAK_ADD_RESULT, reinterpret_cast<WPARAM>(i));
}

void SearchFrame::onHubAdded(HubInfo* info) {
	int nItem = hubs->insertItem(info);
	hubs->setRowChecked(nItem, (hubs->getIsRowChecked(0) ? info->op : true));
	hubs->setColumnWidth(0, LVSCW_AUTOSIZE);
}

void SearchFrame::onHubChanged(HubInfo* info) {
	int nItem = 0;
	int n = hubs->getRowCount();
	for(; nItem < n; nItem++) {
		if(hubs->getItemData(nItem)->url == info->url)
			break;
	}
	if (nItem == n)
		return;

	delete hubs->getItemData(nItem);
	hubs->setItemData(nItem, info);
	hubs->updateItem(nItem);

	if (hubs->getIsRowChecked(0))
		hubs->setRowChecked(nItem, info->op);

	hubs->setColumnWidth(0, LVSCW_AUTOSIZE);
}

void SearchFrame::onHubRemoved(HubInfo* info) {
	int nItem = 0;
	int n = hubs->getRowCount();
	for(; nItem < n; nItem++) {
		if(hubs->getItemData(nItem)->url == info->url)
			break;
	}
	if (nItem == n)
		return;

	delete hubs->getItemData(nItem);
	hubs->removeRow(nItem);
	hubs->setColumnWidth(0, LVSCW_AUTOSIZE);
}

void SearchFrame::speak(Speakers s, Client* aClient) {
	HubInfo* hubInfo = new HubInfo(Text::toT(aClient->getHubUrl()), Text::toT(aClient->getHubName()), aClient->getMyIdentity().isOp());
	speak(s, reinterpret_cast<WPARAM>(hubInfo));
}

void SearchFrame::runSearch() {
	StringList clients;

	// Change Default Settings If Changed
	if (onlyFree != BOOLSETTING(SEARCH_ONLY_FREE_SLOTS))
		SettingsManager::getInstance()->set(SettingsManager::SEARCH_ONLY_FREE_SLOTS, onlyFree);
	if (!initialType && fileType->getSelectedIndex() != SETTING(LAST_SEARCH_TYPE))
		SettingsManager::getInstance()->set(SettingsManager::LAST_SEARCH_TYPE, fileType->getSelectedIndex());

	tstring s = searchBox->getText();
	
	if(s.empty())
		return;

	int n = hubs->getRowCount();
	for(int i = 0; i < n; i++) {
		if(hubs->getIsRowChecked(i)) {
			clients.push_back(Text::fromT(hubs->getItemData(i)->url));
		}
	}

	if(clients.empty())
		return;

	tstring tsize = size->getText();

	double lsize = Util::toDouble(Text::fromT(tsize));
	switch(sizeMode->getSelectedIndex()) {
	case 1:
		lsize*=1024.0; break;
	case 2:
		lsize*=1024.0*1024.0; break;
	case 3:
		lsize*=1024.0*1024.0*1024.0; break;
	}

	int64_t llsize = (int64_t)lsize;

	results->forEachT(DeleteFunction());
	results->removeAllRows();

	{
		Lock l(cs);
		currentSearch = StringTokenizer<tstring>(s, ' ').getTokens();
		s.clear();
		//strip out terms beginning with -
		for(TStringList::iterator si = currentSearch.begin(); si != currentSearch.end(); ) {
			if(si->empty()) {
				si = currentSearch.erase(si);
				continue;
			}
			if ((*si)[0] != _T('-')) 
				s += *si + _T(' ');	
			++si;
		}

		s = s.substr(0, max(s.size(), static_cast<tstring::size_type>(1)) - 1);
	}


	SearchManager::SizeModes mode((SearchManager::SizeModes)sizeMode->getSelectedIndex());
	if(llsize == 0)
		mode = SearchManager::SIZE_DONTCARE;

	int ftype = fileType->getSelectedIndex();

	// Add new searches to the last-search dropdown list
	if(find(lastSearches.begin(), lastSearches.end(), s) == lastSearches.end())
	{
		int i = max(SETTING(SEARCH_HISTORY)-1, 0);

		if(searchBox->getCount() > i)
			searchBox->removeItem(i);
		searchBox->insertValue(0, s);

		while(lastSearches.size() > (TStringList::size_type)i) {
			lastSearches.erase(lastSearches.begin());
		}
		lastSearches.push_back(s);
	}

	setStatus(STATUS_STATUS, TSTRING(SEARCHING_FOR) + s + _T("..."));
	setStatus(STATUS_COUNT, Util::emptyStringT);
	setStatus(STATUS_FILTERED, Util::emptyStringT);
	droppedResults = 0;
	isHash = (ftype == SearchManager::TYPE_TTH);

	setText(TSTRING(SEARCH) + _T(" - ") + s);

	if(SearchManager::getInstance()->okToSearch()) {
		SearchManager::getInstance()->search(clients, Text::fromT(s), llsize,
			(SearchManager::TypeModes)ftype, mode, "manual");
		if(BOOLSETTING(CLEAR_SEARCH)) // Only clear if the search was sent
			searchBox->setText(Util::emptyStringT);
	} else {
		int32_t waitFor = SearchManager::getInstance()->timeToSearch();
		AutoArray<TCHAR> buf(TSTRING(SEARCHING_WAIT).size() + 16);
		_stprintf(buf, CTSTRING(SEARCHING_WAIT), waitFor);

		setStatus(STATUS_STATUS, tstring(buf));
		setStatus(STATUS_COUNT, Util::emptyStringT);
		setStatus(STATUS_FILTERED, Util::emptyStringT);

		setText(TSTRING(SEARCH) + _T(" - ") + tstring(buf));
		// Start the countdown timer
		initSecond();
	}
}

void SearchFrame::initSecond() {
	createTimer(std::tr1::bind(&SearchFrame::eachSecond, this), 1000);
}

bool SearchFrame::eachSecond() {
	int32_t waitFor = SearchManager::getInstance()->timeToSearch();
	if(waitFor > 0) {
		AutoArray<TCHAR> buf(TSTRING(SEARCHING_WAIT).size() + 16);
		_stprintf(buf, CTSTRING(SEARCHING_WAIT), waitFor);

		setStatus(STATUS_STATUS, tstring(buf));
		setText(TSTRING(SEARCH) + _T(" - ") + tstring(buf));
		return true;
	} 
	
	setStatus(STATUS_STATUS, TSTRING(SEARCHING_READY));
	setText(TSTRING(SEARCH) + _T(" - ") + TSTRING(SEARCHING_READY));
	
	return false;
}

void SearchFrame::runUserCommand(const UserCommand& uc) {
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	set<CID> users;

	int sel = -1;
	while((sel = results->getNextItem(sel, LVNI_SELECTED)) != -1) {
		SearchResult* sr = results->getItemData(sel)->sr;

		if(!sr->getUser()->isOnline())
			continue;

		if(uc.getType() == UserCommand::TYPE_RAW_ONCE) {
			if(users.find(sr->getUser()->getCID()) != users.end())
				continue;
			users.insert(sr->getUser()->getCID());
		}

		ucParams["fileFN"] = sr->getFile();
		ucParams["fileSI"] = Util::toString(sr->getSize());
		ucParams["fileSIshort"] = Util::formatBytes(sr->getSize());
		if(sr->getType() == SearchResult::TYPE_FILE) {
			ucParams["fileTR"] = sr->getTTH().toBase32();
		}

		// compatibility with 0.674 and earlier
		ucParams["file"] = ucParams["fileFN"];
		ucParams["filesize"] = ucParams["fileSI"];
		ucParams["filesizeshort"] = ucParams["fileSIshort"];
		ucParams["tth"] = ucParams["fileTR"];

		StringMap tmp = ucParams;
		ClientManager::getInstance()->userCommand(sr->getUser(), uc, tmp, true);
	}
}

#ifdef PORT_ME

LRESULT SearchFrame::onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	switch(wParam) {
	case VK_TAB:
		if(uMsg == WM_KEYDOWN) {
			onTab(WinUtil::isShift());
		}
		break;
	case VK_RETURN:
		if( WinUtil::isShift() || WinUtil::isCtrl() || WinUtil::isAlt() ) {
			bHandled = FALSE;
		} else {
			if(uMsg == WM_KEYDOWN) {
				onEnter();
			}
		}
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT SearchFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	results->forEachSelected(&SearchInfo::getList);
	return 0;
}

LRESULT SearchFrame::onBrowseList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	results->forEachSelected(&SearchInfo::browseList);
	return 0;
}

#endif
