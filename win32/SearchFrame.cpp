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

#include "SearchFrame.h"

#include <dcpp/FavoriteManager.h>
#include <dcpp/QueueManager.h>
#include <dcpp/ClientManager.h>

int SearchFrame::columnIndexes[] = { COLUMN_FILENAME, COLUMN_NICK, COLUMN_TYPE, COLUMN_SIZE,
	COLUMN_PATH, COLUMN_SLOTS, COLUMN_CONNECTION, COLUMN_HUB, COLUMN_EXACT_SIZE, COLUMN_IP, COLUMN_TTH, COLUMN_CID };
int SearchFrame::columnSizes[] = { 200, 100, 50, 80, 100, 40, 70, 150, 80, 100, 125, 125 };
static const char* columnNames[] = {
	N_("File"),
	N_("User"),
	N_("Type"),
	N_("Size"),
	N_("Path"),
	N_("Slots"),
	N_("Connection"),
	N_("Hub"),
	N_("Exact size"),
	N_("IP"),
	N_("TTH Root"),
	N_("CID")
};

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


void SearchFrame::openWindow(SmartWin::WidgetTabView* mdiParent, const tstring& str /* = Util::emptyStringT */, LONGLONG size /* = 0 */, SearchManager::SizeModes mode /* = SearchManager::SIZE_ATLEAST */, SearchManager::TypeModes type /* = SearchManager::TYPE_ANY */) {
	SearchFrame* pChild = new SearchFrame(mdiParent, str, size, mode, type);
	frames.insert(pChild);
}

void SearchFrame::closeAll() {
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		(*i)->close(true);
}

