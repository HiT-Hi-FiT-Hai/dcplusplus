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
#include <client/DCPlusPlus.h>
#include "resource.h"

#include "SearchFrame.h"

#include <client/ResourceManager.h>
#include <client/FavoriteManager.h>
#include <client/QueueManager.h>

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
#ifdef PORT_ME
		rect.bottom = rect.top + comboH + 21;
#else
		rect.bottom = rect.top + 21; //@todo add + comboH when searchBox is a ComboBox
#endif
		searchBox->setBounds(SmartWin::Rectangle::FromRECT(rect));

		searchLabel->setBounds(SmartWin::Rectangle(rect.left + lMargin, rect.top - labelH, width - rMargin, labelH - 1));

		// "Purge"
		rect.right = rect.left + spacing;
		rect.left = lMargin;
		rect.top += 25;
		rect.bottom = rect.top + 21;
		purge->setBounds(SmartWin::Rectangle::FromRECT(rect));

		// "Size"
		int w2 = width - rMargin - lMargin;
		rect.top += spacing;
		rect.bottom = rect.top + comboH;
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

		// "Search"
		rect.right = width - rMargin;
		rect.left = rect.right - 100;
		rect.top = rect.bottom + labelH;
		rect.bottom = rect.top + 21;
		doSearch->setBounds(SmartWin::Rectangle::FromRECT(rect));
	} else {
		results->setBounds(SmartWin::Rectangle::FromRECT(rect));

		SmartWin::Rectangle rNULL(0, 0, 0, 0);
		searchBox->setBounds(rNULL);
		mode->setBounds(rNULL);
		purge->setBounds(rNULL);
		size->setBounds(rNULL);
		sizeMode->setBounds(rNULL);
		fileType->setBounds(rNULL);
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

SearchFrame::SearchFrame(SmartWin::Widget* mdiParent, const tstring& initialString_, LONGLONG initialSize_, SearchManager::SizeModes initialMode_, SearchManager::TypeModes initialType_) :
	SmartWin::Widget(mdiParent),
	bShowUI(true),
	isHash(false),
	initialString(initialString_),
	initialSize(initialSize_),
	initialMode(initialMode_),
	initialType(initialType_),
	onlyFree(BOOLSETTING(SEARCH_ONLY_FREE_SLOTS)),
#ifdef PORT_ME
	timerID(0),
#endif
	droppedResults(0)
{
	searchLabel = createStatic();
	searchLabel->setText(TSTRING(SEARCH_FOR));

	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
		cs.exStyle = WS_EX_CLIENTEDGE;
		searchBox = createTextBox(cs);
		searchBox->setFont(WinUtil::font);
		add_widget(searchBox);

#ifdef PORT_ME
		ctrlSearchBox.Create(handle(), rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
			WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL, 0);
		for(TStringIter i = lastSearches.begin(); i != lastSearches.end(); ++i) {
			ctrlSearchBox.InsertString(0, i->c_str());
		}
		ctrlSearchBox.SetExtendedUI();
#endif
	}

	{
		WidgetButton::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
		cs.caption = TSTRING(PURGE);
		purge = createButton(cs);

		purge->onClicked(&SearchFrame::handlePurgeClicked);
	}

	sizeLabel = createStatic();
	sizeLabel->setText(TSTRING(SIZE));

	{
		WidgetComboBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST;
		cs.exStyle = WS_EX_CLIENTEDGE;
		mode = createComboBox(cs);
		mode->setFont(WinUtil::font);
		add_widget(mode);

		mode->addValue(TSTRING(NORMAL));
		mode->addValue(TSTRING(AT_LEAST));
		mode->addValue(TSTRING(AT_MOST));
	}

	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_NUMBER;
		cs.exStyle = WS_EX_CLIENTEDGE;
		size = createTextBox(cs);
		size->setFont(WinUtil::font);
		add_widget(size);
	}

	{
		WidgetComboBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST;
		cs.exStyle = WS_EX_CLIENTEDGE;
		sizeMode = createComboBox(cs);
		sizeMode->setFont(WinUtil::font);
		add_widget(sizeMode);

		sizeMode->addValue(TSTRING(B));
		sizeMode->addValue(TSTRING(KiB));
		sizeMode->addValue(TSTRING(MiB));
		sizeMode->addValue(TSTRING(GiB));
		sizeMode->setSelectedIndex((initialSize == 0) ? 2 : 0);
	}

	typeLabel = createStatic();
	typeLabel->setText(TSTRING(FILE_TYPE));

	{
		WidgetComboBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST;
		cs.exStyle = WS_EX_CLIENTEDGE;
		fileType = createComboBox(cs);
		fileType->setFont(WinUtil::font);
		add_widget(fileType);

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

	optionLabel = createStatic();
	optionLabel->setText(TSTRING(SEARCH_OPTIONS));

	{
		WidgetCheckBox::Seed cs;
		cs.caption = TSTRING(ONLY_FREE_SLOTS);
		slots = createCheckBox(cs);
		slots->setChecked(onlyFree);

		slots->onClicked(&SearchFrame::handleSlotsClicked);
	}

	hubsLabel = createStatic();
	hubsLabel->setText(TSTRING(HUBS));

	{
		WidgetDataGrid::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_NOCOLUMNHEADER;
		cs.exStyle = WS_EX_CLIENTEDGE;
		hubs = SmartWin::WidgetCreator<WidgetHubs>::create(this, cs);
		hubs->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
		add_widget(hubs);

		TStringList dummy;
		dummy.push_back(Util::emptyStringT);
		hubs->createColumns(dummy);

		hubs->setColor(WinUtil::textColor, WinUtil::bgColor);

		hubs->onRaw(&SearchFrame::handleHubItemChanged, SmartWin::Message(WM_NOTIFY, LVN_ITEMCHANGED));
	}

	{
		WidgetButton::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
		cs.caption = TSTRING(SEARCH);
		doSearch = createButton(cs);

		doSearch->onClicked(&SearchFrame::handleDoSearchClicked);
	}

	{
		WidgetDataGrid::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS;
		cs.exStyle = WS_EX_CLIENTEDGE;
		results = SmartWin::WidgetCreator<WidgetResults>::create(this, cs);
		results->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
		add_widget(results);

#ifdef PORT_ME
		for(int j=0; j<COLUMN_LAST; j++) {
			int fmt = (j == COLUMN_SIZE || j == COLUMN_EXACT_SIZE) ? LVCFMT_RIGHT : LVCFMT_LEFT;
			results->InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
		}
#endif
		results->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		results->setColumnOrder(WinUtil::splitTokens(SETTING(SEARCHFRAME_ORDER), columnIndexes));
		results->setColumnWidths(WinUtil::splitTokens(SETTING(SEARCHFRAME_WIDTHS), columnSizes));

		results->setColor(WinUtil::textColor, WinUtil::bgColor);
		results->setSmallImageList(WinUtil::fileImages);

		results->onRaw(&SearchFrame::handleDoubleClick, SmartWin::Message(WM_NOTIFY, NM_DBLCLK));
		results->onRaw(&SearchFrame::handleKeyDown, SmartWin::Message(WM_NOTIFY, LVN_KEYDOWN));
		results->onRaw(&SearchFrame::handleContextMenu, SmartWin::Message(WM_CONTEXTMENU));
	}

	{
		WidgetCheckBox::Seed cs;
		cs.caption = _T("+/-");
		showUI = createCheckBox(cs);
		showUI->setChecked(bShowUI);

		showUI->onClicked(&SearchFrame::handleShowUIClicked);
	}

	initStatus();
	
	statusSizes[STATUS_SHOW_UI] = 16; ///@todo get real checkbox width
	statusSizes[STATUS_DUMMY] = 16; ///@todo get real resizer width

	layout();

	onSpeaker(&SearchFrame::spoken);

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
#ifdef PORT_ME
		ctrlSearchBox.InsertString(0, initialString.c_str());
		ctrlSearchBox.SetCurSel(0);
#endif
		mode->setSelectedIndex(initialMode);
		size->setText(Text::toT(Util::toString(initialSize)));
		fileType->setSelectedIndex(initialType);
#ifdef PORT_ME
		onEnter();
#endif
	} else {
		setText(TSTRING(SEARCH));
		mode->setSelectedIndex(1);
		fileType->setSelectedIndex(SETTING(LAST_SEARCH_TYPE));
	}
}

