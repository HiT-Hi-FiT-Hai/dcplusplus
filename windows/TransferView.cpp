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

#include "../client/QueueManager.h"
#include "../client/HubManager.h"
#include "../client/ConnectionManager.h"

#include "WinUtil.h"
#include "TransferView.h"
#include "PrivateFrame.h"

int TransferView::columnIndexes[] = { COLUMN_USER, COLUMN_STATUS, COLUMN_TIMELEFT, COLUMN_SPEED, COLUMN_FILE, COLUMN_SIZE, COLUMN_PATH };
int TransferView::columnSizes[] = { 150, 250, 75, 75, 175, 100, 200 };

static ResourceManager::Strings columnNames[] = { ResourceManager::USER, ResourceManager::STATUS,
ResourceManager::TIME_LEFT, ResourceManager::SPEED, ResourceManager::FILENAME, ResourceManager::SIZE, ResourceManager::PATH };

TransferView::~TransferView() {
	arrows.Destroy();
}

LRESULT TransferView::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

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
		int fmt = (j == COLUMN_SIZE || j == COLUMN_TIMELEFT || j == COLUMN_SPEED) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlTransfers.InsertColumn(j, CSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}

	ctrlTransfers.SetColumnOrderArray(COLUMN_LAST, columnIndexes);

	ctrlTransfers.SetBkColor(WinUtil::bgColor);
	ctrlTransfers.SetTextBkColor(WinUtil::bgColor);
	ctrlTransfers.SetTextColor(WinUtil::textColor);

	ctrlTransfers.SetImageList(arrows, LVSIL_SMALL);
	ctrlTransfers.setSort(COLUMN_STATUS, ExListViewCtrl::SORT_FUNC, true, sortStatus);

	transferMenu.CreatePopupMenu();
	transferMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	transferMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CSTRING(MATCH_QUEUE));
	transferMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(SEND_PRIVATE_MESSAGE));
	transferMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CSTRING(GRANT_EXTRA_SLOT));
	transferMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CSTRING(ADD_TO_FAVORITES));
	transferMenu.AppendMenu(MF_STRING, IDC_FORCE, CSTRING(FORCE_ATTEMPT));
	transferMenu.AppendMenu(MF_SEPARATOR, 0, (LPTSTR)NULL);
	transferMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(CLOSE_CONNECTION));
	transferMenu.AppendMenu(MF_STRING, IDC_REMOVEALL, CSTRING(REMOVE_FROM_ALL));

	ConnectionManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
#if 0
	ItemInfo* ii = new ItemInfo(ClientManager::getInstance()->getUser("test"), 
		ItemInfo::TYPE_DOWNLOAD, ItemInfo::STATUS_RUNNING, 75, 100, 25, 50);
	ctrlTransfers.insert(0, string("Test"), 0, (LPARAM)ii);
#endif
	return 0;
}

void TransferView::prepareClose() {
	WinUtil::saveHeaderOrder(ctrlTransfers, SettingsManager::MAINFRAME_ORDER, 
		SettingsManager::MAINFRAME_WIDTHS, COLUMN_LAST, columnIndexes, columnSizes);
	
	ConnectionManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);

}

LRESULT TransferView::onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	RECT rc;
	GetClientRect(&rc);
	ctrlTransfers.MoveWindow(&rc);

	return 0;
}

LRESULT TransferView::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
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

LRESULT TransferView::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		try {
			QueueManager::getInstance()->addList(((ItemInfo*)ctrlTransfers.GetItemData(i))->user, QueueItem::FLAG_CLIENT_VIEW);
		} catch(const Exception& e) {
			dcdebug("TransferView::onGetList caught %s\n", e.getError().c_str());
		}
	}
	return 0;
}

LRESULT TransferView::onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		try {
			QueueManager::getInstance()->addList(((ItemInfo*)ctrlTransfers.GetItemData(i))->user, QueueItem::FLAG_MATCH_QUEUE);
		} catch(const Exception& e) {
			dcdebug("TransferView::onGetList caught %s\n", e.getError().c_str());
		}
	}
	return 0;
}

LRESULT TransferView::onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		PrivateFrame::openWindow(((ItemInfo*)ctrlTransfers.GetItemData(i))->user);
	}
	return 0;
}

LRESULT TransferView::onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		UploadManager::getInstance()->reserveSlot(((ItemInfo*)ctrlTransfers.GetItemData(i))->user);
	}
	return 0;
}

