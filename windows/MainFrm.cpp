/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "MainFrm.h"
#include "AboutDlg.h"
#include "HubFrame.h"
#include "SearchFrm.h"
#include "PublicHubsFrm.h"
#include "PropertiesDlg.h"
#include "UsersFrame.h"
#include "DirectoryListingFrm.h"
#include "FavoritesFrm.h"
#include "NotepadFrame.h"
#include "QueueFrame.h"
#include "SpyFrame.h"
#include "FinishedFrame.h"
#include "ADLSearchFrame.h"
#include "PrivateFrame.h"
#include "FinishedULFrame.h"

#include "../client/ConnectionManager.h"
#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/StringTokenizer.h"
#include "../client/SimpleXML.h"

int MainFrame::columnIndexes[] = { COLUMN_USER, COLUMN_STATUS, COLUMN_FILE, COLUMN_SIZE, COLUMN_PATH };
int MainFrame::columnSizes[] = { 200, 300, 200, 100, 200 };

static ResourceManager::Strings columnNames[] = { ResourceManager::USER, ResourceManager::STATUS,
	ResourceManager::FILENAME, ResourceManager::SIZE, ResourceManager::PATH };

MainFrame::~MainFrame() {
	arrows.Destroy();
	images.Destroy();
	m_CmdBar.m_hImageList = NULL;

	largeImages.Destroy();

	DeleteObject(WinUtil::bgBrush);
	DeleteObject(WinUtil::font);
	WinUtil::fileImages.Destroy();
}

DWORD WINAPI MainFrame::stopper(void* p) {
	MainFrame* mf = (MainFrame*)p;
	HWND wnd, wnd2 = NULL;

	while( (wnd=::GetWindow(mf->m_hWndClient, GW_CHILD)) != NULL) {
		if(wnd == wnd2) 
			Sleep(100);
		else { 
			::PostMessage(wnd, WM_CLOSE, 0, 0);
			wnd2 = wnd;
		}
	}

	shutdown();
	
	mf->PostMessage(WM_CLOSE);	
	return 0;
}

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	TimerManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
	ConnectionManager::getInstance()->addListener(this);
	QueueManager::getInstance()->addListener(this);

	// Register server socket message
	WSAAsyncSelect(ConnectionManager::getInstance()->getServerSocket().getSocket(),
		m_hWnd, SERVER_SOCKET_MESSAGE, FD_ACCEPT);

	WinUtil::mainWnd = m_hWnd;

	LOGFONT lf;
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
	SettingsManager::getInstance()->setDefault(SettingsManager::TEXT_FONT, WinUtil::encodeFont(lf));
	WinUtil::decodeFont(SETTING(TEXT_FONT), lf);

	WinUtil::bgBrush = CreateSolidBrush(SETTING(BACKGROUND_COLOR));
	WinUtil::textColor = SETTING(TEXT_COLOR);
	WinUtil::bgColor = SETTING(BACKGROUND_COLOR);
	WinUtil::font = ::CreateFontIndirect(&lf);
	WinUtil::fontHeight = WinUtil::getTextHeight(m_hWnd, WinUtil::font);
	lf.lfWeight = FW_BOLD;
	WinUtil::boldFont = ::CreateFontIndirect(&lf);

	trayMessage = RegisterWindowMessage("TaskbarCreated");

	if(BOOLSETTING(USE_SYSTEM_ICONS)) {
		SHFILEINFO fi;
		WinUtil::fileImages = CImageList::Duplicate((HIMAGELIST)::SHGetFileInfo(".", FILE_ATTRIBUTE_DIRECTORY, &fi, sizeof(fi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES));
		WinUtil::fileImages.SetBkColor(SETTING(BACKGROUND_COLOR));
		WinUtil::dirIconIndex = fi.iIcon;	
	} else {
		WinUtil::fileImages.CreateFromImage(IDB_FOLDERS, 16, 3, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
		WinUtil::dirIconIndex = 0;
	}

	TimerManager::getInstance()->start();

	if(!SETTING(LANGUAGE_FILE).empty()) {
		ResourceManager::getInstance()->loadLanguage(SETTING(LANGUAGE_FILE));
	}

	// Set window name
	SetWindowText(APPNAME " " VERSIONSTRING);

	// Load images
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);

	WinUtil::buildMenu();
	m_hMenu = WinUtil::mainMenu;

	// attach menu
	m_CmdBar.AttachMenu(WinUtil::mainMenu);
	// load command bar images
	images.CreateFromImage(IDB_TOOLBAR, 16, 5, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	largeImages.CreateFromImage(IDB_TOOLBAR20, 20, 5, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	m_CmdBar.m_hImageList = images;

	m_CmdBar.m_arrCommand.Add(ID_FILE_CONNECT);
	m_CmdBar.m_arrCommand.Add(ID_FILE_RECONNECT);
	m_CmdBar.m_arrCommand.Add(IDC_FOLLOW);
	m_CmdBar.m_arrCommand.Add(IDC_FAVORITES);
	m_CmdBar.m_arrCommand.Add(IDC_QUEUE);
	m_CmdBar.m_arrCommand.Add(IDC_FINISHED); // adds icon to File menu
	m_CmdBar.m_arrCommand.Add(IDC_FINISHED_UL); // Finished Upload 16 x 16 menu icon
	m_CmdBar.m_arrCommand.Add(ID_FILE_SEARCH);
	m_CmdBar.m_arrCommand.Add(IDC_FILE_ADL_SEARCH);
	m_CmdBar.m_arrCommand.Add(ID_FILE_SETTINGS);
	m_CmdBar.m_arrCommand.Add(IDC_NOTEPAD);
	
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = createToolbar();

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	CreateSimpleStatusBar();

	ctrlStatus.Attach(m_hWndStatusBar);
	ctrlStatus.SetSimple(FALSE);
	int w[7] = { 0, 0, 0, 0, 0, 0, 0 };
	ctrlStatus.SetParts(7, w);
	statusSizes[0] = WinUtil::getTextWidth(STRING(AWAY), ::GetDC(ctrlStatus.m_hWnd)); // for "AWAY" segment

	CreateMDIClient();
	m_CmdBar.SetMDIClient(m_hWndMDIClient);

	arrows.CreateFromImage(IDB_ARROWS, 16, 2, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	ctrlTransfers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_TRANSFERS);

	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlTransfers.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	} else {
		ctrlTransfers.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP);
	}

	WinUtil::splitTokens(columnIndexes, SETTING(MAINFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(MAINFRAME_WIDTHS), COLUMN_LAST);

	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_SIZE) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlTransfers.InsertColumn(j, CSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}

	ctrlTransfers.SetColumnOrderArray(COLUMN_LAST, columnIndexes);

	ctrlTransfers.SetBkColor(WinUtil::bgColor);
	ctrlTransfers.SetTextBkColor(WinUtil::bgColor);
	ctrlTransfers.SetTextColor(WinUtil::textColor);

	ctrlTransfers.SetImageList(arrows, LVSIL_SMALL);

	ctrlTab.Create(m_hWnd, rcDefault);
	WinUtil::tabCtrl = &ctrlTab;

	SetSplitterPanes(m_hWndClient, ctrlTransfers.m_hWnd);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	m_nProportionalPos = 8000;

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {

		short lastPort = (short)SETTING(IN_PORT);
		short firstPort = lastPort;

		while(true) {
			try {
				ConnectionManager::getInstance()->setPort(lastPort);
				WSAAsyncSelect(ConnectionManager::getInstance()->getServerSocket().getSocket(), m_hWnd, SERVER_SOCKET_MESSAGE, FD_ACCEPT);
				SearchManager::getInstance()->setPort(lastPort);
				break;
			} catch(Exception e) {
				dcdebug("MainFrame::OnCreate caught %s\n", e.getError().c_str());
				short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
				SettingsManager::getInstance()->setDefault(SettingsManager::IN_PORT, newPort);
				if(SETTING(IN_PORT) == lastPort || (firstPort == newPort)) {
					// Changing default didn't change port, a fixed port must be in use...(or we
					// tried all ports
					char* buf = new char[STRING(PORT_IS_BUSY).size() + 8];
					sprintf(buf, CSTRING(PORT_IS_BUSY), SETTING(IN_PORT));
					MessageBox(buf);
					delete[] buf;
					break;
				}
				lastPort = newPort;
			}
		}
	}

	transferMenu.CreatePopupMenu();

	transferMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	transferMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(SEND_PRIVATE_MESSAGE));
	transferMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CSTRING(GRANT_EXTRA_SLOT));
	transferMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CSTRING(ADD_TO_FAVORITES));
	transferMenu.AppendMenu(MF_STRING, IDC_FORCE, CSTRING(FORCE_ATTEMPT));
	transferMenu.AppendMenu(MF_SEPARATOR, 0, (LPTSTR)NULL);
	transferMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(CLOSE_CONNECTION));
	transferMenu.AppendMenu(MF_STRING, IDC_REMOVEALL, CSTRING(REMOVE_FROM_ALL));

	c->addListener(this);
	c->downloadFile("http://dcplusplus.sourceforge.net/version.xml");

	if(BOOLSETTING(OPEN_PUBLIC))
		PostMessage(WM_COMMAND, ID_FILE_CONNECT);
	if(BOOLSETTING(OPEN_QUEUE))
		PostMessage(WM_COMMAND, IDC_QUEUE);

	PostMessage(WM_SPEAKER, AUTO_CONNECT);
	PostMessage(WM_SPEAKER, PARSE_COMMAND_LINE);

	Util::ensureDirectory(SETTING(LOG_DIRECTORY));

	ctrlTransfers.setSort(COLUMN_STATUS, ExListViewCtrl::SORT_FUNC, true, sortStatus);

	// We want to pass this one on to the splitter...hope it get's there...
	bHandled = FALSE;
	return 0;
}