SearchFrame::~SearchFrame() {
}

HRESULT SearchFrame::spoken(LPARAM lParam, WPARAM wParam) {
 	switch(wParam) {
	case SPEAK_ADD_RESULT:
		{
			SearchInfo* si = (SearchInfo*)lParam;
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
		onHubAdded((HubInfo*)(lParam));
		break;
	case SPEAK_HUB_CHANGED:
		onHubChanged((HubInfo*)(lParam));
		break;
	case SPEAK_HUB_REMOVED:
 		onHubRemoved((HubInfo*)(lParam));
		break;
 	}
	return 0;
}

void SearchFrame::handlePurgeClicked(WidgetButtonPtr) {
#ifdef PORT_ME
	searchBox->ResetContent();
#endif
	lastSearches.clear();
}

void SearchFrame::handleSlotsClicked(WidgetCheckBoxPtr) {
	onlyFree = slots->getChecked();
}

void SearchFrame::handleDoSearchClicked(WidgetButtonPtr) {
#ifdef PORT_ME
	onEnter();
#endif
}

void SearchFrame::handleShowUIClicked(WidgetCheckBoxPtr) {
	bShowUI = showUI->getChecked();
	layout();
}

HRESULT SearchFrame::handleHubItemChanged(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/) {
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

HRESULT SearchFrame::handleDoubleClick(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/) {
	if(((LPNMITEMACTIVATE)lParam)->iItem != -1)
		results->forEachSelectedT(SearchInfo::Download(Text::toT(SETTING(DOWNLOAD_DIRECTORY))));
	return 0;
}

HRESULT SearchFrame::handleKeyDown(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/) {
	if(((LPNMLVKEYDOWN)lParam)->wVKey == VK_DELETE)
		StupidWin::postMessage(this, WM_COMMAND, IDC_REMOVE);
	return 0;
}

HRESULT SearchFrame::handleContextMenu(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/) {
	if(results->getSelectedCount() > 0) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if(pt.x == -1 && pt.y == -1) {
			pt = results->getContextMenuPos();
		}

		contextMenu = makeMenu();
		contextMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return TRUE;
	}
	return FALSE;
}

void SearchFrame::handleDownload(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
	results->forEachSelectedT(SearchInfo::Download(Text::toT(SETTING(DOWNLOAD_DIRECTORY))));
}

void SearchFrame::handleDownloadFavoriteDirs(WidgetMenuPtr /*menu*/, unsigned id) {
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

void SearchFrame::handleDownloadTo(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
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

void SearchFrame::handleDownloadTarget(WidgetMenuPtr /*menu*/, unsigned id) {
	dcassert(id >= IDC_DOWNLOAD_TARGET);
	size_t newId = (size_t)id - IDC_DOWNLOAD_TARGET;

	if(newId < WinUtil::lastDirs.size()) {
		results->forEachSelectedT(SearchInfo::Download(WinUtil::lastDirs[newId]));
	} else {
		dcassert((newId - WinUtil::lastDirs.size()) < targets.size());
		results->forEachSelectedT(SearchInfo::DownloadTarget(Text::toT(targets[newId - WinUtil::lastDirs.size()])));
	}
}

void SearchFrame::handleDownloadDir(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
	results->forEachSelectedT(SearchInfo::DownloadWhole(Text::toT(SETTING(DOWNLOAD_DIRECTORY))));
}

void SearchFrame::handleDownloadWholeFavoriteDirs(WidgetMenuPtr /*menu*/, unsigned id) {
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	dcassert((id-IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS) < (int)spl.size());
	results->forEachSelectedT(SearchInfo::DownloadWhole(Text::toT(spl[id-IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS].first)));
}

void SearchFrame::handleDownloadWholeTarget(WidgetMenuPtr /*menu*/, unsigned id) {
	dcassert((id-IDC_DOWNLOAD_WHOLE_TARGET) < (int)WinUtil::lastDirs.size());
	results->forEachSelectedT(SearchInfo::DownloadWhole(WinUtil::lastDirs[id-IDC_DOWNLOAD_WHOLE_TARGET]));
}

void SearchFrame::handleDownloadDirTo(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
	tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
	if(WinUtil::browseDirectory(target, handle())) {
		WinUtil::addLastDir(target);
		results->forEachSelectedT(SearchInfo::DownloadWhole(target));
	}
}

void SearchFrame::handleViewAsText(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
	results->forEachSelected(&SearchInfo::view);
}

void SearchFrame::handleSearchAlternates(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
	if(results->getSelectedCount() == 1) {
		int i = results->getNextItem(-1, LVNI_SELECTED);
		SearchResult* sr = results->getItemData(i)->sr;
		if(sr->getType() == SearchResult::TYPE_FILE) {
			WinUtil::searchHash(sr->getTTH());
		}
	}
}

void SearchFrame::handleBitziLookup(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
	if(results->getSelectedCount() == 1) {
		int i = results->getNextItem(-1, LVNI_SELECTED);
		SearchResult* sr = results->getItemData(i)->sr;
		if(sr->getType() == SearchResult::TYPE_FILE) {
			WinUtil::bitziLink(sr->getTTH());
		}
	}
}

void SearchFrame::handleCopyMagnet(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
	if(results->getSelectedCount() == 1) {
		int i = results->getNextItem(-1, LVNI_SELECTED);
		SearchResult* sr = results->getItemData(i)->sr;
		if(sr->getType() == SearchResult::TYPE_FILE) {
			WinUtil::copyMagnet(sr->getTTH(), Text::toT(sr->getFileName()));
		}
	}
}

void SearchFrame::handleRemove(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
	int i = -1;
	while((i = results->getNextItem(-1, LVNI_SELECTED)) != -1) {
		delete results->getItemData(i);
		results->removeRow(i);
	}
}

SearchFrame::WidgetPopupMenuPtr SearchFrame::makeMenu() {
	WidgetPopupMenuPtr menu = createPopupMenu();

	StringPairList favoriteDirs = FavoriteManager::getInstance()->getFavoriteDirs();
	SearchInfo::CheckTTH checkTTH = results->forEachSelectedT(SearchInfo::CheckTTH());

	menu->appendItem(IDC_DOWNLOAD, TSTRING(DOWNLOAD), &SearchFrame::handleDownload);
	addTargetMenu(menu, favoriteDirs, checkTTH);
	menu->appendItem(IDC_DOWNLOADDIR, TSTRING(DOWNLOAD_WHOLE_DIR), &SearchFrame::handleDownloadDir);
	addTargetDirMenu(menu, favoriteDirs);
	menu->appendItem(IDC_VIEW_AS_TEXT, TSTRING(VIEW_AS_TEXT), &SearchFrame::handleViewAsText);
	menu->appendSeparatorItem();
	menu->appendItem(IDC_SEARCH_ALTERNATES, TSTRING(SEARCH_FOR_ALTERNATES), &SearchFrame::handleSearchAlternates);
	menu->appendItem(IDC_BITZI_LOOKUP, TSTRING(LOOKUP_AT_BITZI), &SearchFrame::handleBitziLookup);
	menu->appendItem(IDC_COPY_MAGNET, TSTRING(COPY_MAGNET), &SearchFrame::handleCopyMagnet);
	menu->appendSeparatorItem();
#ifdef PORT_ME
	appendUserItems(menu);
#endif
	menu->appendSeparatorItem();
	menu->appendItem(IDC_REMOVE, TSTRING(REMOVE), &SearchFrame::handleRemove);
#ifdef PORT_ME
	menu->SetMenuDefaultItem(IDC_DOWNLOAD);

	prepareMenu(menu, UserCommand::CONTEXT_SEARCH, checkTTH.hubs);
	checkAdcItems(menu);
#endif

	return menu;
}

void SearchFrame::addTargetMenu(const WidgetPopupMenuPtr& parent, const StringPairList& favoriteDirs, const SearchInfo::CheckTTH& checkTTH) {
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(DOWNLOAD_TO));

	int n = 0;
	if(favoriteDirs.size() > 0) {
		for(StringPairList::const_iterator i = favoriteDirs.begin(); i != favoriteDirs.end(); i++)
			menu->appendItem(IDC_DOWNLOAD_FAVORITE_DIRS + n++, Text::toT(i->second), &SearchFrame::handleDownloadFavoriteDirs);
		menu->appendSeparatorItem();
	}

	n = 0;
	menu->appendItem(IDC_DOWNLOADTO, TSTRING(BROWSE), &SearchFrame::handleDownloadTo);
	if(WinUtil::lastDirs.size() > 0) {
		menu->appendSeparatorItem();
		for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i)
			menu->appendItem(IDC_DOWNLOAD_TARGET + n++, *i, &SearchFrame::handleDownloadTarget);
	}

	if(checkTTH.hasTTH) {
		targets.clear();

		QueueManager::getInstance()->getTargets(TTHValue(Text::fromT(checkTTH.tth)), targets);
		if(targets.size() > 0) {
			menu->appendSeparatorItem();
			for(StringIter i = targets.begin(); i != targets.end(); ++i)
				menu->appendItem(IDC_DOWNLOAD_TARGET + n++, Text::toT(*i), &SearchFrame::handleDownloadTarget);
		}
	}
}

void SearchFrame::addTargetDirMenu(const WidgetPopupMenuPtr& parent, const StringPairList& favoriteDirs) {
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(DOWNLOAD_WHOLE_DIR_TO));

	int n = 0;
	if(favoriteDirs.size() > 0) {
		for(StringPairList::const_iterator i = favoriteDirs.begin(); i != favoriteDirs.end(); ++i)
			menu->appendItem(IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + n++, Text::toT(i->second), &SearchFrame::handleDownloadWholeFavoriteDirs);
		menu->appendSeparatorItem();
	}

	n = 0;
	menu->appendItem(IDC_DOWNLOADDIRTO, TSTRING(BROWSE), &SearchFrame::handleDownloadDirTo);
	if(WinUtil::lastDirs.size() > 0) {
		menu->appendSeparatorItem();
		for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i)
			menu->appendItem(IDC_DOWNLOAD_WHOLE_TARGET + n++, *i, &SearchFrame::handleDownloadWholeTarget);
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
				StupidWin::postMessage(this, WM_SPEAKER, SPEAK_FILTER_RESULT);
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
					StupidWin::postMessage(this, WM_SPEAKER, SPEAK_FILTER_RESULT);
					return;
				}
			}
		}
	}

	// Reject results without free slots
	if((onlyFree && aResult->getFreeSlots() < 1))
	{
		droppedResults++;
		StupidWin::postMessage(this, WM_SPEAKER, SPEAK_FILTER_RESULT);
		return;
	}

	SearchInfo* i = new SearchInfo(aResult);
	StupidWin::postMessage(this, WM_SPEAKER, SPEAK_ADD_RESULT, (LPARAM)i);
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
	StupidWin::postMessage(this, WM_SPEAKER, WPARAM(s), LPARAM(hubInfo));
}