LRESULT TransferView::onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		HubManager::getInstance()->addFavoriteUser(((ItemInfo*)ctrlTransfers.GetItemData(i))->user);
	}
	return 0;
}

LRESULT TransferView::onForce(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ctrlTransfers.SetItemText(i, COLUMN_STATUS, CSTRING(CONNECTING_FORCED));
		((ItemInfo*)ctrlTransfers.GetItemData(i))->user->connect();
	}
	return 0;
}

LRESULT TransferView::onRemoveAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		QueueManager::getInstance()->removeSources(((ItemInfo*)ctrlTransfers.GetItemData(i))->user, QueueItem::Source::FLAG_REMOVED);
	}
	return 0;
}

LRESULT TransferView::onColumnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
	if(l->iSubItem == ctrlTransfers.getSortColumn()) {
		if (!ctrlTransfers.getSortDirection())
			ctrlTransfers.setSort(-1, ctrlTransfers.getSortType());
		else
			ctrlTransfers.setSortDirection(false);
	} else {
		switch(l->iSubItem) {
		case COLUMN_TIMELEFT:
			ctrlTransfers.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, &sortTimeLeft); 
			break;
		case COLUMN_SPEED:
			ctrlTransfers.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, &sortSpeed); 
			break;
		case COLUMN_SIZE:
			ctrlTransfers.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, &sortSize); 
			break;
		case COLUMN_STATUS:
			ctrlTransfers.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, &sortStatus); 
			break;
		default:
			ctrlTransfers.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, &sortItem); 
			break;
		}
	}
	return 0;
}

int TransferView::sortSize(LPARAM a, LPARAM b) {
	int i = sortItem(a, b);
	if( i == ExListViewCtrl::SORT_STRING_NOCASE) {
		ItemInfo* c = (ItemInfo*)a;
		ItemInfo* d = (ItemInfo*)b;
		return compare(c->size, d->size);			
	}
	return i;
}
int TransferView::sortStatus(LPARAM a, LPARAM b) {
	int i = sortItem(a, b);
	return (i == ExListViewCtrl::SORT_STRING_NOCASE) ? 0 : i;
}
int TransferView::sortSpeed(LPARAM a, LPARAM b) {
	int i = sortItem(a, b);
	if( i == ExListViewCtrl::SORT_STRING_NOCASE) {
		ItemInfo* c = (ItemInfo*)a;
		ItemInfo* d = (ItemInfo*)b;
		return compare(c->speed, d->speed);			
	}
	return i;
}
int TransferView::sortTimeLeft(LPARAM a, LPARAM b) {
	int i = sortItem(a, b);
	if( i == ExListViewCtrl::SORT_STRING_NOCASE) {
		ItemInfo* c = (ItemInfo*)a;
		ItemInfo* d = (ItemInfo*)b;
		return compare(c->timeLeft, d->timeLeft);			
	}
	return i;
}

int TransferView::sortItem(LPARAM a, LPARAM b) {
	ItemInfo* c = (ItemInfo*)a;
	ItemInfo* d = (ItemInfo*)b;
	if(c->type == d->type) {
		if(c->status == d->status || d->type == ItemInfo::TYPE_UPLOAD) {
			return ExListViewCtrl::SORT_STRING_NOCASE;
		} else {
			return c->status == ItemInfo::STATUS_RUNNING ? -1 : 1;
		}
	} else {
		return c->type == ItemInfo::TYPE_DOWNLOAD ? -1 : 1;
	}
}