HWND MainFrame::createToolbar() {

	CToolBarCtrl ctrl;
	ctrl.Create(m_hWnd, NULL, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, 0, ATL_IDW_TOOLBAR);
	ctrl.SetImageList(largeImages);

	TBBUTTON tb[11];
	memset(tb, 0, sizeof(tb));
	int n = 0;
	tb[n].iBitmap = n;
	tb[n].idCommand = ID_FILE_CONNECT;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = ID_FILE_RECONNECT;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_FOLLOW;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_FAVORITES;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_QUEUE;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++; // toolbar icon
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_FINISHED;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++; // Finished Upload 20x20 toolbar icon
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_FINISHED_UL;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = ID_FILE_SEARCH;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_FILE_ADL_SEARCH;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = ID_FILE_SETTINGS;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_NOTEPAD;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	ctrl.SetButtonStructSize();
	ctrl.AddButtons(11, tb);
	ctrl.AutoSize();

	return ctrl.m_hWnd;
}

LRESULT MainFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		
	if(wParam == ADD_DOWNLOAD_ITEM) {
		StringListInfo* i = (StringListInfo*)lParam;
		StringList l;
		for(int j = 0; j < COLUMN_LAST; j++) {
			l.push_back(i->columns[j]);
		}
		dcassert(ctrlTransfers.find(i->lParam) == -1);
		ctrlTransfers.insert(l, IMAGE_DOWNLOAD, i->lParam);
		delete i;
	} else if(wParam == ADD_UPLOAD_ITEM) {
		StringListInfo* i = (StringListInfo*)lParam;
		StringList l;
		for(int j = 0; j < COLUMN_LAST; j++) {
			l.push_back(i->columns[j]);
		}
		dcassert(ctrlTransfers.find(i->lParam) == -1);
		ctrlTransfers.insert(l, IMAGE_UPLOAD, i->lParam);
		delete i;
	} else if(wParam == REMOVE_ITEM) {
		dcassert(ctrlTransfers.find(lParam) != -1);
		ctrlTransfers.DeleteItem(ctrlTransfers.find(lParam));
		delete (ItemInfo*)lParam;
	} else if(wParam == SET_TEXT) {
		StringListInfo* l = (StringListInfo*)lParam;
		int n = ctrlTransfers.find(l->lParam);
		if(n != -1) {
			ctrlTransfers.SetRedraw(FALSE);
			for(int i = 0; i < COLUMN_LAST; i++) {
				if(!l->columns[i].empty()) {
					ctrlTransfers.SetItemText(n, i, l->columns[i].c_str());
				}
			}
			ctrlTransfers.SetRedraw(TRUE);
		}
		
		if(ctrlTransfers.getSortColumn() != COLUMN_USER)
			ctrlTransfers.resort();

		delete l;
	} else if(wParam == SET_TEXTS) {
		vector<StringListInfo*>* v = (vector<StringListInfo*>*)lParam;
		ctrlTransfers.SetRedraw(FALSE);
		for(vector<StringListInfo*>::iterator j = v->begin(); j != v->end(); ++j) {
			StringListInfo* l = *j;
			int n = ctrlTransfers.find(l->lParam);
			if(n != -1) {
				for(int i = 0; i < COLUMN_LAST; i++) {
					if(!l->columns[i].empty()) {
						ctrlTransfers.SetItemText(n, i, l->columns[i].c_str());
					}
				}
			}
			delete l;
		}
		ctrlTransfers.SetRedraw(TRUE);
		ctrlTransfers.resort();
		delete v;

	} else if(wParam == DOWNLOAD_LISTING) {
		DirectoryListInfo* i = (DirectoryListInfo*)lParam;
		try {
			DirectoryListingFrame* pChild = new DirectoryListingFrame(i->file, i->user);
			pChild->setTab(&ctrlTab);
			pChild->CreateEx(m_hWndClient);
			pChild->setWindowTitle();
			delete i;
		} catch(FileException e) {
			// ...
		}
	} else if(wParam == STATS) {
		StringList& str = *(StringList*)lParam;
		if(ctrlStatus.IsWindow()) {
			HDC dc = ::GetDC(ctrlStatus.m_hWnd);
			bool u = false;
			for(int i = 0; i < 6; i++) {
				int w = WinUtil::getTextWidth(str[i], dc);
				
				if(statusSizes[i] < w) {
					statusSizes[i] = w;
					u = true;
				}
				ctrlStatus.SetText(i+1, str[i].c_str());
			}
			::ReleaseDC(ctrlStatus.m_hWnd, dc);
			if(u)
				UpdateLayout(TRUE);
		}
		delete &str;
	} else if(wParam == AUTO_CONNECT) {
		autoConnect(HubManager::getInstance()->getFavoriteHubs());
	} else if(wParam == PARSE_COMMAND_LINE) {
		parseCommandLine(GetCommandLine());
	}

	return 0;
}

