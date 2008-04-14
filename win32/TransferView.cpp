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

int TransferView::connectionIndexes[] = { CONNECTION_COLUMN_USER, CONNECTION_COLUMN_HUB, CONNECTION_COLUMN_STATUS, CONNECTION_COLUMN_SPEED, CONNECTION_COLUMN_CHUNK, CONNECTION_COLUMN_TRANSFERED, CONNECTION_COLUMN_QUEUED, CONNECTION_COLUMN_CIPHER, CONNECTION_COLUMN_IP };
int TransferView::connectionSizes[] = { 125, 100, 375, 100, 125, 125, 75, 100, 100 };

int TransferView::downloadIndexes[] = { DOWNLOAD_COLUMN_FILE, DOWNLOAD_COLUMN_PATH, DOWNLOAD_COLUMN_STATUS, DOWNLOAD_COLUMN_TIMELEFT, DOWNLOAD_COLUMN_SPEED, DOWNLOAD_COLUMN_DONE, DOWNLOAD_COLUMN_SIZE };
int TransferView::downloadSizes[] = { 200, 300, 150, 200, 125, 100, 100 };

static const char* connectionNames[] = {
	N_("User"),
	N_("Hub"),
	N_("Status"),
	N_("Speed"),
	N_("Chunk"),
	N_("Transferred (Ratio)"),
	N_("Queued"),
	N_("Cipher"),
	N_("IP")
};

static const char* downloadNames[] = {
	N_("Filename"),
	N_("Path"),
	N_("Status"),
	N_("Time left"),
	N_("Speed"),
	N_("Done"),
	N_("Size")
};

TransferView::TransferView(dwt::Widget* parent, dwt::TabView* mdi_) : 
	WidgetFactory<dwt::Container>(parent),
	connections(0),
	connectionsWindow(0),
	downloads(0),
	downloadsWindow(0),
	mdi(mdi_)
{
	create();
	
	{
		TabSheet::Seed tcs;
		tcs.style |= TCS_HOTTRACK | TCS_RAGGEDRIGHT | TCS_FOCUSNEVER;
		tabs = addChild(tcs);
		tabs->onSelectionChanged(std::tr1::bind(&TransferView::handleTabSelected, this));
	}
	
	{
		Container::Seed cs;
		cs.caption = T_("Connections");
		cs.background = (HBRUSH)(COLOR_3DFACE + 1);
		cs.location = tabs->getUsableArea(true);
		connectionsWindow = dwt::WidgetCreator<Container>::create(tabs, cs);
		connectionsWindow->setHelpId(IDH_CONNECTIONS);
		tabs->addPage(T_("Connections"), 0);

		cs.style &= ~WS_VISIBLE;
		cs.caption = T_("Downloads");
		downloadsWindow = dwt::WidgetCreator<Container>::create(tabs, cs);
		downloadsWindow->setHelpId(IDH_DOWNLOADS);
		tabs->addPage(T_("Downloads"), 1);
	}
	
	{
		arrows = dwt::ImageListPtr(new dwt::ImageList(16, 16, ILC_COLOR32 | ILC_MASK));
		dwt::Bitmap tmp(IDB_ARROWS);
		arrows->add(tmp, RGB(255, 0, 255));
	}
	{
		connections = connectionsWindow->addChild(WidgetConnections::Seed());

		connections->setSmallImageList(arrows);
		connections->createColumns(WinUtil::getStrings(connectionNames));
		connections->setColumnOrder(WinUtil::splitTokens(SETTING(CONNECTIONS_ORDER), connectionIndexes));
		connections->setColumnWidths(WinUtil::splitTokens(SETTING(CONNECTIONS_WIDTHS), connectionSizes));
		connections->setColor(WinUtil::textColor, WinUtil::bgColor);
		connections->setSort(CONNECTION_COLUMN_USER);
		connections->onContextMenu(std::tr1::bind(&TransferView::handleConnectionsMenu, this, _1));
		connections->onKeyDown(std::tr1::bind(&TransferView::handleKeyDown, this, _1));
		connections->onDblClicked(std::tr1::bind(&TransferView::handleDblClicked, this));
	}
	
	{
		downloads = downloadsWindow->addChild(WidgetDownloads::Seed());

		downloads->createColumns(WinUtil::getStrings(downloadNames));
		downloads->setColumnOrder(WinUtil::splitTokens(SETTING(DOWNLOADS_ORDER), downloadIndexes));
		downloads->setColumnWidths(WinUtil::splitTokens(SETTING(DOWNLOADS_WIDTHS), downloadSizes));
		downloads->setSort(DOWNLOAD_COLUMN_STATUS);
		downloads->setColor(WinUtil::textColor, WinUtil::bgColor);
		downloads->setSmallImageList(WinUtil::fileImages);

		downloads->onContextMenu(std::tr1::bind(&TransferView::handleDownloadsMenu, this, _1));
	}

	onSized(std::tr1::bind(&TransferView::handleSized, this, _1));
	onRaw(std::tr1::bind(&TransferView::handleDestroy, this, _1, _2), dwt::Message(WM_DESTROY));
	onSpeaker(std::tr1::bind(&TransferView::handleSpeaker, this, _1, _2));
	noEraseBackground();
	
	layout();
	
	ConnectionManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
	QueueManager::getInstance()->addListener(this);
}