#ifdef PORT_ME

void SearchFrame::onEnter() {
	StringList clients;

	// Change Default Settings If Changed
	if (onlyFree != BOOLSETTING(SEARCH_ONLY_FREE_SLOTS))
		SettingsManager::getInstance()->set(SettingsManager::SEARCH_ONLY_FREE_SLOTS, onlyFree);
	if (!initialType && ctrlFiletype.GetCurSel() != SETTING(LAST_SEARCH_TYPE))
		SettingsManager::getInstance()->set(SettingsManager::LAST_SEARCH_TYPE, ctrlFiletype.GetCurSel());

	if(!(ctrlSearch.GetWindowTextLength() > 0))
		return;

	int n = hubs->GetItemCount();
	for(int i = 0; i < n; i++) {
		if(hubs->GetCheckState(i)) {
			clients.push_back(Text::fromT(hubs->getItemData(i)->url));
		}
	}

	if(!clients.size())
		return;

	tstring s(ctrlSearch.GetWindowTextLength() + 1, _T('\0'));
	ctrlSearch.GetWindowText(&s[0], s.size());
	s.resize(s.size()-1);

	tstring size(ctrlSize.GetWindowTextLength() + 1, _T('\0'));
	ctrlSize.GetWindowText(&size[0], size.size());
	size.resize(size.size()-1);

	double lsize = Util::toDouble(Text::fromT(size));
	switch(ctrlSizeMode.GetCurSel()) {
	case 1:
		lsize*=1024.0; break;
	case 2:
		lsize*=1024.0*1024.0; break;
	case 3:
		lsize*=1024.0*1024.0*1024.0; break;
	}

	int64_t llsize = (int64_t)lsize;

	for(int i = 0; i != results->GetItemCount(); i++) {
		delete results->getItemData(i);
	}
	results->DeleteAllItems();

	{
		Lock l(cs);
		search = StringTokenizer<tstring>(s, ' ').getTokens();
		s.clear();
		//strip out terms beginning with -
		for(TStringList::iterator si = search.begin(); si != search.end(); ) {
			if(si->empty()) {
				si = search.erase(si);
				continue;
			}
			if ((*si)[0] != _T('-')) 
				s += *si + _T(' ');	
			++si;
		}

		s = s.substr(0, max(s.size(), static_cast<tstring::size_type>(1)) - 1);
	}


	SearchManager::SizeModes mode((SearchManager::SizeModes)ctrlMode.GetCurSel());
	if(llsize == 0)
		mode = SearchManager::SIZE_DONTCARE;

	int ftype = ctrlFiletype.GetCurSel();


	// Add new searches to the last-search dropdown list
	if(find(lastSearches.begin(), lastSearches.end(), s) == lastSearches.end())
	{
		int i = max(SETTING(SEARCH_HISTORY)-1, 0);

		if(ctrlSearchBox.GetCount() > i)
			ctrlSearchBox.DeleteString(i);
		ctrlSearchBox.InsertString(0, s.c_str());

		while(lastSearches.size() > (TStringList::size_type)i) {
			lastSearches.erase(lastSearches.begin());
		}
		lastSearches.push_back(s);
	}

	ctrlStatus.SetText(1, (TSTRING(SEARCHING_FOR) + s + _T("...")).c_str());
	ctrlStatus.SetText(2, _T(""));
	ctrlStatus.SetText(3, _T(""));
	droppedResults = 0;
	isHash = (ftype == SearchManager::TYPE_TTH);

	SetWindowText((TSTRING(SEARCH) + _T(" - ") + s).c_str());

	if(SearchManager::getInstance()->okToSearch()) {
		SearchManager::getInstance()->search(clients, Text::fromT(s), llsize,
			(SearchManager::TypeModes)ftype, mode, "manual");
		if(BOOLSETTING(CLEAR_SEARCH)) // Only clear if the search was sent
			ctrlSearch.SetWindowText(_T(""));
	} else {
		int32_t waitFor = SearchManager::getInstance()->timeToSearch();
		AutoArray<TCHAR> buf(TSTRING(SEARCHING_WAIT).size() + 16);
		_stprintf(buf, CTSTRING(SEARCHING_WAIT), waitFor);

		ctrlStatus.SetText(1, buf);
		ctrlStatus.SetText(2, _T(""));
		ctrlStatus.SetText(3, _T(""));

		SetWindowText((TSTRING(SEARCH) + _T(" - ") + tstring(buf)).c_str());
		// Start the countdown timer
		timerID = SetTimer(1, 1000);
	}

}