void MainFrame::onConnectionAdded(ConnectionQueueItem* aCqi) {
	ItemInfo::Types t = aCqi->getConnection() && aCqi->getConnection()->isSet(UserConnection::FLAG_UPLOAD) ? ItemInfo::TYPE_UPLOAD : ItemInfo::TYPE_DOWNLOAD;
	ItemInfo* ii = new ItemInfo(aCqi->getUser(), t, ItemInfo::STATUS_WAITING);
	
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) == transferItems.end());
		transferItems.insert(make_pair(aCqi, ii));
	}
	StringListInfo* i = new StringListInfo((LPARAM)ii);
	i->columns[COLUMN_USER] = aCqi->getUser()->getNick() + " (" + aCqi->getUser()->getClientName() + ")";
	i->columns[COLUMN_STATUS] = STRING(CONNECTING);

	if(aCqi->getConnection() && aCqi->getConnection()->isSet(UserConnection::FLAG_UPLOAD)) {
		PostMessage(WM_SPEAKER, ADD_UPLOAD_ITEM, (LPARAM)i);
	} else {
		PostMessage(WM_SPEAKER, ADD_DOWNLOAD_ITEM, (LPARAM)i);
	}
}

void MainFrame::onConnectionStatus(ConnectionQueueItem* aCqi) {
	ItemInfo* ii;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		ii = transferItems[aCqi];		
	}
	StringListInfo* i = new StringListInfo((LPARAM)ii);
	i->columns[COLUMN_STATUS] = aCqi->getState() == ConnectionQueueItem::CONNECTING ? STRING(CONNECTING) : STRING(WAITING_TO_RETRY);
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void MainFrame::onConnectionRemoved(ConnectionQueueItem* aCqi) {
	ItemInfo* ii;
	{
		Lock l(cs);
		ItemInfo::MapIter i = transferItems.find(aCqi);
		dcassert(i != transferItems.end());
		ii = i->second;
		transferItems.erase(i);
	}
	PostMessage(WM_SPEAKER, REMOVE_ITEM, (LPARAM)ii);
}

void MainFrame::onConnectionFailed(ConnectionQueueItem* aCqi, const string& aReason) {
	ItemInfo* ii;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		ii = transferItems[aCqi];		
	}
	StringListInfo* i = new StringListInfo((LPARAM)ii);
	i->columns[COLUMN_STATUS] = aReason;
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void MainFrame::onUploadStarting(Upload* aUpload) {
	ConnectionQueueItem* aCqi = aUpload->getUserConnection()->getCQI();
	ItemInfo* ii;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		ii = transferItems[aCqi];		
	}
	ii->pos = 0;
	ii->size = aUpload->getSize();
	ii->status = ItemInfo::STATUS_RUNNING;

	StringListInfo* i = new StringListInfo((LPARAM)ii);
	
	i->columns[COLUMN_FILE] = Util::getFileName(aUpload->getFileName());
	i->columns[COLUMN_PATH] = Util::getFilePath(aUpload->getFileName());
	i->columns[COLUMN_STATUS] = STRING(UPLOAD_STARTING);
	i->columns[COLUMN_SIZE] = Util::formatBytes(aUpload->getSize());

	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void MainFrame::onUploadTick(const Upload::List& ul) {
	vector<StringListInfo*>* v = new vector<StringListInfo*>();
	v->reserve(ul.size());

	char* buf = new char[STRING(UPLOADED_LEFT).size() + 32];
	
	{
		Lock l(cs);
		for(Upload::List::const_iterator j = ul.begin(); j != ul.end(); ++j) {
			Upload* u = *j;
			
			ConnectionQueueItem* aCqi = u->getUserConnection()->getCQI();
			ItemInfo* ii = transferItems[aCqi];		
			ii->pos = u->getPos();

			sprintf(buf, CSTRING(UPLOADED_LEFT), Util::formatBytes(u->getPos()).c_str(), 
				(double)u->getPos()*100.0/(double)u->getSize(), Util::formatBytes(u->getRunningAverage()).c_str(), Util::formatSeconds(u->getSecondsLeft()).c_str());
			
			StringListInfo* i = new StringListInfo((LPARAM)ii);
			i->columns[COLUMN_STATUS] = buf;
			
			v->push_back(i);
		}
	}

	delete[] buf;
	
	PostMessage(WM_SPEAKER, SET_TEXTS, (LPARAM)v);
}

