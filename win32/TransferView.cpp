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

#include "TransferView.h"
#include "resource.h"
#include "WinUtil.h"
#include "HoldRedraw.h"

#include <dcpp/ResourceManager.h>
#include <dcpp/SettingsManager.h>
#include <dcpp/ConnectionManager.h>
#include <dcpp/DownloadManager.h>
#include <dcpp/UploadManager.h>
#include <dcpp/QueueManager.h>

int TransferView::columnIndexes[] = { COLUMN_USER, COLUMN_HUB, COLUMN_STATUS, COLUMN_TIMELEFT, COLUMN_SPEED, COLUMN_FILE, COLUMN_SIZE, COLUMN_PATH, COLUMN_IP, COLUMN_RATIO, COLUMN_CID };
int TransferView::columnSizes[] = { 150, 100, 250, 75, 75, 175, 100, 200, 50, 75, 125 };

static ResourceManager::Strings columnNames[] = { ResourceManager::USER, ResourceManager::HUB, ResourceManager::STATUS,
ResourceManager::TIME_LEFT, ResourceManager::SPEED, ResourceManager::FILENAME, ResourceManager::SIZE, ResourceManager::PATH,
ResourceManager::IP_BARE, ResourceManager::RATIO, ResourceManager::CID, };

TransferView::TransferView(SmartWin::Widget* parent) : 
	WidgetFactory<SmartWin::WidgetChildWindow>(parent),
	transfers(0)
{
	createWindow();
	
	arrows = SmartWin::ImageListPtr(new SmartWin::ImageList(16, 16, ILC_COLOR32 | ILC_MASK));
	arrows->addMultiple(SmartWin::Bitmap(IDB_ARROWS), RGB(255, 0, 255));
	{
		WidgetTransfers::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_SHAREIMAGELISTS;
		cs.exStyle = WS_EX_CLIENTEDGE;
		transfers = SmartWin::WidgetCreator<WidgetTransfers>::create(this, cs);
		transfers->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
		transfers->setFont(WinUtil::font);

		transfers->setSmallImageList(arrows);
		transfers->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		transfers->setColumnOrder(WinUtil::splitTokens(SETTING(QUEUEFRAME_ORDER), columnIndexes));
		transfers->setColumnWidths(WinUtil::splitTokens(SETTING(QUEUEFRAME_WIDTHS), columnSizes));
		transfers->setColor(WinUtil::textColor, WinUtil::bgColor);
		transfers->setSortColumn(COLUMN_USER);
	}
	
	onSized(std::tr1::bind(&TransferView::handleSized, this, _1));
	onRaw(std::tr1::bind(&TransferView::handleContextMenu, this, _1, _2), SmartWin::Message(WM_CONTEXTMENU));
	onRaw(std::tr1::bind(&TransferView::handleDestroy, this, _1, _2), SmartWin::Message(WM_DESTROY));
	onSpeaker(std::tr1::bind(&TransferView::handleSpeaker, this, _1, _2));
	
	ConnectionManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
}

TransferView::~TransferView() {

}

void TransferView::handleSized(const SmartWin::WidgetSizedEventResult& sz) {
	transfers->setBounds(SmartWin::Point(0,0), getClientAreaSize());
}

void TransferView::prepareClose() {
	ConnectionManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);
}

HRESULT TransferView::handleDestroy(WPARAM wParam, LPARAM lParam) {

	SettingsManager::getInstance()->set(SettingsManager::MAINFRAME_ORDER, WinUtil::toString(transfers->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::MAINFRAME_WIDTHS, WinUtil::toString(transfers->getColumnWidths()));

	transfers->forEach(&ItemInfo::deleteSelf);

	return 0;
}

TransferView::WidgetMenuPtr TransferView::makeContextMenu(ItemInfo* ii) {
	WidgetMenuPtr menu = createMenu(true);
	
	appendUserItems(menu);
	menu->appendSeparatorItem();
	
	menu->appendItem(IDC_FORCE, TSTRING(FORCE_ATTEMPT), std::tr1::bind(&TransferView::handleForce, this));
	if(ii->download) {
		menu->appendItem(IDC_SEARCH_ALTERNATES, CTSTRING(SEARCH_FOR_ALTERNATES), std::tr1::bind(&TransferView::handleSearchAlternates, this));
	}
	menu->appendItem(IDC_COPY_NICK, TSTRING(COPY_NICK), std::tr1::bind(&TransferView::handleCopyNick, this));
	menu->appendSeparatorItem();
	menu->appendItem(IDC_REMOVE, TSTRING(CLOSE_CONNECTION), std::tr1::bind(&TransferView::handleRemove, this));
	menu->setDefaultItem(IDC_PRIVATEMESSAGE);
	return menu;
}

HRESULT TransferView::handleContextMenu(WPARAM wParam, LPARAM lParam) {
	if (reinterpret_cast<HWND>(wParam) == transfers->handle() && transfers->hasSelection()) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if(pt.x == -1 && pt.y == -1) {
			pt = transfers->getContextMenuPos();
		}

		/// @todo Fix multiple selection menu...
		int i = -1;
		ItemInfo* ii = transfers->getSelectedItem();
#ifdef PORT_ME
		checkAdcItems(transferMenu);
#endif
		WidgetMenuPtr contextMenu = makeContextMenu(ii);
		contextMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return TRUE;
	}
	return FALSE;
}