SearchFrame::SearchFrame(SmartWin::WidgetTabView* mdiParent, const tstring& initialString_, LONGLONG initialSize_, SearchManager::SizeModes initialMode_, SearchManager::TypeModes initialType_) :
	BaseType(mdiParent, T_("Search"), SmartWin::IconPtr(new SmartWin::Icon(IDR_SEARCH))),
	searchLabel(0),
	searchBox(0),
	purge(0),
	sizeLabel(0),
	mode(0),
	size(0),
	sizeMode(0),
	typeLabel(0),
	fileType(0),
	optionLabel(0),
	slots(0),
	onlyFree(BOOLSETTING(SEARCH_ONLY_FREE_SLOTS)),
	hubsLabel(0),
	hubs(0),
	doSearch(0),
	results(0),
	showUI(0),
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
		searchLabel->setText(T_("Search for"));

		sizeLabel = createStatic(cs);
		sizeLabel->setText(T_("Size"));
		
		typeLabel = createStatic(cs);
		typeLabel->setText(T_("File type"));

		optionLabel = createStatic();
		optionLabel->setText(T_("Search options"));

		hubsLabel = createStatic();
		hubsLabel->setText(T_("Hubs"));

	}

	{
		searchBox = createComboBox(WinUtil::Seeds::comboBoxEdit);
		addWidget(searchBox);
		
		for(TStringIter i = lastSearches.begin(); i != lastSearches.end(); ++i) {
			searchBox->insertValue(0, *i);
		}
		searchBox->getTextBox()->onKeyDown(std::tr1::bind(&SearchFrame::handleSearchKeyDown, this, _1));
	}

	{
		WidgetButton::Seed cs = WinUtil::Seeds::button;
		cs.style |= BS_DEFPUSHBUTTON;
		cs.caption = T_("Search");
		doSearch = createButton(cs);

		doSearch->onClicked(std::tr1::bind(&SearchFrame::runSearch, this));
	}

	{
		mode = createComboBox(WinUtil::Seeds::comboBoxStatic);
		addWidget(mode);

		mode->addValue(T_("Normal"));
		mode->addValue(T_("At least"));
		mode->addValue(T_("At most"));
	}

	{
		WidgetTextBox::Seed cs = WinUtil::Seeds::textBox;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_NUMBER;
		size = createTextBox(cs);
		addWidget(size);
	}

	{
		sizeMode = createComboBox(WinUtil::Seeds::comboBoxStatic);
		addWidget(sizeMode);

		sizeMode->addValue(T_("B"));
		sizeMode->addValue(T_("KiB"));
		sizeMode->addValue(T_("MiB"));
		sizeMode->addValue(T_("GiB"));
		sizeMode->setSelectedIndex((initialSize == 0) ? 2 : 0);
	}

	{
		fileType = createComboBox(WinUtil::Seeds::comboBoxStatic);
		addWidget(fileType);

		fileType->addValue(T_("Any"));
		fileType->addValue(T_("Audio"));
		fileType->addValue(T_("Compressed"));
		fileType->addValue(T_("Document"));
		fileType->addValue(T_("Executable"));
		fileType->addValue(T_("Picture"));
		fileType->addValue(T_("Video"));
		fileType->addValue(T_("Directory"));
		fileType->addValue(T_("TTH"));
	}

	{
		WidgetCheckBox::Seed cs(T_("Only users with free slots"));
		slots = createCheckBox(cs);
		slots->setChecked(onlyFree);

		slots->onClicked(std::tr1::bind(&SearchFrame::handleSlotsClicked, this)) ;
	}

	{
		WidgetListView::Seed cs = WinUtil::Seeds::listView;
		cs.style |= LVS_NOCOLUMNHEADER;
		cs.lvStyle |= LVS_EX_CHECKBOXES;
		hubs = SmartWin::WidgetCreator<WidgetHubs>::create(this, cs);
		addWidget(hubs);

		TStringList dummy;
		dummy.push_back(Util::emptyStringT);
		hubs->createColumns(dummy);

		hubs->setColor(WinUtil::textColor, WinUtil::bgColor);

		hubs->onRaw(std::tr1::bind(&SearchFrame::handleHubItemChanged, this, _1, _2), SmartWin::Message(WM_NOTIFY, LVN_ITEMCHANGED));

		hubs->insert(new HubInfo(Util::emptyStringT, T_("Only where I'm op"), false));
		hubs->setChecked(0, false);


	}

	{
		results = SmartWin::WidgetCreator<WidgetResults>::create(this, WinUtil::Seeds::listView);
		addWidget(results);

		results->createColumns(WinUtil::getStrings(columnNames));
		results->setColumnOrder(WinUtil::splitTokens(SETTING(SEARCHFRAME_ORDER), columnIndexes));
		results->setColumnWidths(WinUtil::splitTokens(SETTING(SEARCHFRAME_WIDTHS), columnSizes));

		results->setColor(WinUtil::textColor, WinUtil::bgColor);
		results->setSmallImageList(WinUtil::fileImages);

		results->onDblClicked(std::tr1::bind(&SearchFrame::handleDoubleClick, this));
		results->onKeyDown(std::tr1::bind(&SearchFrame::handleKeyDown, this, _1));
		results->onContextMenu(std::tr1::bind(&SearchFrame::handleContextMenu, this, _1));
	}

	{
		WidgetButton::Seed cs = WinUtil::Seeds::button;
		cs.caption = T_("Purge");
		purge = createButton(cs);

		purge->onClicked(std::tr1::bind(&SearchFrame::handlePurgeClicked, this));
	}


	{
		WidgetCheckBox::Seed cs(_T("+/-"));
		showUI = createCheckBox(cs);
		showUI->setChecked(bShowUI);

		showUI->onClicked(std::tr1::bind(&SearchFrame::handleShowUIClicked, this));
	}

	initStatus();
	
	statusSizes[STATUS_SHOW_UI] = 16; ///@todo get real checkbox width

	layout();

	onSpeaker(std::tr1::bind(&SearchFrame::handleSpeaker, this, _1, _2));

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
		mode->setSelectedIndex(1);
		fileType->setSelectedIndex(SETTING(LAST_SEARCH_TYPE));
	}
	searchBox->setFocus();
}

SearchFrame::~SearchFrame() {
}

