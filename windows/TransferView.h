/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(TRANSFER_VIEW_H)
#define TRANSFER_VIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/CriticalSection.h"
#include "../client/ConnectionManagerListener.h"

#include "UCHandler.h"
#include "TypedListViewCtrl.h"
#include "WinUtil.h"

class TransferView : public CWindowImpl<TransferView>, private DownloadManagerListener, 
	private UploadManagerListener, private ConnectionManagerListener, 
	public UserInfoBaseHandler<TransferView>, public UCHandler<TransferView>
{
public:
	DECLARE_WND_CLASS(_T("TransferView"))

	TransferView() { }
	virtual ~TransferView(void);

	typedef UserInfoBaseHandler<TransferView> uibBase;
	typedef UCHandler<TransferView> ucBase;

	BEGIN_MSG_MAP(TransferView)
		NOTIFY_HANDLER(IDC_TRANSFERS, LVN_GETDISPINFO, ctrlTransfers.onGetDispInfo)
		NOTIFY_HANDLER(IDC_TRANSFERS, LVN_COLUMNCLICK, ctrlTransfers.onColumnClick)
		NOTIFY_HANDLER(IDC_TRANSFERS, LVN_KEYDOWN, onKeyDownTransfers)
		NOTIFY_HANDLER(IDC_TRANSFERS, NM_CUSTOMDRAW, onCustomDraw)
		NOTIFY_HANDLER(IDC_TRANSFERS, NM_DBLCLK, onDoubleClickTransfers)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_DESTROY, onDestroy)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SIZE, onSize)
		MESSAGE_HANDLER(WM_NOTIFYFORMAT, onNotifyFormat)
		COMMAND_ID_HANDLER(IDC_FORCE, onForce)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_REMOVEALL, onRemoveAll)
		COMMAND_ID_HANDLER(IDC_COPY_NICK, onCopyNick)
		CHAIN_COMMANDS(ucBase)
		CHAIN_COMMANDS(uibBase)
	END_MSG_MAP()

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onForce(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);			
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onDoubleClickTransfers(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT onCopyNick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void runUserCommand(UserCommand& uc);

	void prepareClose();

	LRESULT onKeyDownTransfers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
		if(kd->wVKey == VK_DELETE) {
			ctrlTransfers.forEachSelected(&ItemInfo::disconnect);
		}
		return 0;
	}

	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ctrlTransfers.forEachSelected(&ItemInfo::disconnect);
		return 0;
	}

	LRESULT onRemoveAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ctrlTransfers.forEachSelected(&ItemInfo::removeAll);
		return 0;
	}

	LRESULT onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlTransfers.forEach(&ItemInfo::deleteSelf);
		return 0;
	}

	LRESULT onNotifyFormat(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
#ifdef _UNICODE
		return NFR_UNICODE;
#else
		return NFR_ANSI;
#endif		
	}

private:
	/** Parameter map for user commands */
	class ItemInfo;
public:
	TypedListViewCtrl<ItemInfo, IDC_TRANSFERS>& getUserList() { return ctrlTransfers; }