void MainFrame::onUploadComplete(Upload* aUpload) {
	ConnectionQueueItem* aCqi = aUpload->getUserConnection()->getCQI();
	ItemInfo* ii;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		ii = transferItems[aCqi];		
	}
	ii->status = ItemInfo::STATUS_WAITING;
	ii->pos = 0;

	StringListInfo* i = new StringListInfo((LPARAM)ii);
	i->columns[COLUMN_STATUS] = STRING(UPLOAD_FINISHED_IDLE);
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);	
}

void MainFrame::onDownloadComplete(Download* p) {
	ConnectionQueueItem* aCqi = p->getUserConnection()->getCQI();
	ItemInfo* ii;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		ii = transferItems[aCqi];		
	}
	ii->status = ItemInfo::STATUS_WAITING;
	ii->pos = 0;

	StringListInfo* i = new StringListInfo((LPARAM)ii);
	i->columns[COLUMN_STATUS] = STRING(DOWNLOAD_FINISHED_IDLE);
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);	
}

void MainFrame::onDownloadFailed(Download* aDownload, const string& aReason) {
	ConnectionQueueItem* aCqi = aDownload->getUserConnection()->getCQI();
	ItemInfo* ii;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		ii = transferItems[aCqi];		
	}
	ii->status = ItemInfo::STATUS_WAITING;
	ii->pos = 0;
	
	StringListInfo* i = new StringListInfo((LPARAM)ii);
	i->columns[COLUMN_STATUS] = aReason;
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void MainFrame::onDownloadStarting(Download* aDownload) {
	ConnectionQueueItem* aCqi = aDownload->getUserConnection()->getCQI();
	ItemInfo* ii;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		ii = transferItems[aCqi];		
	}
	ii->status = ItemInfo::STATUS_RUNNING;
	ii->pos = 0;
	ii->size = aDownload->getSize();

	StringListInfo* i = new StringListInfo((LPARAM)ii);
	i->columns[COLUMN_FILE] = Util::getFileName(aDownload->getTarget());
	i->columns[COLUMN_PATH] = Util::getFilePath(aDownload->getTarget());
	i->columns[COLUMN_STATUS] = STRING(DOWNLOAD_STARTING);
	i->columns[COLUMN_SIZE] = Util::formatBytes(aDownload->getSize());
	
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void MainFrame::onDownloadTick(const Download::List& dl) {
	vector<StringListInfo*>* v = new vector<StringListInfo*>();
	v->reserve(dl.size());

	{
		Lock l(cs);
		for(Download::List::const_iterator j = dl.begin(); j != dl.end(); ++j) {
			Download* d = *j;
			char* buf = new char[STRING(DOWNLOADED_LEFT).size() + 32];
			sprintf(buf, CSTRING(DOWNLOADED_LEFT), Util::formatBytes(d->getPos()).c_str(), 
				(double)d->getPos()*100.0/(double)d->getSize(), Util::formatBytes(d->getRunningAverage()).c_str(), Util::formatSeconds(d->getSecondsLeft()).c_str());
			
			ConnectionQueueItem* aCqi = d->getUserConnection()->getCQI();
			ItemInfo* ii = transferItems[aCqi];
			ii->pos = d->getPos();

			StringListInfo* i = new StringListInfo((LPARAM)ii);
			i->columns[COLUMN_STATUS] = buf;
			delete[] buf;
			
			v->push_back(i);
		}
	}
	
	PostMessage(WM_SPEAKER, SET_TEXTS, (LPARAM)v);
}


void MainFrame::parseCommandLine(const string& cmdLine)
{
	string::size_type i = 0;
	string::size_type j;

	if( (j = cmdLine.find("dchub://", i)) != string::npos) {
		i = j + 8;
		string server;
		string user;
		if( (j = cmdLine.find('/', i)) == string::npos) {
			server = cmdLine.substr(i);
		} else {
			server = cmdLine.substr(i, j-i);
			i = j + 1;
			if( (j = cmdLine.find_first_of("\\/ ", i)) == string::npos) {
				user = cmdLine.substr(i);
			} else {
				user = cmdLine.substr(i, j-i);
			}
		}

		if(!server.empty()) {
			HubFrame::openWindow(m_hWndMDIClient, &ctrlTab, server);
		}
		if(!user.empty()) {
			try {
				QueueManager::getInstance()->addList(ClientManager::getInstance()->getUser(user));
			} catch(Exception) {
				// ...
			}
		}
	}
}

LRESULT MainFrame::onCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	string cmdLine = (LPCSTR) (((COPYDATASTRUCT *)lParam)->lpData);
	parseCommandLine(Util::getAppName() + " " + cmdLine);
	return true;
}

LRESULT MainFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 

	// Get the bounding rectangle of the client area. 
	ctrlTransfers.GetClientRect(&rc);
	ctrlTransfers.ScreenToClient(&pt); 
	if (PtInRect(&rc, pt) && ctrlTransfers.GetSelectedCount() > 0) 
	{ 
		ctrlTransfers.ClientToScreen(&pt);
		transferMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

		return TRUE; 
	}
	return FALSE; 
}

LRESULT MainFrame::OnFileSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	SearchFrame* pChild = new SearchFrame();
	pChild->setTab(&ctrlTab);
	pChild->CreateEx(m_hWndClient);
	return 0;
}	

LRESULT MainFrame::onFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(FavoriteHubsFrame::frame == NULL) {
		FavoriteHubsFrame* pChild = new FavoriteHubsFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(FavoriteHubsFrame::frame->m_hWnd);
	}
	return 0;
}

LRESULT MainFrame::onFavoriteUsers(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(UsersFrame::frame == NULL) {
		UsersFrame* pChild = new UsersFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(UsersFrame::frame->m_hWnd);
	}
	return 0;
}