void TransferView::handleRemove() {
	transfers->forEachSelected(&ItemInfo::disconnect);
}

void TransferView::runUserCommand(const UserCommand& uc) {
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	int i = -1;
	while((i = transfers->getNextItem(i, LVNI_SELECTED)) != -1) {
		ItemInfo* itemI = transfers->getItemData(i);
		if(!itemI->user->isOnline())
			continue;

		StringMap tmp = ucParams;
		/// @todo tmp["fileFN"] = Text::fromT(itemI->path + itemI->file);

		// compatibility with 0.674 and earlier
		ucParams["file"] = ucParams["fileFN"];

		ClientManager::getInstance()->userCommand(itemI->user, uc, tmp, true);
	}
}

void TransferView::handleForce() {
	int i = -1;
	while( (i = transfers->getNextItem(i, LVNI_SELECTED)) != -1) {
		transfers->setCellText(i, COLUMN_STATUS, TSTRING(CONNECTING_FORCED));
		ConnectionManager::getInstance()->force(transfers->getItemData(i)->user);
	}
}

void TransferView::ItemInfo::removeAll() {
	QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
}

void TransferView::handleCopyNick() {
	int i = -1;

	/// @todo Fix when more items are selected
	while( (i = transfers->getNextItem(i, LVNI_SELECTED)) != -1) {
		WinUtil::setClipboard(WinUtil::getNicks(transfers->getItemData(i)->user));
	}
}

#ifdef PORT_ME
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
				TCHAR buf[256];
				COLORREF barBase = ii->download ? SETTING(DOWNLOAD_BAR_COLOR) : SETTING(UPLOAD_BAR_COLOR);
				COLORREF bgBase = WinUtil::bgColor;
				int mod = (HLS_L(RGB2HLS(bgBase)) >= 128) ? -30 : 30;
				COLORREF barPal[3] = { HLS_TRANSFORM(barBase, -40, 50), barBase, HLS_TRANSFORM(barBase, 40, -30) };
				COLORREF bgPal[2] = { HLS_TRANSFORM(bgBase, mod, 0), HLS_TRANSFORM(bgBase, mod/2, 0) };

				transfers->GetItemText((int)cd->nmcd.dwItemSpec, COLUMN_STATUS, buf, 255);
				buf[255] = 0;

				transfers->GetSubItemRect((int)cd->nmcd.dwItemSpec, COLUMN_STATUS, LVIR_BOUNDS, rc);
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

					rc.top += 2;
					::MoveToEx(cd->nmcd.hdc,rc.left+1,rc.top,(LPPOINT)NULL);
					::LineTo(cd->nmcd.hdc,rc.right-2,rc.top);
				}

				// draw status text
				DeleteObject(::SelectObject(cd->nmcd.hdc, oldpen));
				DeleteObject(::SelectObject(cd->nmcd.hdc, oldbr));

				LONG right = rc2.right;
				left = rc2.left;
				rc2.right = rc.right;
				LONG top = rc2.top + (rc2.Height() - WinUtil::getTextHeight(cd->nmcd.hdc) - 1)/2;
				SetTextColor(cd->nmcd.hdc, RGB(255, 255, 255));
				::ExtTextOut(cd->nmcd.hdc, left, top, ETO_CLIPPED, rc2, buf, _tcslen(buf), NULL);
				//::DrawText(cd->nmcd.hdc, buf, strlen(buf), rc2, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

				rc2.left = rc2.right;
				rc2.right = right;

				SetTextColor(cd->nmcd.hdc, WinUtil::textColor);
				::ExtTextOut(cd->nmcd.hdc, left, top, ETO_CLIPPED, rc2, buf, _tcslen(buf), NULL);

				return CDRF_SKIPDEFAULT;
			}
		}
		// Fall through
	default:
		return CDRF_DODEFAULT;
	}
}