void SearchFrame::layout() {
	SmartWin::Rectangle r(getClientAreaSize()); 
	
	layoutStatus(r);
	mapWidget(STATUS_SHOW_UI, showUI);

	RECT rect = r, initialRect = rect;
	if(showUI->getChecked()) {
		const int width = 220, spacing = 50, labelH = 16, comboH = 140, lMargin = 2, rMargin = 4;

		rect.left += width;
		results->setBounds(rect);

		// "Search for"
		rect.right = width - rMargin;
		rect.left = lMargin;
		rect.top += 25;
		rect.bottom = rect.top + comboH + 21;
		searchBox->setBounds(rect);
		rect.bottom -= comboH;

		searchLabel->setBounds(SmartWin::Rectangle(rect.left + lMargin, rect.top - labelH, width - rMargin, labelH - 1));

		// "Purge"
		rect.right = rect.left + spacing;
		rect.left = lMargin;
		rect.top += 25;
		rect.bottom = rect.top + 21;
		purge->setBounds(rect);
		
		// "Search"
		rect.right = width - rMargin;
		rect.left = rect.right - 100;
		doSearch->setBounds(rect);
		
		// "Size"
		int w2 = width - rMargin - lMargin;
		rect.top += spacing;
		rect.bottom = rect.top + comboH;
		rect.left = lMargin;
		rect.right = w2/3;
		mode->setBounds(rect);

		sizeLabel->setBounds(SmartWin::Rectangle(rect.left + lMargin, rect.top - labelH, width - rMargin, labelH - 1));

		rect.left = rect.right + lMargin;
		rect.right += w2/3;
		rect.bottom = rect.top + 21;
		size->setBounds(rect);

		rect.left = rect.right + lMargin;
		rect.right = width - rMargin;
		rect.bottom = rect.top + comboH;
		sizeMode->setBounds(rect);

		// "File type"
		rect.left = lMargin;
		rect.right = width - rMargin;
		rect.top += spacing;
		rect.bottom = rect.top + comboH + 21;
		fileType->setBounds(rect);
		rect.bottom -= comboH;

		typeLabel->setBounds(SmartWin::Rectangle(rect.left + lMargin, rect.top - labelH, width - rMargin, labelH - 1));

		// "Search options"
		rect.left = lMargin;
		rect.right = width - rMargin;
		rect.top += spacing;
		rect.bottom += spacing;
		slots->setBounds(rect);

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

		hubs->setBounds(rect);

		hubsLabel->setBounds(SmartWin::Rectangle(rect.left + lMargin, rect.top - labelH, width - rMargin, labelH - 1));

	} else {
		results->setBounds(rect);

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
	SettingsManager::getInstance()->set(SettingsManager::SEARCHFRAME_ORDER, WinUtil::toString(results->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::SEARCHFRAME_WIDTHS, WinUtil::toString(results->getColumnWidths()));
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
		columns[COLUMN_TYPE] = T_("Directory");
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

LRESULT SearchFrame::handleSpeaker(WPARAM wParam, LPARAM lParam) {
 	switch(wParam) {
	case SPEAK_ADD_RESULT:
		{
			SearchInfo* si = reinterpret_cast<SearchInfo*>(lParam);
			SearchResult* sr = si->sr;
			// Check previous search results for dupes
			for(int i = 0, j = results->size(); i < j; ++i) {
				SearchInfo* si2 = results->getData(i);
				SearchResult* sr2 = si2->sr;
				if((sr->getUser()->getCID() == sr2->getUser()->getCID()) && (sr->getFile() == sr2->getFile())) {
					delete si;
					return 0;
				}
			}

			results->insert(si);
			setStatus(STATUS_COUNT, str(TFN_("%1% item", "%1% items", results->size()) % results->size()));
			setDirty(SettingsManager::BOLD_SEARCH);
		}
		break;
	case SPEAK_FILTER_RESULT:
		setStatus(STATUS_FILTERED, str(TF_("%1% filtered") % droppedResults));
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

LRESULT SearchFrame::handleHubItemChanged(WPARAM wParam, LPARAM lParam) {
	LPNMLISTVIEW lv = (LPNMLISTVIEW)lParam;
	if(lv->iItem == 0 && (lv->uNewState ^ lv->uOldState) & LVIS_STATEIMAGEMASK) {
		if (((lv->uNewState & LVIS_STATEIMAGEMASK) >> 12) - 1) {
			for(int iItem = 0; (iItem = hubs->getNext(iItem, LVNI_ALL)) != -1; ) {
				HubInfo* client = hubs->getData(iItem);
				if (!client->op)
					hubs->setChecked(iItem, false);
			}
		}
	}
	return 0;
}

void SearchFrame::handleDoubleClick() {
	results->forEachSelectedT(SearchInfo::Download(Text::toT(SETTING(DOWNLOAD_DIRECTORY))));
}

bool SearchFrame::handleKeyDown(int c) {
	if(c == VK_DELETE) {
		postMessage(WM_COMMAND, IDC_REMOVE);
		return true;
	}
	return false;
}

bool SearchFrame::handleContextMenu(SmartWin::ScreenCoordinate pt) {
	if(results->getSelectedCount() > 0) {
		if(pt.x() == -1 && pt.y() == -1) {
			pt = results->getContextMenuPos();
		}

		WidgetMenuPtr contextMenu = makeMenu();
		contextMenu->trackPopupMenu(pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return true;
	}
	return false;
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
		int i = results->getNext(-1, LVNI_SELECTED);
		dcassert(i != -1);
		SearchInfo* si = results->getData(i);
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

void SearchFrame::handleRemove() {
	int i = -1;
	while((i = results->getNext(-1, LVNI_SELECTED)) != -1) {
		results->erase(i);
	}
}

SearchFrame::WidgetMenuPtr SearchFrame::makeMenu() {
	WidgetMenuPtr menu = createMenu(WinUtil::Seeds::menu);

	StringPairList favoriteDirs = FavoriteManager::getInstance()->getFavoriteDirs();
	SearchInfo::CheckTTH checkTTH = results->forEachSelectedT(SearchInfo::CheckTTH());

	menu->appendItem(IDC_DOWNLOAD, T_("&Download"), std::tr1::bind(&SearchFrame::handleDownload, this));
	addTargetMenu(menu, favoriteDirs, checkTTH);
	menu->appendItem(IDC_DOWNLOADDIR, T_("Download whole directory"), std::tr1::bind(&SearchFrame::handleDownloadDir, this));
	addTargetDirMenu(menu, favoriteDirs);
	menu->appendItem(IDC_VIEW_AS_TEXT, T_("&View as text"), std::tr1::bind(&SearchFrame::handleViewAsText, this));
	menu->appendSeparatorItem();
	if(checkTTH.hasTTH) {
		SearchInfo* si = results->getSelectedData();
		WinUtil::addHashItems(menu, TTHValue(Text::fromT(checkTTH.tth)), si->getText(COLUMN_FILENAME));
	}
	menu->appendSeparatorItem();
	appendUserItems(getParent(), menu);
	menu->appendSeparatorItem();
	menu->appendItem(IDC_REMOVE, T_("&Remove"), std::tr1::bind(&SearchFrame::handleRemove, this));
	prepareMenu(menu, UserCommand::CONTEXT_SEARCH, checkTTH.hubs);

	menu->setDefaultItem(IDC_DOWNLOAD);

	return menu;
}

void SearchFrame::addTargetMenu(const WidgetMenuPtr& parent, const StringPairList& favoriteDirs, const SearchInfo::CheckTTH& checkTTH) {
	WidgetMenuPtr menu = parent->appendPopup(T_("Download to..."));

	int n = 0;
	if(favoriteDirs.size() > 0) {
		for(StringPairList::const_iterator i = favoriteDirs.begin(); i != favoriteDirs.end(); i++)
			menu->appendItem(IDC_DOWNLOAD_FAVORITE_DIRS + n++, Text::toT(i->second), std::tr1::bind(&SearchFrame::handleDownloadFavoriteDirs, this, _1));
		menu->appendSeparatorItem();
	}

	n = 0;
	menu->appendItem(IDC_DOWNLOADTO, T_("&Browse..."), std::tr1::bind(&SearchFrame::handleDownloadTo, this));
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
	WidgetMenuPtr menu = parent->appendPopup(T_("Download whole directory to..."));

	int n = 0;
	if(favoriteDirs.size() > 0) {
		for(StringPairList::const_iterator i = favoriteDirs.begin(); i != favoriteDirs.end(); ++i)
			menu->appendItem(IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + n++, Text::toT(i->second), std::tr1::bind(&SearchFrame::handleDownloadWholeFavoriteDirs, this, _1));
		menu->appendSeparatorItem();
	}

	n = 0;
	menu->appendItem(IDC_DOWNLOADDIRTO, T_("&Browse..."), std::tr1::bind(&SearchFrame::handleDownloadDirTo, this));
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
		
		if(!aResult->getToken().empty() && token != aResult->getToken()) {
			droppedResults++;
			speak(SPEAK_FILTER_RESULT);
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
	int nItem = hubs->insert(info);
	hubs->setChecked(nItem, (hubs->isChecked(0) ? info->op : true));
	hubs->setColumnWidth(0, LVSCW_AUTOSIZE);
}

void SearchFrame::onHubChanged(HubInfo* info) {
	int nItem = 0;
	int n = hubs->size();
	for(; nItem < n; nItem++) {
		if(hubs->getData(nItem)->url == info->url)
			break;
	}
	if (nItem == n)
		return;

	hubs->setData(nItem, info);
	hubs->update(nItem);

	if (hubs->isChecked(0))
		hubs->setChecked(nItem, info->op);

	hubs->setColumnWidth(0, LVSCW_AUTOSIZE);
}

void SearchFrame::onHubRemoved(HubInfo* info) {
	int nItem = 0;
	int n = hubs->size();
	for(; nItem < n; nItem++) {
		if(hubs->getData(nItem)->url == info->url)
			break;
	}
	if (nItem == n)
		return;

	hubs->erase(nItem);
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

	int n = hubs->size();
	for(int i = 0; i < n; i++) {
		if(hubs->isChecked(i)) {
			clients.push_back(Text::fromT(hubs->getData(i)->url));
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

	results->clear();

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
		token = Util::toString(Util::rand());
	}

	SearchManager::SizeModes searchMode((SearchManager::SizeModes)mode->getSelectedIndex());
	if(llsize == 0)
		searchMode = SearchManager::SIZE_DONTCARE;

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

	setStatus(STATUS_STATUS, str(TF_("Searching for %1%...") % s));
	setStatus(STATUS_COUNT, Util::emptyStringT);
	setStatus(STATUS_FILTERED, Util::emptyStringT);
	droppedResults = 0;
	isHash = (ftype == SearchManager::TYPE_TTH);

	setText(str(TF_("Search - %1%") % s));

	if(SearchManager::getInstance()->okToSearch()) {
		SearchManager::getInstance()->search(clients, Text::fromT(s), llsize,
			(SearchManager::TypeModes)ftype, searchMode, token);
		if(BOOLSETTING(CLEAR_SEARCH)) // Only clear if the search was sent
			searchBox->setText(Util::emptyStringT);
	} else {
		int32_t waitFor = SearchManager::getInstance()->timeToSearch();
		tstring msg = str(TFN_("Searching too soon, next search in %1% second", "Searching too soon, next search in %1% seconds", waitFor) % waitFor);

		setStatus(STATUS_STATUS, msg);
		setStatus(STATUS_COUNT, Util::emptyStringT);
		setStatus(STATUS_FILTERED, Util::emptyStringT);

		setText(str(TF_("Search - %1%") % msg));
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
		tstring msg = str(TFN_("Searching too soon, next search in %1% second", "Searching too soon, next search in %1% seconds", waitFor) % waitFor);
		setStatus(STATUS_STATUS, msg);
		setText(str(TF_("Search - %1%") % msg));
		return true;
	} 
	
	setStatus(STATUS_STATUS, T_("Ready to search..."));
	setText(T_("Search - Ready to search..."));
	
	return false;
}

void SearchFrame::runUserCommand(const UserCommand& uc) {
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	set<CID> users;

	int sel = -1;
	while((sel = results->getNext(sel, LVNI_SELECTED)) != -1) {
		SearchResult* sr = results->getData(sel)->sr;

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

bool SearchFrame::handleSearchKeyDown(int c) {
	if(c == VK_RETURN && !(WinUtil::isShift() || WinUtil::isCtrl() || WinUtil::isAlt())) {
		runSearch();
		return true;
	}
	return false;
}

void SearchFrame::handleGetList() {
	results->forEachSelected(&SearchInfo::getList);
}

void SearchFrame::handleBrowseList() {
	results->forEachSelected(&SearchInfo::browseList);
}