LRESULT MainFrame::onNotepad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(NotepadFrame::frame == NULL) {
		NotepadFrame* pChild = new NotepadFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(NotepadFrame::frame->m_hWnd);
	}
	return 0;
}

LRESULT MainFrame::onQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(QueueFrame::frame == NULL) {
		QueueFrame* pChild = new QueueFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(QueueFrame::frame->m_hWnd);
	}
	return 0;
}

LRESULT MainFrame::OnFileConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(PublicHubsFrame::frame == NULL) {
		PublicHubsFrame* pChild = new PublicHubsFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(PublicHubsFrame::frame->m_hWnd);
	}

	return 0;
}

LRESULT MainFrame::onSearchSpy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(SpyFrame::frame == NULL) {
		SpyFrame* pChild = new SpyFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(SpyFrame::frame->m_hWnd);
	}
	
	return 0;
}

LRESULT MainFrame::onFileADLSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) 
{
	if(ADLSearchFrame::frame == NULL) 
	{
		ADLSearchFrame* pChild = new ADLSearchFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else 
	{
		MDIActivate(ADLSearchFrame::frame->m_hWnd);
	}

	return 0;
}

LRESULT MainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CAboutDlg dlg;
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT MainFrame::OnFileSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PropertiesDlg dlg(SettingsManager::getInstance());

	short lastPort = (short)SETTING(IN_PORT);
	short firstPort = lastPort;
	int lastConn = SETTING(CONNECTION_TYPE);

	if(dlg.DoModal(m_hWnd) == IDOK)
	{		
		SettingsManager::getInstance()->save();
		
		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE && 
			((SETTING(CONNECTION_TYPE) != lastConn) || (SETTING(IN_PORT) != lastPort))) {
			while(true) {
				try {
					lastPort = (short)SETTING(IN_PORT);
					ConnectionManager::getInstance()->setPort((short)SETTING(IN_PORT));
					SearchManager::getInstance()->setPort((short)SETTING(IN_PORT));
					break;
				} catch(Exception e) {
					dcdebug("MainFrame::OnCreate caught %s\n", e.getError().c_str());
					short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
					SettingsManager::getInstance()->setDefault(SettingsManager::IN_PORT, newPort);
					if(SETTING(IN_PORT) == lastPort || (firstPort == newPort)) {
						// Changing default didn't change port, a fixed port must be in use...(or we
						// tried all ports
						char* buf = new char[STRING(PORT_IS_BUSY).size() + 8];
						sprintf(buf, CSTRING(PORT_IS_BUSY), SETTING(IN_PORT));
						MessageBox(buf);
						delete[] buf;
						break;
					}
					lastPort = newPort;
				}
			}
		}
		ClientManager::getInstance()->infoUpdated();
	}
	return 0;
}

void MainFrame::onHttpComplete(HttpConnection* /*aConn*/)  {
	try {
		SimpleXML xml;
		xml.fromXML(versionInfo);
		xml.stepIn();
		if(xml.findChild("Version")) {
			if(atof(xml.getChildData().c_str()) > VERSIONFLOAT) {
				xml.resetCurrentChild();
				if(xml.findChild("Message")) {
					const string& msg = xml.getChildData();
					xml.resetCurrentChild();
					if(xml.findChild("Title")) {
						MessageBox(msg.c_str(), xml.getChildData().c_str());
					}
				}
				xml.resetCurrentChild();
				if(xml.findChild("VeryOldVersion")) {
					if(atof(xml.getChildData().c_str()) >= VERSIONFLOAT) {
						xml.resetCurrentChild();
						if(xml.findChild("URL")) {
							MessageBox(("Your version of DC++ is very old and will be shut down. Please get a new one at \r\n" + xml.getChildData()).c_str());
							oldshutdown = true;
							PostMessage(WM_CLOSE);
						}
					}
				}
			}
		}
	} catch (Exception e) {
		// ...
	}
}

LRESULT MainFrame::onGetToolTip(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMTTDISPINFO pDispInfo = (LPNMTTDISPINFO)pnmh;
	pDispInfo->szText[0] = 0;

	if((idCtrl != 0) && !(pDispInfo->uFlags & TTF_IDISHWND))
	{
		int stringId = -1;
		switch(idCtrl) {
			case ID_FILE_CONNECT: stringId = ResourceManager::MENU_FILE_PUBLIC_HUBS; break;
			case ID_FILE_RECONNECT: stringId = ResourceManager::MENU_FILE_RECONNECT; break;
			case IDC_FOLLOW: stringId = ResourceManager::MENU_FILE_FOLLOW_REDIRECT; break;
			case IDC_FAVORITES: stringId = ResourceManager::MENU_FILE_FAVORITE_HUBS; break;
			case IDC_QUEUE: stringId = ResourceManager::MENU_FILE_DOWNLOAD_QUEUE; break;
			case ID_FILE_SEARCH: stringId = ResourceManager::MENU_FILE_SEARCH; break;
			case ID_FILE_SETTINGS: stringId = ResourceManager::MENU_FILE_SETTINGS; break;
			case IDC_NOTEPAD: stringId = ResourceManager::MENU_FILE_NOTEPAD; break;
			case IDC_FILE_ADL_SEARCH: stringId = ResourceManager::MENU_FILE_ADL_SEARCH; break;
			case IDC_FINISHED: stringId = ResourceManager::FINISHED_DOWNLOADS; break; // tooltip
			case IDC_FINISHED_UL: stringId = ResourceManager::FINISHED_UPLOADS; break; // Finished Uploads tooltip
		}
		if(stringId != -1) {
			strncpy(pDispInfo->lpszText, ResourceManager::getInstance()->getString((ResourceManager::Strings)stringId).c_str(), 79);
			pDispInfo->uFlags |= TTF_DI_SETITEM;
		}
	}
	return 0;
}

void MainFrame::autoConnect(const FavoriteHubEntry::List& fl) {
	for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
		FavoriteHubEntry* entry = *i;
		if(entry->getConnect())
			HubFrame::openWindow(m_hWndMDIClient, &ctrlTab, entry->getServer(), entry->getNick(), entry->getPassword(), entry->getUserDescription());
	}
}