TransferView::~TransferView() {

}

void TransferView::handleTabSelected() {
	int i = tabs->getSelected();
	
	if(i == 0) {
		::ShowWindow(downloadsWindow->handle(), SW_HIDE);
		::ShowWindow(connectionsWindow->handle(), SW_SHOW);
	} else {
		::ShowWindow(connectionsWindow->handle(), SW_HIDE);
		::ShowWindow(downloadsWindow->handle(), SW_SHOW);
	}
}

void TransferView::handleSized(const dwt::SizedEvent& sz) {
	layout();
}

void TransferView::layout() {
	tabs->setBounds(dwt::Point(0,0), getClientAreaSize());
	dwt::Rectangle rect = tabs->getUsableArea(true);

	connectionsWindow->setBounds(rect);
	connections->setBounds(dwt::Rectangle(connectionsWindow->getClientAreaSize()));
	downloadsWindow->setBounds(rect);
	downloads->setBounds(dwt::Rectangle(downloadsWindow->getClientAreaSize()));
}

void TransferView::prepareClose() {
	QueueManager::getInstance()->removeListener(this);
	ConnectionManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);
}

HRESULT TransferView::handleDestroy(WPARAM wParam, LPARAM lParam) {
	SettingsManager::getInstance()->set(SettingsManager::CONNECTIONS_ORDER, WinUtil::toString(connections->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::CONNECTIONS_WIDTHS, WinUtil::toString(connections->getColumnWidths()));

	SettingsManager::getInstance()->set(SettingsManager::DOWNLOADS_ORDER, WinUtil::toString(downloads->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::DOWNLOADS_WIDTHS, WinUtil::toString(downloads->getColumnWidths()));
	
	return 0;
}

TransferView::WidgetMenuPtr TransferView::makeContextMenu(ConnectionInfo* ii) {
	WidgetMenuPtr menu = createMenu(WinUtil::Seeds::menu);
	
	appendUserItems(mdi, menu);
	menu->appendSeparatorItem();
	
	menu->appendItem(IDC_FORCE, T_("&Force attempt"), std::tr1::bind(&TransferView::handleForce, this));
	menu->appendItem(IDC_COPY_NICK, T_("Copy &nick to clipboard"), std::tr1::bind(&TransferView::handleCopyNick, this));
	menu->appendSeparatorItem();
	menu->appendItem(IDC_REMOVE, T_("&Disconnect"), std::tr1::bind(&TransferView::handleDisconnect, this));
	menu->setDefaultItem(IDC_PRIVATEMESSAGE);
	return menu;
}

bool TransferView::handleConnectionsMenu(dwt::ScreenCoordinate pt) {
	if (connections->hasSelected()) {
		if(pt.x() == -1 && pt.y() == -1) {
			pt = connections->getContextMenuPos();
		}

		/// @todo Fix multiple selection menu...
		ConnectionInfo* ii = connections->getSelectedData();
		WidgetMenuPtr contextMenu = makeContextMenu(ii);
		contextMenu->trackPopupMenu(pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return true;
	}
	return false;
}

bool TransferView::handleDownloadsMenu(dwt::ScreenCoordinate pt) {
	if (downloads->countSelected() == 1) {
		if(pt.x() == -1 && pt.y() == -1) {
			pt = downloads->getContextMenuPos();
		}

		WidgetMenuPtr menu = createMenu(WinUtil::Seeds::menu);
		DownloadInfo* di = downloads->getSelectedData();
		WinUtil::addHashItems(menu, di->tth, di->columns[DOWNLOAD_COLUMN_FILE]);
		menu->trackPopupMenu(pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return true;
	}
	return false;
}

int TransferView::find(const string& path) {
	for(size_t i = 0; i < downloads->size(); ++i) {
		DownloadInfo* di = downloads->getData(i);
		if(Util::stricmp(di->path, path) == 0) {
			return i;
		}
	}
	return -1;
}


void TransferView::handleDisconnect() {
	connections->forEachSelected(&ConnectionInfo::disconnect);
}

void TransferView::runUserCommand(const UserCommand& uc) {
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	int i = -1;
	while((i = connections->getNext(i, LVNI_SELECTED)) != -1) {
		ConnectionInfo* itemI = connections->getData(i);
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
		connections->forEachSelected(&ConnectionInfo::disconnect);
	}
	return true;
}

void TransferView::handleForce() {
	int i = -1;
	while( (i = connections->getNext(i, LVNI_SELECTED)) != -1) {
		connections->getData(i)->columns[CONNECTION_COLUMN_STATUS] = T_("Connecting (forced)");
		connections->update(i);
		ConnectionManager::getInstance()->force(connections->getData(i)->user);
	}
}

void TransferView::handleCopyNick() {
	int i = -1;

	/// @todo Fix when more items are selected
	while( (i = connections->getNext(i, LVNI_SELECTED)) != -1) {
		WinUtil::setClipboard(WinUtil::getNicks(connections->getData(i)->user));
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
			ConnectionInfo* ii = (ConnectionInfo*)cd->nmcd.lItemlParam;
			if(ii->status == ConnectionInfo::STATUS_RUNNING) {
				// draw something nice...
				TCHAR buf[256];
				COLORREF barBase = ii->download ? SETTING(DOWNLOAD_BAR_COLOR) : SETTING(UPLOAD_BAR_COLOR);
				COLORREF bgBase = WinUtil::bgColor;
				int mod = (HLS_L(RGB2HLS(bgBase)) >= 128) ? -30 : 30;
				COLORREF barPal[3] = { HLS_TRANSFORM(barBase, -40, 50), barBase, HLS_TRANSFORM(barBase, 40, -30) };
				COLORREF bgPal[2] = { HLS_TRANSFORM(bgBase, mod, 0), HLS_TRANSFORM(bgBase, mod/2, 0) };

				connections->GetItemText((int)cd->nmcd.dwItemSpec, COLUMN_STATUS, buf, 255);
				buf[255] = 0;

				connections->GetSubItemRect((int)cd->nmcd.dwItemSpec, COLUMN_STATUS, LVIR_BOUNDS, rc);
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
	ConnectionInfo* ii = connections->getSelectedData();
	if(ii) {
		ii->pm(mdi);
	}
}

int TransferView::ConnectionInfo::compareItems(ConnectionInfo* a, ConnectionInfo* b, int col) {
	if(BOOLSETTING(ALT_SORT_ORDER)) {
		if(a->download == b->download) {
			if(a->status != b->status) {
				return (a->status == ConnectionInfo::STATUS_RUNNING) ? -1 : 1;
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
			return (a->status == ConnectionInfo::STATUS_RUNNING) ? -1 : 1;
		}
	}
	switch(col) {
	case CONNECTION_COLUMN_STATUS: return 0;
	case CONNECTION_COLUMN_SPEED: return compare(a->speed, b->speed);
	case CONNECTION_COLUMN_TRANSFERED: return compare(a->transfered, b->transfered);
	case CONNECTION_COLUMN_QUEUED: return compare(a->queued, b->queued);
	case CONNECTION_COLUMN_CHUNK: return compare(a->chunk, b->chunk);
	default: return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
	}
}

HRESULT TransferView::handleSpeaker(WPARAM wParam, LPARAM lParam) {
	TaskQueue::List t;
	tasks.get(t);
	
	HoldRedraw hold(connections, t.size() > 1);
	
	for(TaskQueue::Iter i = t.begin(); i != t.end(); ++i) {
		if(i->first == CONNECTIONS_ADD) {
			boost::scoped_ptr<UpdateInfo> ui(static_cast<UpdateInfo*>(i->second));
			ConnectionInfo* ii = new ConnectionInfo(ui->user, ui->download);
			ii->update(*ui);
			connections->insert(ii);
		} else if(i->first == CONNECTIONS_REMOVE) {
			auto_ptr<UpdateInfo> ui(static_cast<UpdateInfo*>(i->second));
			int ic = connections->size();
			for(int i = 0; i < ic; ++i) {
				ConnectionInfo* ii = connections->getData(i);
				if(*ui == *ii) {
					connections->erase(i);
					break;
				}
			}
		} else if(i->first == CONNECTIONS_UPDATE) {
			boost::scoped_ptr<UpdateInfo> ui(static_cast<UpdateInfo*>(i->second));
			int ic = connections->size();
			for(int i = 0; i < ic; ++i) {
				ConnectionInfo* ii = connections->getData(i);
				if(ii->download == ui->download && ii->user == ui->user) {
					ii->update(*ui);
					connections->update(i);
					break;
				}
			}
		} else if(i->first == DOWNLOADS_ADD_USER) {
			boost::scoped_ptr<TickInfo> ti(static_cast<TickInfo*>(i->second));
			int i = find(ti->path);
			if(i == -1) {
				int64_t size = QueueManager::getInstance()->getSize(ti->path);
				TTHValue tth;
				if(size != -1 && QueueManager::getInstance()->getTTH(ti->path, tth)) {
					i = downloads->insert(new DownloadInfo(ti->path, size, tth));
				}
			} else {
				downloads->getData(i)->users++;
				downloads->update(i);
			}
		} else if(i->first == DOWNLOADS_TICK) {
			boost::scoped_ptr<TickInfo> ti(static_cast<TickInfo*>(i->second));
			int i = find(ti->path);
			if(i != -1) {
				DownloadInfo* di = downloads->getData(i);
				di->update(*ti);
				downloads->update(i);
			}
		} else if(i->first == DOWNLOADS_REMOVE_USER) {
			boost::scoped_ptr<TickInfo> ti(static_cast<TickInfo*>(i->second));
			int i = find(ti->path);
			
			if(i != -1) {
				DownloadInfo* di = downloads->getData(i);
				if(--di->users == 0) {
					di->bps = 0;
				}
				di->update();
				downloads->update(i);
			}
		} else if(i->first == DOWNLOADS_REMOVED) {
			boost::scoped_ptr<TickInfo> ti(static_cast<TickInfo*>(i->second));
			int i = find(ti->path);
			if(i != -1) {
				downloads->erase(i);
			}
		}
	
	}

	if(!t.empty()) {
		connections->resort();
	}

	return 0;
}

TransferView::ConnectionInfo::ConnectionInfo(const UserPtr& u, bool aDownload) : 
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
	columns[CONNECTION_COLUMN_USER] = WinUtil::getNicks(u);
	columns[CONNECTION_COLUMN_HUB] = WinUtil::getHubNames(u).first;
	columns[CONNECTION_COLUMN_STATUS] = T_("Idle");
	columns[CONNECTION_COLUMN_TRANSFERED] = Text::toT(Util::toString(0));
	if(aDownload) {
		queued = QueueManager::getInstance()->getQueued(u);
		columns[CONNECTION_COLUMN_QUEUED] = Text::toT(Util::formatBytes(queued));
	}
}

void TransferView::ConnectionInfo::update(const UpdateInfo& ui) {
	if(ui.updateMask & UpdateInfo::MASK_STATUS) {
		lastTransfered = lastActual = 0;
		status = ui.status;
		if(download) {
			// Also update queued when status changes...
			queued = QueueManager::getInstance()->getQueued(user);
			columns[CONNECTION_COLUMN_QUEUED] = Text::toT(Util::formatBytes(queued));
		}
	}

	if(ui.updateMask & UpdateInfo::MASK_STATUS_STRING) {
		// No slots etc from transfermanager better than disconnected from connectionmanager
		if(!transferFailed)
			columns[CONNECTION_COLUMN_STATUS] = ui.statusString;
		transferFailed = ui.transferFailed;
	}
	
	if(ui.updateMask & UpdateInfo::MASK_TRANSFERED) {
		actual += ui.actual - lastActual;
		lastActual = ui.actual;
		transfered += ui.transfered - lastTransfered;
		lastTransfered = ui.transfered;
		if(actual == transfered) {
			columns[CONNECTION_COLUMN_TRANSFERED] = Text::toT(Util::formatBytes(transfered));
		} else {
			columns[CONNECTION_COLUMN_TRANSFERED] = str(TF_("%1% (%2$0.2f)") 
				% Text::toT(Util::formatBytes(transfered))
				% (static_cast<double>(actual) / transfered));
		}
	}
	
	if(ui.updateMask & UpdateInfo::MASK_CHUNK) {
		chunkPos = ui.chunkPos;
		chunk = ui.chunk;
		columns[CONNECTION_COLUMN_CHUNK] = Text::toT(Util::formatBytes(chunkPos) + "/" + Util::formatBytes(chunk));
	}
	
	if(ui.updateMask & UpdateInfo::MASK_SPEED) {
		speed = ui.speed;
		if (status == STATUS_RUNNING) {
			columns[CONNECTION_COLUMN_SPEED] = str(TF_("%1%/s") % Text::toT(Util::formatBytes(speed)));
		} else {
			columns[CONNECTION_COLUMN_SPEED] = Util::emptyStringT;
		}
	}
	
	if(ui.updateMask & UpdateInfo::MASK_IP) {
		columns[CONNECTION_COLUMN_IP] = ui.ip;
	}
	
	if(ui.updateMask & UpdateInfo::MASK_CIPHER) {
		columns[CONNECTION_COLUMN_CIPHER] = ui.cipher;
	}
}

TransferView::DownloadInfo::DownloadInfo(const string& target, int64_t size_, const TTHValue& tth_) : 
	path(target), 
	done(QueueManager::getInstance()->getPos(target)), 
	size(size_), 
	users(1),
	tth(tth_)
{
	columns[DOWNLOAD_COLUMN_FILE] = Text::toT(Util::getFileName(target));
	columns[DOWNLOAD_COLUMN_PATH] = Text::toT(Util::getFilePath(target));
	columns[DOWNLOAD_COLUMN_SIZE] = Text::toT(Util::formatBytes(size));
	
	update();
}

int TransferView::DownloadInfo::getImage() const {
	return WinUtil::getIconIndex(Text::toT(path));
}

void TransferView::DownloadInfo::update(const TransferView::TickInfo& ti) {
	done = ti.done + QueueManager::getInstance()->getInstance()->getPos(ti.path);
	bps = ti.bps;
	update();
}

void TransferView::DownloadInfo::update() {
	if(users == 0) {
		columns[DOWNLOAD_COLUMN_STATUS] = T_("Waiting for slot");
		columns[DOWNLOAD_COLUMN_TIMELEFT].clear();
		columns[DOWNLOAD_COLUMN_SPEED].clear();
	} else {
		columns[DOWNLOAD_COLUMN_STATUS] = str(TFN_("Downloading from %1% user", "Downloading from %1% users", users) % users);
		columns[DOWNLOAD_COLUMN_TIMELEFT] = Text::toT(Util::formatSeconds(static_cast<int64_t>(timeleft())));
		columns[DOWNLOAD_COLUMN_SPEED] = str(TF_("%1%/s") % Text::toT(Util::formatBytes(static_cast<int64_t>(bps))));
	}
	columns[DOWNLOAD_COLUMN_DONE] = Text::toT(Util::formatBytes(done));
}

void TransferView::on(ConnectionManagerListener::Added, ConnectionQueueItem* aCqi) throw() {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());

	ui->setStatus(ConnectionInfo::STATUS_WAITING);
	ui->setStatusString(T_("Connecting"));
	speak(CONNECTIONS_ADD, ui);
}

void TransferView::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem* aCqi) throw() {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());

	ui->setStatusString((aCqi->getState() == ConnectionQueueItem::CONNECTING) ? T_("Connecting") : T_("Waiting to retry"));

	speak(CONNECTIONS_UPDATE, ui);
}

void TransferView::on(ConnectionManagerListener::Removed, ConnectionQueueItem* aCqi) throw() {
	speak(CONNECTIONS_REMOVE, new UpdateInfo(aCqi->getUser(), aCqi->getDownload()));
}

void TransferView::on(ConnectionManagerListener::Failed, ConnectionQueueItem* aCqi, const string& aReason) throw() {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());
	if(aCqi->getUser()->isSet(User::OLD_CLIENT)) {
		ui->setStatusString(T_("Remote client does not fully support TTH - cannot download"));
	} else {
		ui->setStatusString(Text::toT(aReason));
	}
	speak(CONNECTIONS_UPDATE, ui);
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
	ui->setStatus(ConnectionInfo::STATUS_RUNNING);
	ui->setTransfered(t->getPos(), t->getActual());
	ui->setChunk(t->getPos(), t->getSize());
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

void TransferView::on(DownloadManagerListener::Requesting, Download* d) throw() {
	UpdateInfo* ui = new UpdateInfo(d->getUser(), true);
	
	starting(ui, d);

	ui->setStatusString(str(TF_("Requesting %1%") % getFile(d)));

	speak(CONNECTIONS_UPDATE, ui);
	
	speak(DOWNLOADS_ADD_USER, new TickInfo(d->getPath()));
}

void TransferView::on(DownloadManagerListener::Starting, Download* d) throw() {
	UpdateInfo* ui = new UpdateInfo(d->getUser(), true);
	
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
	speak(CONNECTIONS_UPDATE, ui);
}

void TransferView::onTransferTick(Transfer* t, bool isDownload) {
	UpdateInfo* ui = new UpdateInfo(t->getUser(), isDownload);
	ui->setTransfered(t->getPos(), t->getActual());
	ui->setSpeed(t->getAverageSpeed());
	ui->setChunk(t->getPos(), t->getSize());
	tasks.add(CONNECTIONS_UPDATE, ui);
}

void TransferView::on(DownloadManagerListener::Tick, const DownloadList& dl) throw()  {
	for(DownloadList::const_iterator i = dl.begin(); i != dl.end(); ++i) {
		onTransferTick(*i, true);
	}

	std::vector<TickInfo*> dis;
	for(DownloadList::const_iterator i = dl.begin(); i != dl.end(); ++i) {
		Download* d = *i;
		if(d->getType() != Transfer::TYPE_FILE) {
			continue;
		}
		
		TickInfo* ti = 0;
		for(std::vector<TickInfo*>::iterator j = dis.begin(); j != dis.end(); ++j) {
			TickInfo* ti2 = *j;
			if(Util::stricmp(ti2->path, d->getPath()) == 0) {
				ti = ti2;
				break;
			}
		}
		if(!ti) {
			ti = new TickInfo(d->getPath());
			dis.push_back(ti);
		}
		ti->bps += d->getAverageSpeed();
		ti->done += d->getPos();
	}
	for(std::vector<TickInfo*>::iterator i = dis.begin(); i != dis.end(); ++i) {
		tasks.add(DOWNLOADS_TICK, *i);
	}

	speak();
}

void TransferView::on(DownloadManagerListener::Failed, Download* d, const string& aReason) throw() {
	UpdateInfo* ui = new UpdateInfo(d->getUser(), true, true);
	ui->setStatus(ConnectionInfo::STATUS_WAITING);
	ui->setStatusString(Text::toT(aReason));

	speak(CONNECTIONS_UPDATE, ui);
	
	speak(DOWNLOADS_REMOVE_USER, new TickInfo(d->getPath()));
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

	speak(CONNECTIONS_UPDATE, ui);
}

void TransferView::on(UploadManagerListener::Tick, const UploadList& ul) throw() {
	for(UploadList::const_iterator i = ul.begin(); i != ul.end(); ++i) {
		onTransferTick(*i, false);
	}

	speak();
}

void TransferView::on(DownloadManagerListener::Complete, Download* d) throw() { 
	onTransferComplete(d, true);

	speak(DOWNLOADS_REMOVE_USER, new TickInfo(d->getPath()));
}

void TransferView::on(UploadManagerListener::Complete, Upload* aUpload) throw() { 
	onTransferComplete(aUpload, false); 
}

void TransferView::onTransferComplete(Transfer* aTransfer, bool isDownload) {
	UpdateInfo* ui = new UpdateInfo(aTransfer->getUser(), isDownload);

	ui->setStatus(ConnectionInfo::STATUS_WAITING);
	ui->setStatusString(T_("Idle"));
	ui->setChunk(aTransfer->getPos(), aTransfer->getSize());

	speak(CONNECTIONS_UPDATE, ui);
}

void TransferView::ConnectionInfo::disconnect() {
	ConnectionManager::getInstance()->disconnect(user, download);
}

void TransferView::on(QueueManagerListener::Removed, QueueItem* qi) throw() {
	speak(DOWNLOADS_REMOVED, new TickInfo(qi->getTarget()));
}
