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

#include "TransferView.h"
#include "resource.h"
#include "WinUtil.h"
#include "HoldRedraw.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/ConnectionManager.h>
#include <dcpp/DownloadManager.h>
#include <dcpp/UploadManager.h>
#include <dcpp/QueueManager.h>
#include <dcpp/ClientManager.h>
#include <dcpp/Download.h>
#include <dcpp/Upload.h>

int TransferView::columnIndexes[] = { COLUMN_USER, COLUMN_STATUS, COLUMN_SPEED, COLUMN_CHUNK, COLUMN_TRANSFERED, COLUMN_QUEUED, COLUMN_CIPHER, COLUMN_IP };
int TransferView::columnSizes[] = { 125, 375, 100, 100, 125, 75, 100, 100 };

static const char* columnNames[] = {
	N_("User"),
	N_("Status"),
	N_("Speed"),
	N_("Chunk size"),
	N_("Transfered (Ratio)"),
	N_("Queued"),
	N_("Cipher"),
	N_("IP")
};

TransferView::TransferView(SmartWin::Widget* parent, SmartWin::WidgetTabView* mdi_) : 
	WidgetFactory<SmartWin::WidgetChildWindow>(parent),
	transfers(0),
	mdi(mdi_)
{
	createWindow();
	{
		arrows = SmartWin::ImageListPtr(new SmartWin::ImageList(16, 16, ILC_COLOR32 | ILC_MASK));
		SmartWin::Bitmap tmp(IDB_ARROWS);
		arrows->add(tmp, RGB(255, 0, 255));
	}
	{
		transfers = SmartWin::WidgetCreator<WidgetTransfers>::create(this, WinUtil::Seeds::listView);

		transfers->setSmallImageList(arrows);
		transfers->createColumns(WinUtil::getStrings(columnNames));
		transfers->setColumnOrder(WinUtil::splitTokens(SETTING(MAINFRAME_ORDER), columnIndexes));
		transfers->setColumnWidths(WinUtil::splitTokens(SETTING(MAINFRAME_WIDTHS), columnSizes));
		transfers->setColor(WinUtil::textColor, WinUtil::bgColor);
		transfers->setSort(COLUMN_USER);
		transfers->onContextMenu(std::tr1::bind(&TransferView::handleContextMenu, this, _1));
		transfers->onKeyDown(std::tr1::bind(&TransferView::handleKeyDown, this, _1));
		transfers->onDblClicked(std::tr1::bind(&TransferView::handleDblClicked, this));
	}
	
	onSized(std::tr1::bind(&TransferView::handleSized, this, _1));
	onRaw(std::tr1::bind(&TransferView::handleDestroy, this, _1, _2), SmartWin::Message(WM_DESTROY));
	onSpeaker(std::tr1::bind(&TransferView::handleSpeaker, this, _1, _2));
	noEraseBackground();
	
	ConnectionManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
}

TransferView::~TransferView() {

}

bool TransferView::handleSized(const SmartWin::WidgetSizedEventResult& sz) {
	transfers->setBounds(SmartWin::Point(0,0), getClientAreaSize());
	return true;
}

void TransferView::prepareClose() {
	ConnectionManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);
}