LRESULT MainFrame::onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		PrivateFrame::openWindow(((ItemInfo*)ctrlTransfers.GetItemData(i))->user, m_hWndMDIClient, &ctrlTab);
	}
	return 0;
}

LRESULT MainFrame::onRemoveAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		QueueManager::getInstance()->removeSources(((ItemInfo*)ctrlTransfers.GetItemData(i))->user, QueueItem::Source::FLAG_REMOVED);
	}
	return 0;
}

LRESULT MainFrame::onForce(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ctrlTransfers.SetItemText(i, COLUMN_STATUS, CSTRING(CONNECTING_FORCED));
		((ItemInfo*)ctrlTransfers.GetItemData(i))->user->connect();
	}
	return 0;
}

void MainFrame::updateTray(bool add /* = true */) {
	if(add) {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
		nid.uCallbackMessage = WM_APP + 242;
		nid.hIcon = (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		strncpy(nid.szTip, "DC++",64);
		nid.szTip[63] = '\0';
		lastMove = GET_TICK() - 1000;
		::Shell_NotifyIcon(NIM_ADD, &nid);
		trayIcon = true;
	} else {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = 0;
		::Shell_NotifyIcon(NIM_DELETE, &nid);
		ShowWindow(SW_SHOW);
		trayIcon = false;		
	}
}

/**
 * @todo Fix so that the away mode is not reset if it was set manually...
 */
LRESULT MainFrame::onSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if(wParam == SIZE_MINIMIZED) {
		if(BOOLSETTING(AUTO_AWAY)) {
			Util::setAway(true);
		}
		if(BOOLSETTING(MINIMIZE_TRAY)) {
			updateTray(true);
			ShowWindow(SW_HIDE);
		}
		maximized = IsZoomed() > 0;

	} else if( (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) ) {
		if(BOOLSETTING(AUTO_AWAY)) {
			Util::setAway(false);
		}
		if(trayIcon) {
			updateTray(false);
		}
	}
	
	bHandled = FALSE;
	return 0;
}

LRESULT MainFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		try {
			QueueManager::getInstance()->addList(((ItemInfo*)ctrlTransfers.GetItemData(i))->user);
		} catch(QueueException e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		} catch(FileException e) {
			dcdebug("MainFonGetList caught %s\n", e.getError().c_str());
		}
	}
	return 0;
}

LRESULT MainFrame::onEndSession(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(c != NULL) {
		c->removeListener(this);
		delete c;
		c = NULL;
	}

	string tmp1;
	string tmp2;
	
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(&wp);
	
	CRect rc;
	GetWindowRect(rc);
	
	if(wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWNORMAL) {
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_X, rc.left);
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_Y, rc.top);
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_X, rc.Width());
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_Y, rc.Height());
	}
	if(wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_MAXIMIZE)
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_STATE, (int)wp.showCmd);
	
	ctrlTransfers.GetColumnOrderArray(COLUMN_LAST, columnIndexes);
	for(int i = COLUMN_FIRST; i != COLUMN_LAST; i++) {
		columnSizes[i] = ctrlTransfers.GetColumnWidth(i);
		tmp1 += Util::toString(columnIndexes[i]) + ",";
		tmp2 += Util::toString(columnSizes[i]) + ",";
	}
	tmp1.erase(tmp1.size()-1, 1);
	tmp2.erase(tmp2.size()-1, 1);
	
	SettingsManager::getInstance()->set(SettingsManager::MAINFRAME_ORDER, tmp1);
	SettingsManager::getInstance()->set(SettingsManager::MAINFRAME_WIDTHS, tmp2);
	
	QueueManager::getInstance()->saveQueue();
	SettingsManager::getInstance()->save();
	
	return 0;
}

LRESULT MainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(c != NULL) {
		c->removeListener(this);
		delete c;
		c = NULL;
	}

	DWORD id;
	if(stopperThread) {
		if(WaitForSingleObject(stopperThread, 0) == WAIT_TIMEOUT) {

			// Hm, the thread's not finished stopping the client yet...post a close message and continue processing...
			Thread::yield();
			PostMessage(WM_CLOSE);
			return 0;
		}
		CloseHandle(stopperThread);
		stopperThread = NULL;
		bHandled = FALSE;
	} else {
		if( oldshutdown ||(!BOOLSETTING(CONFIRM_EXIT)) || (MessageBox(CSTRING(REALLY_EXIT), APPNAME " " VERSIONSTRING, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) ) {
			string tmp1;
			string tmp2;

			WINDOWPLACEMENT wp;
			wp.length = sizeof(wp);
			GetWindowPlacement(&wp);

			CRect rc;
			GetWindowRect(rc);
			if(wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWNORMAL) {
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_X, rc.left);
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_Y, rc.top);
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_X, rc.Width());
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_Y, rc.Height());
			}
			if(wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_MAXIMIZE)
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_STATE, (int)wp.showCmd);

			ctrlTransfers.GetColumnOrderArray(COLUMN_LAST, columnIndexes);
			for(int i = COLUMN_FIRST; i != COLUMN_LAST; i++) {
				columnSizes[i] = ctrlTransfers.GetColumnWidth(i);
				tmp1 += Util::toString(columnIndexes[i]) + ",";
				tmp2 += Util::toString(columnSizes[i]) + ",";
			}
			tmp1.erase(tmp1.size()-1, 1);
			tmp2.erase(tmp2.size()-1, 1);
			
			SettingsManager::getInstance()->set(SettingsManager::MAINFRAME_ORDER, tmp1);
			SettingsManager::getInstance()->set(SettingsManager::MAINFRAME_WIDTHS, tmp2);

			ShowWindow(SW_HIDE);
			stopperThread = CreateThread(NULL, 0, stopper, this, 0, &id);
		}
	}
	return 0;
}