LRESULT TransferView::onDoubleClickTransfers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	if (item->iItem != -1 ) {
		transfers->getItemData(item->iItem)->pm();
	}
	return 0;
}
#endif

int TransferView::ItemInfo::compareItems(ItemInfo* a, ItemInfo* b, int col) {
	if(BOOLSETTING(ALT_SORT_ORDER)) {
		if(a->download == b->download) {
			if(a->status != b->status) {
				return (a->status == ItemInfo::STATUS_RUNNING) ? -1 : 1;
			}
		} else {
			return a->download ? -1 : 1;
		}
	} else {
		if(a->status == b->status) {
			if(a->download != b->download) {
				return a->download ? -1 : 1;
			}
		} else {
			return (a->status == ItemInfo::STATUS_RUNNING) ? -1 : 1;
		}
	}
	switch(col) {
	case COLUMN_STATUS: return 0;
	case COLUMN_TIMELEFT: return compare(a->timeLeft, b->timeLeft);
	case COLUMN_SPEED: return compare(a->speed, b->speed);
	case COLUMN_SIZE: return compare(a->size, b->size);
	case COLUMN_RATIO: return compare(a->getRatio(), b->getRatio());
	default: return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
	}
}

HRESULT TransferView::handleSpeaker(WPARAM wParam, LPARAM lParam) {
	TaskQueue::List t;
	tasks.get(t);
	
	HoldRedraw hold(transfers, t.size() > 1);
	
	for(TaskQueue::Iter i = t.begin(); i != t.end(); ++i) {
		if(i->first == ADD_ITEM) {
			auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(i->second));
			ItemInfo* ii = new ItemInfo(ui->user, ui->download);
			ii->update(*ui);
			transfers->insertItem(ii);
		} else if(i->first == REMOVE_ITEM) {
			auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(i->second));
			int ic = transfers->getRowCount();
			for(int i = 0; i < ic; ++i) {
				ItemInfo* ii = transfers->getItemData(i);
				if(*ui == *ii) {
					transfers->removeRow(i);
					delete ii;
					break;
				}
			}
		} else if(i->first == UPDATE_ITEM) {
			auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(i->second));
			int ic = transfers->getRowCount();
			for(int i = 0; i < ic; ++i) {
				ItemInfo* ii = transfers->getItemData(i);
				if(ii->download == ui->download && ii->user == ui->user) {
					ii->update(*ui);
					transfers->updateItem(i);
					break;
				}
			}
		}
	}

	if(!t.empty()) {
		transfers->resort();
	}

	return 0;
}

void TransferView::handleSearchAlternates() {
	int i = transfers->getNextItem(-1, LVNI_SELECTED);

	if(i != -1) {
		ItemInfo *ii = transfers->getItemData(i);

		string target = Text::fromT(ii->getText(COLUMN_PATH) + ii->getText(COLUMN_FILE));

		TTHValue tth;
		if(QueueManager::getInstance()->getTTH(target, tth)) {
			WinUtil::searchHash(tth);
		}
	}
}

TransferView::ItemInfo::ItemInfo(const UserPtr& u, bool aDownload) : UserInfoBase(u), download(aDownload), transferFailed(false),
	status(STATUS_WAITING), pos(0), size(0), start(0), actual(0), speed(0), timeLeft(0)
{
	columns[COLUMN_USER] = WinUtil::getNicks(u);
	columns[COLUMN_HUB] = WinUtil::getHubNames(u).first;
	columns[COLUMN_CID] = Text::toT(u->getCID().toBase32());
}