HRESULT TransferView::handleDestroy(WPARAM wParam, LPARAM lParam) {
	SettingsManager::getInstance()->set(SettingsManager::MAINFRAME_ORDER, WinUtil::toString(transfers->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::MAINFRAME_WIDTHS, WinUtil::toString(transfers->getColumnWidths()));

	return 0;
}

TransferView::WidgetMenuPtr TransferView::makeContextMenu(ItemInfo* ii) {
	WidgetMenuPtr menu = createMenu(true);
	
	appendUserItems(mdi, menu);
	menu->appendSeparatorItem();
	
	menu->appendItem(IDC_FORCE, T_("Force attempt"), std::tr1::bind(&TransferView::handleForce, this));
	menu->appendItem(IDC_COPY_NICK, T_("Copy nick to clipboard"), std::tr1::bind(&TransferView::handleCopyNick, this));
	menu->appendSeparatorItem();
	menu->appendItem(IDC_REMOVE, T_("Disconnect"), std::tr1::bind(&TransferView::handleDisconnect, this));
	menu->setDefaultItem(IDC_PRIVATEMESSAGE);
	return menu;
}

bool TransferView::handleContextMenu(SmartWin::ScreenCoordinate pt) {
	if (transfers->hasSelection()) {
		if(pt.x() == -1 && pt.y() == -1) {
			pt = transfers->getContextMenuPos();
		}

		/// @todo Fix multiple selection menu...
		ItemInfo* ii = transfers->getSelectedData();
		WidgetMenuPtr contextMenu = makeContextMenu(ii);
		contextMenu->trackPopupMenu(this, pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return true;
	}
	return false;
}

void TransferView::handleDisconnect() {
	transfers->forEachSelected(&ItemInfo::disconnect);
}

void TransferView::runUserCommand(const UserCommand& uc) {
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	int i = -1;
	while((i = transfers->getNext(i, LVNI_SELECTED)) != -1) {
		ItemInfo* itemI = transfers->getData(i);
		if(!itemI->user->isOnline())
			continue;

		StringMap tmp = ucParams;
		/// @todo tmp["fileFN"] = Text::fromT(itemI->path + itemI->file);

		// compatibility with 0.674 and earlier
		ucParams["file"] = ucParams["fileFN"];

		ClientManager::getInstance()->userCommand(itemI->user, uc, tmp, true);
	}
}

bool TransferView::handleKeyDown(int c) {
	if(c == VK_DELETE) {
		transfers->forEachSelected(&ItemInfo::disconnect);
	}
	return true;
}

void TransferView::handleForce() {
	int i = -1;
	while( (i = transfers->getNext(i, LVNI_SELECTED)) != -1) {
		transfers->getData(i)->columns[COLUMN_STATUS] = T_("Connecting (forced)");
		transfers->update(i);
		ConnectionManager::getInstance()->force(transfers->getData(i)->user);
	}
}

void TransferView::handleCopyNick() {
	int i = -1;

	/// @todo Fix when more items are selected
	while( (i = transfers->getNext(i, LVNI_SELECTED)) != -1) {
		WinUtil::setClipboard(WinUtil::getNicks(transfers->getData(i)->user));
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
#endif

void TransferView::handleDblClicked() {
	ItemInfo* ii = transfers->getSelectedData();
	if(ii) {
		ii->pm(mdi);
	}
}

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
	case COLUMN_SPEED: return compare(a->speed, b->speed);
	case COLUMN_TRANSFERED: return compare(a->transfered, b->transfered);
	case COLUMN_QUEUED: return compare(a->queued, b->queued);
	case COLUMN_CHUNK: return compare(a->chunk, b->chunk);
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
			transfers->insert(ii);
		} else if(i->first == REMOVE_ITEM) {
			auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(i->second));
			int ic = transfers->size();
			for(int i = 0; i < ic; ++i) {
				ItemInfo* ii = transfers->getData(i);
				if(*ui == *ii) {
					transfers->erase(i);
					break;
				}
			}
		} else if(i->first == UPDATE_ITEM) {
			auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(i->second));
			int ic = transfers->size();
			for(int i = 0; i < ic; ++i) {
				ItemInfo* ii = transfers->getData(i);
				if(ii->download == ui->download && ii->user == ui->user) {
					ii->update(*ui);
					transfers->update(i);
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

TransferView::ItemInfo::ItemInfo(const UserPtr& u, bool aDownload) : 
	UserInfoBase(u), 
	download(aDownload), 
	transferFailed(false),
	status(STATUS_WAITING), 
	actual(0), 
	lastActual(0),
	transfered(0),
	lastTransfered(0),
	queued(0),
	speed(0)	
{
	columns[COLUMN_USER] = WinUtil::getNicks(u);
	columns[COLUMN_STATUS] = T_("Idle");
	columns[COLUMN_TRANSFERED] = Text::toT(Util::toString(0));
	if(aDownload) {
		queued = QueueManager::getInstance()->getQueued(u);
		columns[COLUMN_QUEUED] = Text::toT(Util::formatBytes(queued));
	}
}

void TransferView::ItemInfo::update(const UpdateInfo& ui) {
	if(ui.updateMask & UpdateInfo::MASK_STATUS) {
		lastTransfered = lastActual = 0;
		status = ui.status;
		if(download) {
			// Also update queued when status changes...
			queued = QueueManager::getInstance()->getQueued(user);
			columns[COLUMN_QUEUED] = Text::toT(Util::formatBytes(queued));
		}
	}

	if(ui.updateMask & UpdateInfo::MASK_STATUS_STRING) {
		// No slots etc from transfermanager better than disconnected from connectionmanager
		if(!transferFailed)
			columns[COLUMN_STATUS] = ui.statusString;
		transferFailed = ui.transferFailed;
	}
	
	if(ui.updateMask & UpdateInfo::MASK_TRANSFERED) {
		actual += ui.actual - lastActual;
		lastActual = ui.actual;
		transfered += ui.transfered - lastTransfered;
		lastTransfered = ui.transfered;
		if(actual == transfered) {
			columns[COLUMN_TRANSFERED] = Text::toT(Util::formatBytes(transfered));
		} else {
			columns[COLUMN_TRANSFERED] = str(TF_("%1% (%2$0.2f)") 
				% Text::toT(Util::formatBytes(transfered))
				% (static_cast<double>(actual) / transfered));
		}
	}
	
	if(ui.updateMask & UpdateInfo::MASK_CHUNK) {
		chunk = ui.chunk;
		columns[COLUMN_CHUNK] = Text::toT(Util::formatBytes(ui.chunk));
	}
	
	if(ui.updateMask & UpdateInfo::MASK_SPEED) {
		speed = ui.speed;
		if (status == STATUS_RUNNING) {
			columns[COLUMN_SPEED] = str(TF_("%1%/s") % Text::toT(Util::formatBytes(speed)));
		} else {
			columns[COLUMN_SPEED] = Util::emptyStringT;
		}
	}
	
	if(ui.updateMask & UpdateInfo::MASK_IP) {
		columns[COLUMN_IP] = ui.ip;
	}
	
	if(ui.updateMask & UpdateInfo::MASK_CIPHER) {
		columns[COLUMN_CIPHER] = ui.cipher;
	}
}

void TransferView::on(ConnectionManagerListener::Added, ConnectionQueueItem* aCqi) throw() {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());

	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setStatusString(T_("Connecting"));
	speak(ADD_ITEM, ui);
}

void TransferView::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem* aCqi) throw() {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());

	ui->setStatusString((aCqi->getState() == ConnectionQueueItem::CONNECTING) ? T_("Connecting") : T_("Waiting to retry"));

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(ConnectionManagerListener::Removed, ConnectionQueueItem* aCqi) throw() {
	speak(REMOVE_ITEM, new UpdateInfo(aCqi->getUser(), aCqi->getDownload()));
}

void TransferView::on(ConnectionManagerListener::Failed, ConnectionQueueItem* aCqi, const string& aReason) throw() {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());
	if(aCqi->getUser()->isSet(User::OLD_CLIENT)) {
		ui->setStatusString(T_("Remote client does not fully support TTH - cannot download"));
	} else {
		ui->setStatusString(Text::toT(aReason));
	}
	speak(UPDATE_ITEM, ui);
}

static tstring getFile(Transfer* t) {
	tstring file;
	
	if(t->getType() == Transfer::TYPE_TREE) {
		file = str(TF_("TTH: %1%") % Text::toT(Util::getFileName(t->getPath())));
	} else if(t->getType() == Transfer::TYPE_FULL_LIST || t->getType() == Transfer::TYPE_PARTIAL_LIST) {
		file = T_("file list");
	} else {
		file = Text::toT(Util::getFileName(t->getPath()) + 
			" (" + Util::formatBytes(t->getStartPos()) + 
			" - " + Util::formatBytes(t->getStartPos() + t->getSize()) + ")");
	}
	return file;
}

void TransferView::starting(UpdateInfo* ui, Transfer* t) {
	ui->setStatus(ItemInfo::STATUS_RUNNING);
	ui->setTransfered(t->getPos(), t->getActual());
	ui->setChunk(t->getSize());
	const UserConnection& uc = t->getUserConnection();
	ui->setCipher(Text::toT(uc.getCipherName()));
	tstring country = Text::toT(Util::getIpCountry(uc.getRemoteIp()));
	tstring ip = Text::toT(uc.getRemoteIp());
	if(country.empty()) {
		ui->setIP(ip);
	} else {
		ui->setIP(country + _T(" (") + ip + _T(")"));
	}
}

void TransferView::on(DownloadManagerListener::Starting, Download* d) throw() {
	UpdateInfo* ui = new UpdateInfo(d->getUser(), true);
	
	starting(ui, d);
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
	statusString += str(TF_("Downloading %1%") % getFile(d));
	
	ui->setStatusString(statusString);
	speak(UPDATE_ITEM, ui);
}

void TransferView::on(DownloadManagerListener::Tick, const DownloadList& dl) throw()  {
	for(DownloadList::const_iterator j = dl.begin(); j != dl.end(); ++j) {
		Download* d = *j;

		UpdateInfo* ui = new UpdateInfo(d->getUser(), true);
		ui->setTransfered(d->getPos(), d->getActual());
		ui->setSpeed(d->getAverageSpeed());

		tasks.add(UPDATE_ITEM, ui);
	}

	speak();
}

void TransferView::on(DownloadManagerListener::Failed, Download* aDownload, const string& aReason) throw() {
	UpdateInfo* ui = new UpdateInfo(aDownload->getUser(), true, true);
	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setStatusString(Text::toT(aReason));

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(UploadManagerListener::Starting, Upload* u) throw() {
	UpdateInfo* ui = new UpdateInfo(u->getUser(), false);

	starting(ui, u);

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
	statusString += str(TF_("Uploading %1%") % getFile(u));

	ui->setStatusString(statusString);

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(UploadManagerListener::Tick, const UploadList& ul) throw() {
	for(UploadList::const_iterator j = ul.begin(); j != ul.end(); ++j) {
		Upload* u = *j;

		UpdateInfo* ui = new UpdateInfo(u->getUser(), false);
		ui->setTransfered(u->getPos(), u->getActual());
		ui->setSpeed(u->getAverageSpeed());

		tasks.add(UPDATE_ITEM, ui);
	}

	speak();
}

void TransferView::on(DownloadManagerListener::Complete, Download* aDownload) throw() { 
	onTransferComplete(aDownload, false);
}

void TransferView::on(UploadManagerListener::Complete, Upload* aUpload) throw() { 
	onTransferComplete(aUpload, true); 
}

void TransferView::onTransferComplete(Transfer* aTransfer, bool isUpload) {
	UpdateInfo* ui = new UpdateInfo(aTransfer->getUser(), !isUpload);

	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setStatusString(T_("Idle"));

	speak(UPDATE_ITEM, ui);
}

void TransferView::ItemInfo::disconnect() {
	ConnectionManager::getInstance()->disconnect(user, download);
}