LRESULT MainFrame::onLink(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	string site;

	switch(wID) {
	case IDC_HELP_README: site = Util::getAppPath() + "README.txt"; break;
	case IDC_HELP_HOMEPAGE: site = "http://dcplusplus.sourceforge.net"; break;
	case IDC_HELP_DOWNLOADS: site = "http://dcplusplus.sourceforge.net/index.php?page=download"; break;
	case IDC_HELP_FAQ: site = "http://dcplusplus.sourceforge.net/faq/faq.php?list=all&prog=1"; break;
	case IDC_HELP_HELP_FORUM: site = "http://dcplusplus.sf.net/forum"; break;
	case IDC_HELP_DISCUSS: site = "http://dcplusplus.sf.net/forum"; break;
	case IDC_HELP_REQUEST_FEATURE: site = "http://sourceforge.net/tracker/?atid=427635&group_id=40287&func=browse"; break;
	case IDC_HELP_REPORT_BUG: site = "http://sourceforge.net/tracker/?atid=427632&group_id=40287&func=browse"; break;
	case IDC_HELP_DONATE: site = "https://www.paypal.com/xclick/business=j_s%40telia.com&item_name=DC%2B%2B&no_shipping=1&cn=DC%2B%2B+forum+nick+or+greeting"; break;
	default: dcassert(0);
	}

	ShellExecute(NULL, NULL, site.c_str(), NULL, NULL, SW_SHOWNORMAL);

	return 0;
}

LRESULT MainFrame::onImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	string file = Util::getAppPath() + "queue.config";
 	if(WinUtil::browseFile(file, m_hWnd, false) == IDOK) {
		try {
			QueueManager::getInstance()->importNMQueue(file);
 		} catch(FileException e) {
			ctrlStatus.SetText(0, CSTRING(ERROR_OPENING_FILE));
 		}
 	} 
 
 	return 0;
}
 
void MainFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */)
{
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[7];
		ctrlStatus.GetClientRect(sr);
		w[6] = sr.right - 16;
#define setw(x) w[x] = max(w[x+1] - statusSizes[x], 0)
		setw(5); setw(4); setw(3); setw(2); setw(1); setw(0);

		ctrlStatus.SetParts(7, w);
	}
	CRect rc = rect;
	rc.top = rc.bottom - ctrlTab.getHeight();
	if(ctrlTab.IsWindow())
		ctrlTab.MoveWindow(rc);
	
	CRect rc2 = rect;
	rc2.bottom = rc.top;
	SetSplitterRect(rc2);
}

LRESULT MainFrame::onOpenFileList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	string file;
	if(WinUtil::browseFile(file, m_hWnd, false, Util::getAppPath() + "FileLists\\")) {
		string username;
		if(file.rfind('\\') != string::npos) {
			username = file.substr(file.rfind('\\') + 1);
			if(username.rfind('.') != string::npos) {
				username.erase(username.rfind('.'));
			}
			User::Ptr& u = ClientManager::getInstance()->getUser(username);

			DirectoryListingFrame* pChild = new DirectoryListingFrame(file, u);
			pChild->setTab(&ctrlTab);
			pChild->CreateEx(m_hWndClient);
			pChild->setWindowTitle();
		}
	}
	return 0;
}

LRESULT MainFrame::onTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (lParam == WM_LBUTTONUP) {
		ShowWindow(SW_SHOW);
		ShowWindow(maximized ? SW_MAXIMIZE : SW_RESTORE);
	} else if(lParam == WM_MOUSEMOVE && ((lastMove + 1000) < GET_TICK()) ) {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = NIF_TIP;
		strncpy(nid.szTip, ("D: " + Util::formatBytes(DownloadManager::getInstance()->getAverageSpeed()) + "/s (" + 
			Util::toString(DownloadManager::getInstance()->getDownloads()) + ")\r\nU: " +
			Util::formatBytes(UploadManager::getInstance()->getAverageSpeed()) + "/s (" + 
			Util::toString(UploadManager::getInstance()->getUploads()) + ")").c_str(), 64);
		
		::Shell_NotifyIcon(NIM_MODIFY, &nid);
		lastMove = GET_TICK();
	}
	return 0;
}

LRESULT MainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT MainFrame::onFinished(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
 	if(FinishedFrame::frame == NULL) {
 		FinishedFrame* pChild = new FinishedFrame();
 		pChild->setTab(&ctrlTab);
 		pChild->CreateEx(m_hWndClient);
 	} else {
 		MDIActivate(FinishedFrame::frame->m_hWnd);
	}
	return 0;
}

LRESULT MainFrame::onFinishedUploads(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
 	if(FinishedULFrame::frame == NULL) {
		FinishedULFrame* pChild = new FinishedULFrame();
 		pChild->setTab(&ctrlTab);
 		pChild->CreateEx(m_hWndClient);
 	} else {
 		MDIActivate(FinishedULFrame::frame->m_hWnd);
	}
	return 0;
}

LRESULT MainFrame::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	if(!BOOLSETTING(SHOW_PROGRESS_BARS)) {
		bHandled = FALSE;
		return 0;
	}

	CRect rc;
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;
	
	switch(cd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;
		
    case CDDS_ITEMPREPAINT:
		return CDRF_NOTIFYSUBITEMDRAW;
		
    case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		// Let's draw a box if needed...
		if(cd->iSubItem == COLUMN_STATUS) {
			ItemInfo* ii = (ItemInfo*)cd->nmcd.lItemlParam;
			if(ii->status == ItemInfo::STATUS_RUNNING) {
				// draw something nice...
				char buf[256];
				ctrlTransfers.GetItemText((int)cd->nmcd.dwItemSpec, COLUMN_STATUS, buf, 255);
				buf[255] = 0;

				ctrlTransfers.GetSubItemRect((int)cd->nmcd.dwItemSpec, COLUMN_STATUS, LVIR_BOUNDS, rc);
				CRect rc2 = rc;
				rc2.left += 6;
				//::Rectangle(cd->nmcd.hdc, rc.left, rc.top, rc.right, rc.bottom);
				//rc.DeflateRect(1, 1, 1, 1);
				if(ii->size == 0)
					ii->size = 1;
				rc.right = rc.left + (int) (((int64_t)rc.Width()) * ii->pos / ii->size);
				HGDIOBJ old = ::SelectObject(cd->nmcd.hdc, GetSysColorBrush(COLOR_HIGHLIGHT));
				::Rectangle(cd->nmcd.hdc, rc.left, rc.top, 
					rc.right, rc.bottom);
				::SelectObject(cd->nmcd.hdc, old);
				COLORREF oldcol = ::SetTextColor(cd->nmcd.hdc, cd->clrText);
				::DrawText(cd->nmcd.hdc, buf, strlen(buf), rc2, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
				::SetTextColor(cd->nmcd.hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
				rc2.right = rc.right;
				::DrawText(cd->nmcd.hdc, buf, strlen(buf), rc2, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
				::SelectObject(cd->nmcd.hdc, old);
				::SetTextColor(cd->nmcd.hdc, oldcol);
				return CDRF_SKIPDEFAULT;
			}
		}
		// Fall through
	default:
		return CDRF_DODEFAULT;
    }
}

LRESULT MainFrame::onCloseDisconnected(WORD , WORD , HWND , BOOL& ) {
	HubFrame::closeDisconnected();
	return 0;
}

LRESULT MainFrame::onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		UploadManager::getInstance()->reserveSlot(((ItemInfo*)ctrlTransfers.GetItemData(i))->user);
	}
	return 0;
}