void TransferView::ItemInfo::update(const UpdateInfo& ui) {
	if(ui.updateMask & UpdateInfo::MASK_STATUS) {
		status = ui.status;
	}
	if(ui.updateMask & UpdateInfo::MASK_STATUS_STRING) {
		// No slots etc from transfermanager better than disconnected from connectionmanager
		if(!transferFailed)
			columns[COLUMN_STATUS] = ui.statusString;
		transferFailed = ui.transferFailed;
	}
	if(ui.updateMask & UpdateInfo::MASK_SIZE) {
		size = ui.size;
		columns[COLUMN_SIZE] = Text::toT(Util::formatBytes(size));
	}
	if(ui.updateMask & UpdateInfo::MASK_START) {
		start = ui.start;
	}
	if(ui.updateMask & UpdateInfo::MASK_POS) {
		pos = start + ui.pos;
	}
	if(ui.updateMask & UpdateInfo::MASK_ACTUAL) {
		actual = start + ui.actual;
		columns[COLUMN_RATIO] = Text::toT(Util::toString(getRatio()));
	}
	if(ui.updateMask & UpdateInfo::MASK_SPEED) {
		speed = ui.speed;
		if (status == STATUS_RUNNING) {
			columns[COLUMN_SPEED] = Text::toT(Util::formatBytes(speed) + "/s");
		} else {
			columns[COLUMN_SPEED] = Util::emptyStringT;
		}
	}
	if(ui.updateMask & UpdateInfo::MASK_FILE) {
		columns[COLUMN_FILE] = ui.file;
		columns[COLUMN_PATH] = ui.path;
	}
	if(ui.updateMask & UpdateInfo::MASK_TIMELEFT) {
		timeLeft = ui.timeLeft;
		if (status == STATUS_RUNNING) {
			columns[COLUMN_TIMELEFT] = Text::toT(Util::formatSeconds(timeLeft));
		} else {
			columns[COLUMN_TIMELEFT] = Util::emptyStringT;
		}
	}
	if(ui.updateMask & UpdateInfo::MASK_IP) {
		columns[COLUMN_IP] = ui.IP;
	}
}

void TransferView::on(ConnectionManagerListener::Added, ConnectionQueueItem* aCqi) throw() {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());

	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setStatusString(TSTRING(CONNECTING));

	speak(ADD_ITEM, ui);
}

void TransferView::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem* aCqi) throw() {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());

	ui->setStatusString((aCqi->getState() == ConnectionQueueItem::CONNECTING) ? TSTRING(CONNECTING) : TSTRING(WAITING_TO_RETRY));

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(ConnectionManagerListener::Removed, ConnectionQueueItem* aCqi) throw() {
	speak(REMOVE_ITEM, new UpdateInfo(aCqi->getUser(), aCqi->getDownload()));
}

void TransferView::on(ConnectionManagerListener::Failed, ConnectionQueueItem* aCqi, const string& aReason) throw() {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());
	if(aCqi->getUser()->isSet(User::OLD_CLIENT)) {
		ui->setStatusString(TSTRING(SOURCE_TOO_OLD));
	} else {
		ui->setStatusString(Text::toT(aReason));
	}
	speak(UPDATE_ITEM, ui);
}