LRESULT TransferView::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
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
				COLORREF barBase = ::GetSysColor(COLOR_HIGHLIGHT);
				COLORREF bgBase = WinUtil::bgColor;
				int mod = (HLS_L(RGB2HLS(bgBase)) >= 128) ? -30 : 30;
				COLORREF barPal[3] = { HLS_TRANSFORM(barBase, -50, 50), barBase, HLS_TRANSFORM(barBase, 450, -30) };
				COLORREF bgPal[2] = { HLS_TRANSFORM(bgBase, mod, 0), HLS_TRANSFORM(bgBase, mod/2, 0) };

				ctrlTransfers.GetItemText((int)cd->nmcd.dwItemSpec, COLUMN_STATUS, buf, 255);
				buf[255] = 0;

				ctrlTransfers.GetSubItemRect((int)cd->nmcd.dwItemSpec, COLUMN_STATUS, LVIR_BOUNDS, rc);
				CRect rc2 = rc;
				rc2.left += 6;
				
				// draw background
				HGDIOBJ oldpen = ::SelectObject(cd->nmcd.hdc, CreatePen(PS_SOLID,0,bgPal[0]));
				HGDIOBJ oldbr = ::SelectObject(cd->nmcd.hdc, CreateSolidBrush(bgPal[1]));
				::Rectangle(cd->nmcd.hdc, rc.left, rc.top - 1, rc.right, rc.bottom);			
				rc.DeflateRect(1, 0, 1, 1);

				LONG left = rc.left;
				int64_t w = rc.Width();
				// draw start part
				if(ii->size == 0)
					ii->size = 1;
				rc.right = left + (int) (w * ii->start / ii->size);
				DeleteObject(SelectObject(cd->nmcd.hdc, CreateSolidBrush(barPal[0])));
				DeleteObject(SelectObject(cd->nmcd.hdc, CreatePen(PS_SOLID,0,barPal[0])));
				
				::Rectangle(cd->nmcd.hdc, rc.left, rc.top, rc.right, rc.bottom);
				
				// Draw actual part
				rc.left = rc.right;
				rc.right = left + (int) (w * ii->actual / ii->size);
				DeleteObject(SelectObject(cd->nmcd.hdc, CreateSolidBrush(barPal[1])));

				::Rectangle(cd->nmcd.hdc, rc.left, rc.top, rc.right, rc.bottom);

				// And the effective part...
				if(ii->pos > ii->actual) {
					rc.left = rc.right - 1;
					rc.right = left + (int) (w * ii->pos / ii->size);
					DeleteObject(SelectObject(cd->nmcd.hdc, CreateSolidBrush(barPal[2])));

					::Rectangle(cd->nmcd.hdc, rc.left, rc.top, rc.right, rc.bottom);

				}
				rc.left = left;
				// draw progressbar highlight
				if(rc.Width()>2) {
					DeleteObject(SelectObject(cd->nmcd.hdc, CreatePen(PS_SOLID,1,barPal[2])));

					rc.top += (int)rc.Height()*0.33;
					::MoveToEx(cd->nmcd.hdc,rc.left+1,rc.top,(LPPOINT)NULL);
					::LineTo(cd->nmcd.hdc,rc.right-2,rc.top);
				};
				
				// draw status text
				DeleteObject(::SelectObject(cd->nmcd.hdc, oldpen));
				DeleteObject(::SelectObject(cd->nmcd.hdc, oldbr));

				LONG right = rc2.right;
				left = rc2.left;
				rc2.right = rc.right;
				LONG top = rc2.top + (rc2.Height() - WinUtil::getTextHeight(cd->nmcd.hdc) - 1)/2;
				SetTextColor(cd->nmcd.hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
				::ExtTextOut(cd->nmcd.hdc, left, top, ETO_CLIPPED, rc2, buf, strlen(buf), NULL);
				//::DrawText(cd->nmcd.hdc, buf, strlen(buf), rc2, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

				rc2.left = rc2.right;
				rc2.right = right;

				SetTextColor(cd->nmcd.hdc, WinUtil::textColor);
				::ExtTextOut(cd->nmcd.hdc, left, top, ETO_CLIPPED, rc2, buf, strlen(buf), NULL);

				return CDRF_SKIPDEFAULT;
			}
		}
		// Fall through
	default:
		return CDRF_DODEFAULT;
	}
}

LRESULT TransferView::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
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
			if(ctrlTransfers.getSortColumn() != COLUMN_USER)
				ctrlTransfers.resort();
			ctrlTransfers.SetRedraw(TRUE);
		}


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

		if(ctrlTransfers.getSortColumn() != COLUMN_USER && ctrlTransfers.getSortColumn() != COLUMN_STATUS)
			ctrlTransfers.resort();
		ctrlTransfers.SetRedraw(TRUE);
		
		delete v;
	}

	return 0;
}

void TransferView::onConnectionAdded(ConnectionQueueItem* aCqi) {
	ItemInfo::Types t = aCqi->getConnection() && aCqi->getConnection()->isSet(UserConnection::FLAG_UPLOAD) ? ItemInfo::TYPE_UPLOAD : ItemInfo::TYPE_DOWNLOAD;
	ItemInfo* ii = new ItemInfo(aCqi->getUser(), t, ItemInfo::STATUS_WAITING);

	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) == transferItems.end());
		transferItems.insert(make_pair(aCqi, ii));
	}
	StringListInfo* i = new StringListInfo((LPARAM)ii);
	i->columns[COLUMN_USER] = aCqi->getUser()->getFullNick();
	i->columns[COLUMN_STATUS] = STRING(CONNECTING);

	if(aCqi->getConnection() && aCqi->getConnection()->isSet(UserConnection::FLAG_UPLOAD)) {
		PostMessage(WM_SPEAKER, ADD_UPLOAD_ITEM, (LPARAM)i);
	} else {
		PostMessage(WM_SPEAKER, ADD_DOWNLOAD_ITEM, (LPARAM)i);
	}
}