LRESULT SearchFrame::onTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	int32_t waitFor = SearchManager::getInstance()->timeToSearch();
	if(waitFor > 0) {
		AutoArray<TCHAR> buf(TSTRING(SEARCHING_WAIT).size() + 16);
		_stprintf(buf, CTSTRING(SEARCHING_WAIT), waitFor);

		ctrlStatus.SetText(1, buf);
		SetWindowText((TSTRING(SEARCH) + _T(" - ") + tstring(buf)).c_str());
	} else {
		if(timerID != 0) {
			KillTimer(timerID);
			timerID = 0;
		}

		ctrlStatus.SetText(1, (TSTRING(SEARCHING_READY)).c_str());
		SetWindowText((TSTRING(SEARCH) + _T(" - ") + TSTRING(SEARCHING_READY)).c_str());
	}

	ctrlStatus.SetText(2, _T(""));
	ctrlStatus.SetText(3, _T(""));
	return 0;
}

void SearchFrame::runUserCommand(UserCommand& uc) {
	if(!WinUtil::getUCParams(handle(), uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	set<CID> users;

	int sel = -1;
	while((sel = results->GetNextItem(sel, LVNI_SELECTED)) != -1) {
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

LRESULT SearchFrame::onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	HWND hWnd = (HWND)lParam;
	HDC hDC = (HDC)wParam;

	if(hWnd == searchLabel.handle() || hWnd == sizeLabel.handle() || hWnd == optionLabel.handle() || hWnd == typeLabel.handle()
		|| hWnd == hubsLabel.handle() || hWnd == ctrlSlots.handle()) {
		::SetBkColor(hDC, ::GetSysColor(COLOR_3DFACE));
		::SetTextColor(hDC, ::GetSysColor(COLOR_BTNTEXT));
		return (LRESULT)::GetSysColorBrush(COLOR_3DFACE);
	} else {
		::SetBkColor(hDC, WinUtil::bgColor);
		::SetTextColor(hDC, WinUtil::textColor);
		return (LRESULT)WinUtil::bgBrush;
	}
}

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

void SearchFrame::onTab(bool shift) {
	HWND wnds[] = {
		ctrlSearch.handle(), ctrlPurge.handle(), ctrlMode.handle(), ctrlSize.handle(), ctrlSizeMode.handle(),
		ctrlFiletype.handle(), ctrlSlots.handle(), ctrlDoSearch.handle(), ctrlSearch.handle(),
		results->handle()
	};

	HWND focus = GetFocus();
	if(focus == ctrlSearchBox.handle())
		focus = ctrlSearch.handle();

	static const int size = sizeof(wnds) / sizeof(wnds[0]);
	int i;
	for(i = 0; i < size; i++) {
		if(wnds[i] == focus)
			break;
	}

	::SetFocus(wnds[(i + (shift ? -1 : 1)) % size]);
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