void TransferView::on(DownloadManagerListener::Starting, Download* aDownload) throw() {
	UpdateInfo* ui = new UpdateInfo(aDownload->getUser(), true);
	ui->setStatus(ItemInfo::STATUS_RUNNING);
	ui->setPos(aDownload->getTotal());
	ui->setActual(aDownload->getActual());
	ui->setStart(aDownload->getPos());
	ui->setSize(aDownload->getSize());
	ui->setFile(Text::toT(aDownload->getTarget()));
	ui->setStatusString(TSTRING(DOWNLOAD_STARTING));
	tstring country = Text::toT(Util::getIpCountry(aDownload->getUserConnection().getRemoteIp()));
	tstring ip = Text::toT(aDownload->getUserConnection().getRemoteIp());
	if(country.empty()) {
		ui->setIP(ip);
	} else {
		ui->setIP(country + _T(" (") + ip + _T(")"));
	}
	if(aDownload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
		ui->file = _T("TTH: ") + ui->file;
	}

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(DownloadManagerListener::Tick, const DownloadList& dl) throw()  {
	AutoArray<TCHAR> buf(TSTRING(DOWNLOADED_BYTES).size() + 64);

	for(DownloadList::const_iterator j = dl.begin(); j != dl.end(); ++j) {
		Download* d = *j;

		UpdateInfo* ui = new UpdateInfo(d->getUser(), true);
		ui->setActual(d->getActual());
		ui->setPos(d->getTotal());
		ui->setTimeLeft(d->getSecondsLeft());
		ui->setSpeed(d->getRunningAverage());

		_stprintf(buf, CTSTRING(DOWNLOADED_BYTES), Text::toT(Util::formatBytes(d->getPos())).c_str(),
			(double)d->getPos()*100.0/(double)d->getSize(), Text::toT(Util::formatSeconds((GET_TICK() - d->getStart())/1000)).c_str());

		tstring statusString;

		if(d->getUserConnection().isSecure()) {
			if(d->getUserConnection().isTrusted()) {
				statusString += _T("[S]");
			} else {
				statusString += _T("[U]");
			}
		}
		if(d->isSet(Download::FLAG_TTH_CHECK)) {
			statusString += _T("[T]");
		}
		if(d->isSet(Download::FLAG_ZDOWNLOAD)) {
			statusString += _T("[Z]");
		}
		if(!statusString.empty()) {
			statusString += _T(" ");
		}
		statusString += buf;
		ui->setStatusString(statusString);

		tasks.add(UPDATE_ITEM, ui);
	}

	speak();
}

void TransferView::on(DownloadManagerListener::Failed, Download* aDownload, const string& aReason) throw() {
	UpdateInfo* ui = new UpdateInfo(aDownload->getUser(), true, true);
	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setPos(0);
	ui->setStatusString(Text::toT(aReason));
	ui->setSize(aDownload->getSize());
	ui->setFile(Text::toT(aDownload->getTarget()));
	if(aDownload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
		ui->file = _T("TTH: ") + ui->file;
	}

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(UploadManagerListener::Starting, Upload* aUpload) throw() {
	UpdateInfo* ui = new UpdateInfo(aUpload->getUser(), false);

	ui->setStatus(ItemInfo::STATUS_RUNNING);
	ui->setPos(aUpload->getTotal());
	ui->setActual(aUpload->getActual());
	ui->setStart(aUpload->getPos());
	ui->setSize(aUpload->getSize());
	ui->setFile(Text::toT(aUpload->getSourceFile()));
	ui->setStatusString(TSTRING(UPLOAD_STARTING));
	tstring country = Text::toT(Util::getIpCountry(aUpload->getUserConnection().getRemoteIp()));
	tstring ip = Text::toT(aUpload->getUserConnection().getRemoteIp());
	if(country.empty()) {
		ui->setIP(ip);
	} else {
		ui->setIP(country + _T(" (") + ip + _T(")"));
	}
	if(aUpload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
		ui->file = _T("TTH: ") + ui->file;
	}

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(UploadManagerListener::Tick, const UploadList& ul) throw() {
	AutoArray<TCHAR> buf(TSTRING(UPLOADED_BYTES).size() + 64);

	for(UploadList::const_iterator j = ul.begin(); j != ul.end(); ++j) {
		Upload* u = *j;

		UpdateInfo* ui = new UpdateInfo(u->getUser(), false);
		ui->setActual(u->getActual());
		ui->setPos(u->getTotal());
		ui->setTimeLeft(u->getSecondsLeft());
		ui->setSpeed(u->getRunningAverage());

		_stprintf(buf, CTSTRING(UPLOADED_BYTES), Text::toT(Util::formatBytes(u->getPos())).c_str(),
			(double)u->getPos()*100.0/(double)u->getSize(), Text::toT(Util::formatSeconds((GET_TICK() - u->getStart())/1000)).c_str());

		tstring statusString;

		if(u->getUserConnection().isSecure()) {
			if(u->getUserConnection().isTrusted()) {
				statusString += _T("[S]");
			} else {
				statusString += _T("[U]");
			}
		}
		if(u->isSet(Upload::FLAG_ZUPLOAD)) {
			statusString += _T("[Z]");
		}
		if(!statusString.empty()) {
			statusString += _T(" ");
		}
		statusString += buf;

		ui->setStatusString(statusString);

		tasks.add(UPDATE_ITEM, ui);
	}

	speak();
}

void TransferView::onTransferComplete(Transfer* aTransfer, bool isUpload) {
	UpdateInfo* ui = new UpdateInfo(aTransfer->getUser(), !isUpload);

	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setPos(0);
	ui->setStatusString(isUpload ? TSTRING(UPLOAD_FINISHED_IDLE) : TSTRING(DOWNLOAD_FINISHED_IDLE));

	speak(UPDATE_ITEM, ui);
}

void TransferView::ItemInfo::disconnect() {
	ConnectionManager::getInstance()->disconnect(user, download);
}