LRESULT MainFrame::onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		HubManager::getInstance()->addFavoriteUser(((ItemInfo*)ctrlTransfers.GetItemData(i))->user);
	}
	return 0;
}


void MainFrame::onAction(UploadManagerListener::Types type, Upload* aUpload) throw() {
	switch(type) {
		case UploadManagerListener::COMPLETE: onUploadComplete(aUpload); break;
		case UploadManagerListener::STARTING: onUploadStarting(aUpload); break;
		default: dcassert(0);
	}
}
void MainFrame::onAction(UploadManagerListener::Types type, const Upload::List& ul) throw() {
	switch(type) {	
		case UploadManagerListener::TICK: onUploadTick(ul); break;
	}
}
void MainFrame::onAction(DownloadManagerListener::Types type, Download* aDownload) throw() {
	switch(type) {
	case DownloadManagerListener::COMPLETE: onDownloadComplete(aDownload); break;
	case DownloadManagerListener::STARTING: onDownloadStarting(aDownload); break;
	default: dcassert(0); break;
	}
}
void MainFrame::onAction(DownloadManagerListener::Types type, const Download::List& dl) throw() {
	switch(type) {	
	case DownloadManagerListener::TICK: onDownloadTick(dl); break;
	}
}
void MainFrame::onAction(DownloadManagerListener::Types type, Download* aDownload, const string& aReason) throw() {
	switch(type) {
	case DownloadManagerListener::FAILED: onDownloadFailed(aDownload, aReason); break;
	default: dcassert(0); break;
	}
}
void MainFrame::onAction(ConnectionManagerListener::Types type, ConnectionQueueItem* aCqi) throw() { 
	switch(type) {
		case ConnectionManagerListener::ADDED: onConnectionAdded(aCqi); break;
		case ConnectionManagerListener::CONNECTED: onConnectionConnected(aCqi); break;
		case ConnectionManagerListener::REMOVED: onConnectionRemoved(aCqi); break;
		case ConnectionManagerListener::STATUS_CHANGED: onConnectionStatus(aCqi); break;
		default: dcassert(0); break;
	}
};
void MainFrame::onAction(ConnectionManagerListener::Types type, ConnectionQueueItem* aCqi, const string& aLine) throw() { 
	switch(type) {
		case ConnectionManagerListener::FAILED: onConnectionFailed(aCqi, aLine); break;
		default: dcassert(0); break;
	}
}

void MainFrame::onAction(TimerManagerListener::Types type, u_int32_t aTick) throw() {
	if(type == TimerManagerListener::SECOND) {
		int64_t diff = (int64_t)((lastUpdate == 0) ? aTick - 1000 : aTick - lastUpdate);

		StringList* str = new StringList();
		str->push_back(Util::getAway() ? STRING(AWAY) : "");
		str->push_back(STRING(SLOTS) + ": " + Util::toString(SETTING(SLOTS) - UploadManager::getInstance()->getRunning()) + '/' + Util::toString(SETTING(SLOTS)));
		str->push_back("D: " + Util::formatBytes(Socket::getTotalDown()));
		str->push_back("U: " + Util::formatBytes(Socket::getTotalUp()));
		str->push_back("D: " + Util::formatBytes(Socket::getDown()*1000I64/diff) + "/s (" + Util::toString(DownloadManager::getInstance()->getDownloads()) + ")");
		str->push_back("U: " + Util::formatBytes(Socket::getUp()*1000I64/diff) + "/s (" + Util::toString(UploadManager::getInstance()->getUploads()) + ")");
		PostMessage(WM_SPEAKER, STATS, (LPARAM)str);
		SettingsManager::getInstance()->set(SettingsManager::TOTAL_UPLOAD, SETTING(TOTAL_UPLOAD) + Socket::getUp());
		SettingsManager::getInstance()->set(SettingsManager::TOTAL_DOWNLOAD, SETTING(TOTAL_DOWNLOAD) + Socket::getDown());
		lastUpdate = aTick;
		Socket::resetStats();
	}
}

// HttpConnectionListener
void MainFrame::onAction(HttpConnectionListener::Types type, HttpConnection* conn) throw() {
	switch(type) {
		case HttpConnectionListener::COMPLETE: onHttpComplete(conn); break;
	}
}
void MainFrame::onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const BYTE* buf, int len) throw() {
	switch(type) {
		case HttpConnectionListener::DATA: versionInfo += string((const char*)buf, len); break;
	}
}

void MainFrame::onAction(QueueManagerListener::Types type, QueueItem* qi) throw() {
	if(type == QueueManagerListener::FINISHED) {
		if(qi->isSet(QueueItem::FLAG_CLIENT_VIEW)) {
			// This is a file listing, show it...
			DirectoryListInfo* i = new DirectoryListInfo();
			if(qi->isSet(QueueItem::FLAG_BZLIST) ){
				dcassert(qi->getTarget().rfind('.') != string::npos);
				i->file = qi->getTarget().substr(0, qi->getTarget().rfind('.')+1) + "bz2";
			} else {
				i->file = qi->getTarget();
			}
			i->user = qi->getCurrent()->getUser();
			PostMessage(WM_SPEAKER, DOWNLOAD_LISTING, (LPARAM)i);
		}
	}
}

/**
 * @file MainFrm.cpp
 * $Id: MainFrm.cpp,v 1.20 2003/03/26 08:47:45 arnetheduck Exp $
 */