void TransferView::onConnectionStatus(ConnectionQueueItem* aCqi) {
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

void TransferView::onConnectionRemoved(ConnectionQueueItem* aCqi) {
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

void TransferView::onConnectionFailed(ConnectionQueueItem* aCqi, const string& aReason) {
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

void TransferView::onDownloadStarting(Download* aDownload) {
	ConnectionQueueItem* aCqi = aDownload->getUserConnection()->getCQI();
	ItemInfo* ii;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		ii = transferItems[aCqi];		
	}
	ii->status = ItemInfo::STATUS_RUNNING;
	ii->pos = 0;
	ii->start = aDownload->getPos();
	ii->actual = ii->start;
	ii->size = aDownload->getSize();

	StringListInfo* i = new StringListInfo((LPARAM)ii);
	i->columns[COLUMN_FILE] = Util::getFileName(aDownload->getTarget());
	i->columns[COLUMN_PATH] = Util::getFilePath(aDownload->getTarget());
	i->columns[COLUMN_STATUS] = STRING(DOWNLOAD_STARTING);
	i->columns[COLUMN_SIZE] = Util::formatBytes(aDownload->getSize());

	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void TransferView::onDownloadTick(const Download::List& dl) {
	vector<StringListInfo*>* v = new vector<StringListInfo*>();
	v->reserve(dl.size());

	char* buf = new char[STRING(DOWNLOADED_BYTES).size() + 64];

	{
		Lock l(cs);
		for(Download::List::const_iterator j = dl.begin(); j != dl.end(); ++j) {
			Download* d = *j;

			sprintf(buf, CSTRING(DOWNLOADED_BYTES), Util::formatBytes(d->getPos()).c_str(), 
				(double)d->getPos()*100.0/(double)d->getSize(), Util::formatSeconds((GET_TICK() - d->getStart())/1000).c_str());

			ConnectionQueueItem* aCqi = d->getUserConnection()->getCQI();
			ItemInfo* ii = transferItems[aCqi];
			ii->actual = ii->start + d->getActual();
			ii->pos = ii->start + d->getTotal();
			ii->timeLeft = d->getSecondsLeft();
			ii->speed = d->getRunningAverage();

			StringListInfo* i = new StringListInfo((LPARAM)ii);
			i->columns[COLUMN_STATUS] = buf;
			i->columns[COLUMN_TIMELEFT] = Util::formatSeconds(d->getSecondsLeft());
			i->columns[COLUMN_SPEED] = Util::formatBytes(d->getRunningAverage()) + "/s";

			v->push_back(i);
		}
	}
	delete[] buf;

	PostMessage(WM_SPEAKER, SET_TEXTS, (LPARAM)v);
}

void TransferView::onDownloadFailed(Download* aDownload, const string& aReason) {
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

void TransferView::onUploadStarting(Upload* aUpload) {
	ConnectionQueueItem* aCqi = aUpload->getUserConnection()->getCQI();
	ItemInfo* ii;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		ii = transferItems[aCqi];		
	}
	ii->pos = 0;
	ii->start = aUpload->getPos();
	ii->actual = ii->start;
	ii->size = aUpload->getSize();
	ii->status = ItemInfo::STATUS_RUNNING;
	ii->speed = 0;
	ii->timeLeft = 0;

	StringListInfo* i = new StringListInfo((LPARAM)ii);

	i->columns[COLUMN_FILE] = Util::getFileName(aUpload->getFileName());
	i->columns[COLUMN_PATH] = Util::getFilePath(aUpload->getFileName());
	i->columns[COLUMN_STATUS] = STRING(UPLOAD_STARTING);
	i->columns[COLUMN_SIZE] = Util::formatBytes(aUpload->getSize());

	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void TransferView::onUploadTick(const Upload::List& ul) {
	vector<StringListInfo*>* v = new vector<StringListInfo*>();
	v->reserve(ul.size());

	char* buf = new char[STRING(UPLOADED_BYTES).size() + 64];

	{
		Lock l(cs);
		for(Upload::List::const_iterator j = ul.begin(); j != ul.end(); ++j) {
			Upload* u = *j;

			ConnectionQueueItem* aCqi = u->getUserConnection()->getCQI();
			ItemInfo* ii = transferItems[aCqi];	
			ii->actual = ii->start + u->getActual();
			ii->pos = ii->start + u->getTotal();
			ii->timeLeft = u->getSecondsLeft();
			ii->speed = u->getRunningAverage();

			sprintf(buf, CSTRING(UPLOADED_BYTES), Util::formatBytes(u->getPos()).c_str(), 
				(double)u->getPos()*100.0/(double)u->getSize(), Util::formatSeconds((GET_TICK() - u->getStart())/1000).c_str());

			StringListInfo* i = new StringListInfo((LPARAM)ii);
			i->columns[COLUMN_STATUS] = buf;
			i->columns[COLUMN_TIMELEFT] = Util::formatSeconds(u->getSecondsLeft());
			i->columns[COLUMN_SPEED] = Util::formatBytes(u->getRunningAverage()) + "/s";

			v->push_back(i);
		}
	}

	delete[] buf;

	PostMessage(WM_SPEAKER, SET_TEXTS, (LPARAM)v);
}

void TransferView::onTransferComplete(Transfer* aTransfer, bool isUpload) {
	ConnectionQueueItem* aCqi = aTransfer->getUserConnection()->getCQI();
	ItemInfo* ii;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		ii = transferItems[aCqi];		
	}
	ii->status = ItemInfo::STATUS_WAITING;
	ii->pos = 0;

	StringListInfo* i = new StringListInfo((LPARAM)ii);
	i->columns[COLUMN_STATUS] = isUpload ? STRING(UPLOAD_FINISHED_IDLE) : STRING(DOWNLOAD_FINISHED_IDLE);
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);	
}

void TransferView::removeSelected() {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ItemInfo* ii = (ItemInfo*)ctrlTransfers.GetItemData(i);
		ConnectionManager::getInstance()->removeConnection(ii->user, ii->type == ItemInfo::TYPE_DOWNLOAD);
	}
}

void TransferView::onAction(ConnectionManagerListener::Types type, ConnectionQueueItem* aCqi) throw() { 
	switch(type) {
		case ConnectionManagerListener::ADDED: onConnectionAdded(aCqi); break;
		case ConnectionManagerListener::CONNECTED: onConnectionConnected(aCqi); break;
		case ConnectionManagerListener::REMOVED: onConnectionRemoved(aCqi); break;
		case ConnectionManagerListener::STATUS_CHANGED: onConnectionStatus(aCqi); break;
		default: dcassert(0); break;
	}
};
void TransferView::onAction(ConnectionManagerListener::Types type, ConnectionQueueItem* aCqi, const string& aLine) throw() { 
	switch(type) {
		case ConnectionManagerListener::FAILED: onConnectionFailed(aCqi, aLine); break;
		default: dcassert(0); break;
	}
}

void TransferView::onAction(DownloadManagerListener::Types type, Download* aDownload) throw() {
	switch(type) {
	case DownloadManagerListener::COMPLETE: onTransferComplete(aDownload, false); break;
	case DownloadManagerListener::STARTING: onDownloadStarting(aDownload); break;
	default: dcassert(0); break;
	}
}
void TransferView::onAction(DownloadManagerListener::Types type, const Download::List& dl) throw() {
	switch(type) {	
	case DownloadManagerListener::TICK: onDownloadTick(dl); break;
	}
}
void TransferView::onAction(DownloadManagerListener::Types type, Download* aDownload, const string& aReason) throw() {
	switch(type) {
	case DownloadManagerListener::FAILED: onDownloadFailed(aDownload, aReason); break;
	default: dcassert(0); break;
	}
}

void TransferView::onAction(UploadManagerListener::Types type, Upload* aUpload) throw() {
	switch(type) {
		case UploadManagerListener::COMPLETE: onTransferComplete(aUpload, true); break;
		case UploadManagerListener::STARTING: onUploadStarting(aUpload); break;
		default: dcassert(0);
	}
}
void TransferView::onAction(UploadManagerListener::Types type, const Upload::List& ul) throw() {
	switch(type) {	
		case UploadManagerListener::TICK: onUploadTick(ul); break;
	}
}

/**
 * @file
 * $Id: TransferView.cpp,v 1.10 2003/11/07 16:38:22 arnetheduck Exp $
 */