private:
	enum {
		ADD_ITEM,
		REMOVE_ITEM,
		UPDATE_ITEM,
		UPDATE_ITEMS,
	};

	enum {
		COLUMN_FIRST,
		COLUMN_USER = COLUMN_FIRST,
		COLUMN_HUB,
		COLUMN_STATUS,
		COLUMN_TIMELEFT,
		COLUMN_SPEED,
		COLUMN_FILE,
		COLUMN_SIZE,
		COLUMN_PATH,
		COLUMN_IP,
		COLUMN_RATIO,
		COLUMN_LAST
	};

	enum {
		IMAGE_DOWNLOAD = 0,
		IMAGE_UPLOAD
	};

	struct UpdateInfo;
	class ItemInfo : public UserInfoBase {
	public:
		enum Status {
			STATUS_RUNNING,
			STATUS_WAITING
		};

		ItemInfo(const User::Ptr& u, bool aDownload);

		bool download;
		bool transferFailed;
		Status status;
		int64_t pos;
		int64_t size;
		int64_t start;
		int64_t actual;
		int64_t speed;
		int64_t timeLeft;

		tstring columns[COLUMN_LAST];
		void update(const UpdateInfo& ui);

		void disconnect();
		void removeAll();
		void deleteSelf() { delete this; }

		double getRatio() { return (pos > 0) ? (double)actual / (double)pos : 1.0; }

		const tstring& getText(int col) const {
			return columns[col];
		}

		static int compareItems(ItemInfo* a, ItemInfo* b, int col) {
			if(a->status == b->status) {
				if(a->download != b->download) {
					return a->download ? -1 : 1;
				}
			} else {
				return (a->status == ItemInfo::STATUS_RUNNING) ? -1 : 1;
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
	};

	struct UpdateInfo {
		enum {
			MASK_POS = 1 << 0,
			MASK_SIZE = 1 << 1,
			MASK_START = 1 << 2,
			MASK_ACTUAL = 1 << 3,
			MASK_SPEED = 1 << 4,
			MASK_FILE = 1 << 5,
			MASK_STATUS = 1 << 6,
			MASK_TIMELEFT = 1 << 7,
			MASK_IP = 1 << 8,
			MASK_STATUS_STRING = 1 << 9,
			MASK_COUNTRY = 1 << 10
		};

		bool operator==(const ItemInfo& ii) { return download == ii.download && user == ii.user; }

		UpdateInfo(const User::Ptr& aUser, bool isDownload, bool isTransferFailed = false) : updateMask(0), user(aUser), download(isDownload), transferFailed(isTransferFailed) { }

		u_int32_t updateMask;

		User::Ptr user;
		bool download;
		bool transferFailed;
		void setStatus(ItemInfo::Status aStatus) { status = aStatus; updateMask |= MASK_STATUS; }
		ItemInfo::Status status;
		void setPos(int64_t aPos) { pos = aPos; updateMask |= MASK_POS; }
		int64_t pos;
		void setSize(int64_t aSize) { size = aSize; updateMask |= MASK_SIZE; }
		int64_t size;
		void setStart(int64_t aStart) { start = aStart; updateMask |= MASK_START; }
		int64_t start;
		void setActual(int64_t aActual) { actual = aActual; updateMask |= MASK_ACTUAL; }
		int64_t actual;
		void setSpeed(int64_t aSpeed) { speed = aSpeed; updateMask |= MASK_SPEED; }
		int64_t speed;
		void setTimeLeft(int64_t aTimeLeft) { timeLeft = aTimeLeft; updateMask |= MASK_TIMELEFT; }
		int64_t timeLeft;
		void setStatusString(const tstring& aStatusString) { statusString = aStatusString; updateMask |= MASK_STATUS_STRING; }
		tstring statusString;
		void setFile(const tstring& aFile) { file = Util::getFileName(aFile); path = Util::getFilePath(aFile); updateMask|= MASK_FILE; }
		tstring file;
		tstring path;
		void setIP(const tstring& aIP) { IP = aIP; updateMask |= MASK_IP; }
		tstring IP;
	};

	void speak(int type, UpdateInfo* ui) { PostMessage(WM_SPEAKER, type, reinterpret_cast<LPARAM>(ui)); }
	void speak(int type, vector<UpdateInfo*>* ui) { PostMessage(WM_SPEAKER, type, reinterpret_cast<LPARAM>(ui)); }

	CriticalSection cs;

	TypedListViewCtrl<ItemInfo, IDC_TRANSFERS> ctrlTransfers;
	static int columnIndexes[];
	static int columnSizes[];

	CMenu transferMenu;
	CImageList arrows;

	virtual void on(ConnectionManagerListener::Added, ConnectionQueueItem* aCqi) throw();
	virtual void on(ConnectionManagerListener::Failed, ConnectionQueueItem* aCqi, const string& aReason) throw();
	virtual void on(ConnectionManagerListener::Removed, ConnectionQueueItem* aCqi) throw();
	virtual void on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem* aCqi) throw();

	virtual void on(DownloadManagerListener::Complete, Download* aDownload) throw() { onTransferComplete(aDownload, false);}
	virtual void on(DownloadManagerListener::Failed, Download* aDownload, const string& aReason) throw();
	virtual void on(DownloadManagerListener::Starting, Download* aDownload) throw();
	virtual void on(DownloadManagerListener::Tick, const Download::List& aDownload) throw();

	virtual void on(UploadManagerListener::Starting, Upload* aUpload) throw();
	virtual void on(UploadManagerListener::Tick, const Upload::List& aUpload) throw();
	virtual void on(UploadManagerListener::Complete, Upload* aUpload) throw() { onTransferComplete(aUpload, true); }

	void onTransferComplete(Transfer* aTransfer, bool isUpload);
};

#endif // !defined(TRANSFER_VIEW_H)

/**
 * @file
 * $Id: TransferView.h,v 1.26 2006/02/19 16:19:06 arnetheduck Exp $
 */
